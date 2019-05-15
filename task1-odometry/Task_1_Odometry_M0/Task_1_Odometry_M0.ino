#include <SPI.h>
#include <WiFi101.h>
#include <WiFiUdp.h>
#include <math.h>

// Constant values for pin numbers and robot dimensions
#define RADIUS          29.0 // Wheel radii in mm 31.4/2
#define BASELINE        87.5 // Transaxial distance between center of each wheels 82.4
#define GEAR_RATIO      75.81f // Gear ratio of motor gearbox (input/output) 75.81
#define TICKS_PER_ROT   2.0f  // How many ticks will register per rotation of encoder wheel (depends on both encoder design AND what edge(s) trigger interrupts)
#define MOTOR_RIGHT_A   12    // Right motor pin
#define MOTOR_RIGHT_B   11    // Right motor pin
#define MOTOR_LEFT_A    10    // Left motor pin
#define MOTOR_LEFT_B    9    // Left motor pin
#define IR_RIGHT        5     // Right wheel IR sensor pin
#define IR_LEFT         6     // Left wheel IR sensor pin

// Network constants
#define SECRET_SSID "Unifi-Home"
#define SECRET_PASS "montakhebolmolouk"

struct __attribute__((__packed__)) command
{
  double translational;
  double rotational;
  int mode;
} input_command = {0.0, 0.0, 0};

struct __attribute__((__packed__)) robot_info
{
  double x;
  double y;
  double phi;
} cur_info = {0.0, 0.0, 0.0};

// Phi angle radians per single tick
// (RADIUS/BASELINE) phi radians/wheel radians 
// 2pi wheel radians/wheel rotation
// 1/GEAR_RATIO wheel rotations/encoder rotation
// 1/TICKS_PER_ROT encoder rotation/tick
// So phi radians / tick = ((RADIUS/BASELINE) phi radians/wheel radians)*(2pi wheel radians/wheel rotation)*(1/GEAR_RATIO wheel rotations/encoder rotation)*(1/TICKS_PER_ROT encoder rotation/tick)
const double phi_radians_per_tick = (RADIUS / BASELINE) * (PI) * (1.0f / GEAR_RATIO) * (1.0f / TICKS_PER_ROT);

// Constant delta_x and delta_y values based on phi_radians_per_tick used by ISRs
const double delta_x = (BASELINE / 2.0) * sin(phi_radians_per_tick); // Calculate local delta x based on local phi
const double delta_y = (BASELINE / 2.0) - ((BASELINE / 2.0) * cos(phi_radians_per_tick)); // Calculate local delta y based on local phi

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
int status = WL_IDLE_STATUS;     // the WiFi radio's status

unsigned int localPort = 4242;      // local port to listen on

unsigned int right_speed, left_speed;

int packetSize;

WiFiUDP Udp;

void setup()
{
  pinMode(MOTOR_RIGHT_A, OUTPUT);
  pinMode(MOTOR_RIGHT_B, OUTPUT);
  pinMode(MOTOR_LEFT_A, OUTPUT);
  pinMode(MOTOR_LEFT_B, OUTPUT);
  pinMode(IR_RIGHT, INPUT_PULLUP);
  pinMode(IR_LEFT, INPUT_PULLUP);
  analogWrite(MOTOR_RIGHT_A, 0);
  analogWrite(MOTOR_RIGHT_B, 0);
  analogWrite(MOTOR_LEFT_A, 0);
  analogWrite(MOTOR_LEFT_B, 0);
  attachInterrupt(digitalPinToInterrupt(IR_RIGHT), analytical_odometry_right, RISING);
  attachInterrupt(digitalPinToInterrupt(IR_LEFT), analytical_odometry_left, RISING);
  WiFi.setPins(8, 7, 4, 2);
  Serial.begin(9600);
//  while (!Serial);
  while ( status != WL_CONNECTED)
  {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass); 

    // wait 10 seconds for connection:
    delay(5000);
  }

  Serial.print("You're connected to the network");
  printCurrentNet();
  printWiFiData();

  Serial.println("\nStarting connection to server...");
  // if you get a connection, report back via serial:
  Udp.begin(localPort);

  cur_info.x = 0.0;
  cur_info.y = 0.0;
  cur_info.phi = 0.0;
}

void loop()
{
  if (packetSize = Udp.parsePacket())
  {
    Udp.read((char*)(&input_command), sizeof(command));
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write((char*)(&cur_info), sizeof(cur_info));
    Udp.endPacket();
  }

  right_speed = ((2.0 * input_command.translational) + (input_command.rotational * BASELINE)) / (2.0 * RADIUS);
  left_speed = ((2.0 * input_command.translational) - (input_command.rotational * BASELINE)) / (2.0 * RADIUS);

  if (right_speed > 255) right_speed = 255;
  if (left_speed > 255) left_speed = 255;
  
  analogWrite(MOTOR_RIGHT_A, right_speed);
  analogWrite(MOTOR_LEFT_A, left_speed);

  delay(20);
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

void printMacAddress(byte mac[])
{
  for (int i = 5; i >= 0; i--)
  {
    if (mac[i] < 16)
    {
      Serial.print("0");
    }
    Serial.print(mac[i], HEX);
    if (i > 0)
    {
      Serial.print(":");
    }
  }
  Serial.println();
}

// ISR for calculating odometry of right wheel based on IR sensor signal using analytical method
void analytical_odometry_right()
{ 
  cur_info.phi *= (PI / 180.0);
  cur_info.phi += phi_radians_per_tick; // Calculate new global phi angle based on ratio between phi radians and ticks
  cur_info.x += (delta_x * cos(cur_info.phi)) + (delta_y * sin(cur_info.phi));  // Update global x based on global phi and local delta x and y
  cur_info.y += (delta_x * sin(cur_info.phi)) + (delta_y * cos(cur_info.phi));  // Update global y based on global phi and local delta x and y
  cur_info.phi *= (180.0 / PI);
}

// ISR for calculating odometry of left wheel based on IR sensor signal using analytical method
void analytical_odometry_left()
{
  cur_info.phi *= (PI / 180.0);
  cur_info.phi -= phi_radians_per_tick; // Calculate new global phi angle based on ratio between phi radians and ticks
  cur_info.x += (delta_x * cos(cur_info.phi)) + (delta_y * sin(cur_info.phi));  // Update global x based on global phi and local delta x and y
  cur_info.y -= (delta_x * sin(cur_info.phi)) + (delta_y * cos(cur_info.phi));  // Update global y based on global phi and local delta x and y
  cur_info.phi *= (180.0 / PI);
}
