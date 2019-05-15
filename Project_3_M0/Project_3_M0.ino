#include <SPI.h>
#include <WiFi101.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <LSM303.h>
#include <math.h>

// Constant values for pin numbers and robot dimensions
#define RADIUS          29.0 // Wheel radii in mm 31.4/2
#define BASELINE        87.5 // Transaxial distance between center of each wheels 82.4
#define GEAR_RATIO      75.81 // Gear ratio of motor gearbox (input/output) 75.81
#define TICKS_PER_ROT   2.0f  // How many ticks will register per rotation of encoder wheel (depends on both encoder design AND what edge(s) trigger interrupts)
#define MOTOR_RIGHT_FORWARD   12    // Right motor pin
#define MOTOR_RIGHT_REVERSE   11    // Right motor pin
#define MOTOR_LEFT_FORWARD    10    // Left motor pin
#define MOTOR_LEFT_REVERSE    9    // Left motor pin
#define NORTH           0.0f  // North heading
#define SOUTH           180.0f  // South heading
#define EAST            90.0f  // East heading
#define WEST            270.0f  // West heading
#define IR_RIGHT        5     // Right wheel IR sensor pin
#define IR_LEFT         6     // Left wheel IR sensor pin

#define SECRET_SSID "Unifi-Home"
#define SECRET_PASS "montakhebolmolouk"

struct __attribute__((__packed__)) command
{
  double translational;
  double angle;
  int mode;
} input_command = {0.0, 0.0, 0};

struct __attribute__((__packed__)) robot_info
{
  double odo[3];  // x, y, z positions
  double imu[6];  // x, y, z accelerations and x, y, z magnetisms
  double head;  // Current IMU heading. NOTE: Not named "heading" because that's the name of a function
} cur_info = {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0, 0.0, 0.0}, 0.0};

//char ssid[] = "Narimaan-M0";
int status = WL_IDLE_STATUS;
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
unsigned int localPort = 4242;      // local port to listen on

unsigned long info_time;

int packetSize;

unsigned int reset = 0;

float desired_angle, start_angle, error, control, K_p = 8.0;

unsigned int send_port = 4242, receive_port = 5005;
unsigned long tick_time, tock_time, desired_control_time = 10;
double odometry_phi = 0.0;

// Phi angle radians per single tick
// (RADIUS/BASELINE) phi radians/wheel radians 
// 2pi wheel radians/wheel rotation
// 1/GEAR_RATIO wheel rotations/encoder rotation
// 1/TICKS_PER_ROT encoder rotation/tick
// So phi radians / tick = ((RADIUS/BASELINE) phi radians/wheel radians)*(2pi wheel radians/wheel rotation)*(1/GEAR_RATIO wheel rotations/encoder rotation)*(1/TICKS_PER_ROT encoder rotation/tick)
const float phi_radians_per_tick = (RADIUS / BASELINE) * (PI) * (1.0f / GEAR_RATIO) * (1.0f / TICKS_PER_ROT);

// Constant delta_x and delta_y values based on phi_radians_per_tick used by ISRs
const float delta_x = (BASELINE / 2.0) * sin(phi_radians_per_tick); // Calculate local delta x based on local phi
const float delta_y = (BASELINE / 2.0) - ((BASELINE / 2.0) * cos(phi_radians_per_tick)); // Calculate local delta y based on local phi

float acc_y;

WiFiUDP send_udp, receive_udp;
IPAddress m0(192, 168, 1, 74);
IPAddress pi(192, 168, 1, 72);
LSM303 compass;

void setup()
{
  Serial.begin(9600);

  pinMode(MOTOR_RIGHT_FORWARD, OUTPUT);
  pinMode(MOTOR_RIGHT_REVERSE, OUTPUT);
  pinMode(MOTOR_LEFT_FORWARD, OUTPUT);
  pinMode(MOTOR_LEFT_REVERSE, OUTPUT);
  pinMode(IR_RIGHT, INPUT_PULLUP);
  pinMode(IR_LEFT, INPUT_PULLUP);
  analogWrite(MOTOR_RIGHT_FORWARD, 0);
  analogWrite(MOTOR_RIGHT_REVERSE, 0);
  analogWrite(MOTOR_LEFT_FORWARD, 0);
  analogWrite(MOTOR_LEFT_REVERSE, 0);
  attachInterrupt(digitalPinToInterrupt(IR_RIGHT), analytical_odometry_right, RISING);
  attachInterrupt(digitalPinToInterrupt(IR_LEFT), analytical_odometry_left, RISING);
//  while(!Serial);
  Wire.begin();
  compass.init();
  compass.enableDefault();
  compass.m_min = (LSM303::vector<int16_t>){+1962, -4001, -923};
  compass.m_max = (LSM303::vector<int16_t>){+10028, +3653, +6260};
  
  WiFi.setPins(8, 7, 4, 2);
  while (status != WL_CONNECTED)
  {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    delay(3000);
  }

  Serial.print("You're connected to the network");
  printCurrentNet();
  printWiFiData();
  
  send_udp.begin(send_port);

  compass.read();
  start_angle = compass.heading((LSM303::vector<int>){-1, 0, 0});
  cur_info.head = start_angle;
  cur_info.odo[0] = 0.0;
  cur_info.odo[1] = 0.0;
  cur_info.odo[2] = 0.0;
}

