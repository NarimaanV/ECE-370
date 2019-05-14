#include <SPI.h>
#include <WiFi101.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <LSM303.h>
#include <math.h>

// Constant values for pin numbers and robot dimensions
#define RADIUS          58.4f // Wheel radii in mm 31.4/2
#define BASELINE        87.5f // Transaxial distance between center of each wheels 82.4
#define GEAR_RATIO      75.81f // Gear ratio of motor gearbox (input/output) 75.81
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
} input_command;

struct __attribute__((__packed__)) robot_info
{
  double odo[3];
  double imu[6];
  double head;
} cur_info = {{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0, 7.0, 8.0, 9.0}, 10.0};

//char ssid[] = "Narimaan-M0";
int status = WL_IDLE_STATUS;
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
unsigned int localPort = 4242;      // local port to listen on

unsigned long info_time;

int packetSize;

unsigned int reset = 0;

float desired_angle, start_angle, error, control, K_p = 3.0;

unsigned int send_port = 4242, receive_port = 5005;
unsigned long tick_time, tock_time, desired_control_time = 10;

// Phi angle radians per single tick
// (RADIUS/BASELINE) phi radians/wheel radians 
// 2pi wheel radians/wheel rotation
// 1/GEAR_RATIO wheel rotations/encoder rotation
// 1/TICKS_PER_ROT encoder rotation/tick
// So phi radians / tick = ((RADIUS/BASELINE) phi radians/wheel radians)*(2pi wheel radians/wheel rotation)*(1/GEAR_RATIO wheel rotations/encoder rotation)*(1/TICKS_PER_ROT encoder rotation/tick)
const float phi_radians_per_tick = (RADIUS / BASELINE) * (2.0f * PI) * (1.0f / GEAR_RATIO) * (1.0f / TICKS_PER_ROT);

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
  while(!Serial);
  pinMode(MOTOR_RIGHT_FORWARD, OUTPUT);
  pinMode(MOTOR_RIGHT_REVERSE, OUTPUT);
  pinMode(MOTOR_LEFT_FORWARD, OUTPUT);
  pinMode(MOTOR_LEFT_REVERSE, OUTPUT);
  analogWrite(MOTOR_RIGHT_FORWARD, 0);
  analogWrite(MOTOR_RIGHT_REVERSE, 0);
  analogWrite(MOTOR_LEFT_FORWARD, 0);
  analogWrite(MOTOR_LEFT_REVERSE, 0);
  attachInterrupt(digitalPinToInterrupt(IR_RIGHT), analytical_odometry_right, RISING);
  attachInterrupt(digitalPinToInterrupt(IR_LEFT), analytical_odometry_left, RISING);
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
  receive_udp.begin(receive_port);

  info_time = millis();
  compass.read();
  start_angle = compass.heading((LSM303::vector<int>){-1, 0, 0});
}

void loop()
{
  tick_time = millis();
  
  if (packetSize = receive_udp.parsePacket())
    receive_udp.read((char*)(&input_command), sizeof(command));

  if (millis() - info_time >= 1000)
  {
    send_udp.beginPacket(pi, send_port);
    char send_buffer[1024] = { 0 };
    memcpy(send_buffer, &cur_info, sizeof(robot_info));
    send_udp.write(send_buffer, sizeof(robot_info));
    send_udp.endPacket();
    Serial.println("Sent!");
    info_time = millis();
  }

  compass.read();
  cur_info.odo[2] = compass.heading((LSM303::vector<int>){-1, 0, 0});
  acc_y = (float)compass.a.y * 0.061 / 1000.0;

  if (acc_y > 1.15f)
  {
    rotate(0.0);
    reset = 1;
  }

  switch (input_command.mode)
  {
    case 0:
      K_p = 3.0;
      desired_angle = start_angle + input_command.angle;
      if (desired_angle > 360.0f)
        desired_angle -= 360.0f;
      else if (desired_angle < 0.0f)
        desired_angle += 360.0f;
      break;
    case 1:
      K_p = 2.0;
      start_angle = desired_angle = 0.0f;
      input_command.mode = 5;
      break;
    case 2:
      K_p = 2.0;
      start_angle = desired_angle = 270.0f;
      input_command.mode = 5;
      break;
    case 3:
      K_p = 2.0;
      start_angle = desired_angle = 180.0f;
      input_command.mode = 5;
      break;
    case 4:
      K_p = 2.0;
      start_angle = desired_angle = 90.0f;
      input_command.mode = 5;
      break;
    default:
      break;
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
    
    control = error * K_p;
  
    rotate(control);
  }

  // Get time at end of P controller loop
  tock_time = millis();

  // Ensure at least the minimum delay has occured for P controller loop
  delay(desired_control_time - (tock_time - tick_time));
}

void rotate(float omega)
{
  float s = abs(omega);
  if (s < 30.0f)
    s = 30.0f;
  else if (s > 200.0f)
    s = 200.0f;
  
  if (omega > 0.0f)
  {
    analogWrite(MOTOR_LEFT_REVERSE, 0);
    analogWrite(MOTOR_RIGHT_FORWARD, 0);
    analogWrite(MOTOR_RIGHT_REVERSE, (int)s);
    analogWrite(MOTOR_LEFT_FORWARD, (int)s);
  }

  else if (omega < 0.0f)
  {
     analogWrite(MOTOR_RIGHT_REVERSE, 0);
    analogWrite(MOTOR_LEFT_FORWARD, 0);
    analogWrite(MOTOR_LEFT_REVERSE, (int)s);
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
  cur_info.odo[2] += phi_radians_per_tick; // Calculate new global phi angle based on ratio between phi radians and ticks
  cur_info.odo[0] += (delta_x * cos(cur_info.odo[2])) + (delta_y * sin(cur_info.odo[2]));  // Update global x based on global phi and local delta x and y
  cur_info.odo[1] += (delta_x * sin(cur_info.odo[2])) + (delta_y * cos(cur_info.odo[2]));  // Update global y based on global phi and local delta x and y
}

// ISR for calculating odometry of left wheel based on IR sensor signal using analytical method
void analytical_odometry_left()
{
  cur_info.odo[2] -= phi_radians_per_tick; // Calculate new global phi angle based on ratio between phi radians and ticks
  cur_info.odo[0] += (delta_x * cos(cur_info.odo[2])) + (delta_y * sin(cur_info.odo[2]));  // Update global x based on global phi and local delta x and y
  cur_info.odo[1] -= (delta_x * sin(cur_info.odo[2])) + (delta_y * cos(cur_info.odo[2]));  // Update global y based on global phi and local delta x and y
}