void loop()
{
  tick_time = millis();

  compass.read();
  cur_info.imu[0] = (double)compass.a.x * -9.80665 * 0.061 / 1000.0;  // Convert from bits to m/s^2
  cur_info.imu[1] = (double)compass.a.y * -9.80665 * 0.061 / 1000.0;  // Convert from bits to m/s^2
  cur_info.imu[2] = (double)compass.a.z * -9.80665 * 0.061 / 1000.0;  // Convert from bits to m/s^2
  cur_info.imu[3] = (double)compass.m.x * 0.160 / 1000.0; // Convert from bits to gauss
  cur_info.imu[4] = (double)compass.m.y * 0.160 / 1000.0; // Convert from bits to gauss
  cur_info.imu[5] = (double)compass.m.z * 0.160 / 1000.0; // Convert from bits to gauss
  cur_info.head = compass.heading((LSM303::vector<int>){-1, 0, 0}); // Save heading
  
  if (packetSize = send_udp.parsePacket())
  {
    send_udp.read((char*)(&input_command), sizeof(command));
    send_udp.beginPacket(send_udp.remoteIP(), send_udp.remotePort());
    send_udp.write((char*)&cur_info, sizeof(cur_info));
    send_udp.endPacket();
  }
  
  acc_y = (float)compass.a.y * 0.061 / 1000.0;

  if (acc_y > 1.15)
  {
    rotate(0.0);
    reset = 1;
  }

  // Relative angle mode since arrow keys are being used
  if (input_command.mode == 0)
  {
//    K_p = 3.0;
    desired_angle = start_angle + input_command.angle;
    if (desired_angle > 360.0f)
      desired_angle -= 360.0f;
    else if (desired_angle < 0.0f)
      desired_angle += 360.0f;
  }

  // Absolute angle mode since WASD is being used to go in the cardinal directions
  else if (input_command.mode == 1)
  {
//    K_p = 5.0;
    desired_angle = input_command.angle;
  }

  if (!reset)
  {
    analogWrite(MOTOR_RIGHT_FORWARD, input_command.translational);
    analogWrite(MOTOR_LEFT_FORWARD, input_command.translational);
  }

  if (!reset && !input_command.translational)
  {
    error = abs(desired_angle - compass.heading((LSM303::vector<int>){-1, 0, 0}));
  
    if (error > 180.0f)
      error = 360.0f - error;
    else
      error = desired_angle - compass.heading((LSM303::vector<int>){-1, 0, 0});

    if (abs(error) < 5.0)
      error = 0.0;
    
    control = error * K_p;
  
    rotate(control);
  }



  // Get time at end of P controller loop
  tock_time = millis();

  // Ensure at least the minimum delay has occured for P controller loop
  delay(desired_control_time - (tock_time - tick_time));
}

void rotate(double omega)
{
  double s = abs(omega);
  if (s < 30.0f)
    s = 30.0f;
  else if (s > 200.0f)
    s = 200.0f;
  
  if (omega > 0.0)
  {
    analogWrite(MOTOR_LEFT_REVERSE, 0);
    analogWrite(MOTOR_RIGHT_FORWARD, 0);
    analogWrite(MOTOR_RIGHT_REVERSE, 0);
    analogWrite(MOTOR_LEFT_FORWARD, (int)s);
  }

  else if (omega < 0.0)
  {
     analogWrite(MOTOR_RIGHT_REVERSE, 0);
    analogWrite(MOTOR_LEFT_FORWARD, 0);
    analogWrite(MOTOR_LEFT_REVERSE, 0);
    analogWrite(MOTOR_RIGHT_FORWARD, (int)s);
  }

  else
  {
    analogWrite(MOTOR_RIGHT_FORWARD, 0);
    analogWrite(MOTOR_RIGHT_REVERSE, 0);
    analogWrite(MOTOR_LEFT_FORWARD, 0);
    analogWrite(MOTOR_LEFT_REVERSE, 0);
  }
}

void printWiFiData() {
  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  Serial.println(ip);

  // print your MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC address: ");
  printMacAddress(mac);

}

void printCurrentNet() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print the MAC address of the router you're attached to:
  byte bssid[6];
  WiFi.BSSID(bssid);
  Serial.print("BSSID: ");
  printMacAddress(bssid);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.println(rssi);

  // print the encryption type:
  byte encryption = WiFi.encryptionType();
  Serial.print("Encryption Type:");
  Serial.println(encryption, HEX);
  Serial.println();
}

void printMacAddress(byte mac[]) {
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16) {
      Serial.print("0");
    }
    Serial.print(mac[i], HEX);
    if (i > 0) {
      Serial.print(":");
    }
  }
  Serial.println();
}

// ISR for calculating odometry of right wheel based on IR sensor signal using analytical method
void analytical_odometry_right()
{ 
  odometry_phi += phi_radians_per_tick; // Calculate new global phi angle based on ratio between phi radians and ticks
  cur_info.odo[0] += (delta_x * cos(odometry_phi)) + (delta_y * sin(odometry_phi));  // Update global x based on global phi and local delta x and y
  cur_info.odo[1] += (delta_x * sin(odometry_phi)) + (delta_y * cos(odometry_phi));  // Update global y based on global phi and local delta x and y
}

// ISR for calculating odometry of left wheel based on IR sensor signal using analytical method
void analytical_odometry_left()
{
  odometry_phi -= phi_radians_per_tick; // Calculate new global phi angle based on ratio between phi radians and ticks
  cur_info.odo[0] += (delta_x * cos(odometry_phi)) + (delta_y * sin(odometry_phi));  // Update global x based on global phi and local delta x and y
  cur_info.odo[1] -= (delta_x * sin(odometry_phi)) + (delta_y * cos(odometry_phi));  // Update global y based on global phi and local delta x and y
}
