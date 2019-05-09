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
  float translational;
  float rotational;
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

unsigned int send_port = 4242, receive_port = 5005;

WiFiUDP send_udp, receive_udp;
//WiFiUDP Udp;

void setup()
{
  Serial.begin(9600);
  pinMode(MOTOR_RIGHT_FORWARD, OUTPUT);
  pinMode(MOTOR_RIGHT_REVERSE, OUTPUT);
  pinMode(MOTOR_LEFT_FORWARD, OUTPUT);
  pinMode(MOTOR_LEFT_REVERSE, OUTPUT);
  analogWrite(MOTOR_RIGHT_FORWARD, 0);
  analogWrite(MOTOR_RIGHT_REVERSE, 0);
  analogWrite(MOTOR_LEFT_FORWARD, 0);
  analogWrite(MOTOR_LEFT_REVERSE, 0);

  WiFi.setPins(8, 7, 4, 2);
//  WiFi.config(IPAddress(10, 0, 0, 1));
//  status = WiFi.beginAP(ssid);
//  if (status != WL_AP_LISTENING) {
//    Serial.println("Creating access point failed");
//    // don't continue
//    while (true);
//  }
//
//  send_udp.begin(send_port);
//  receive_udp.begin(receive_port);

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
//  Udp.begin(localPort);
  send_udp.begin(send_port);
  receive_udp.begin(receive_port);

  info_time = millis();
//  while(!(packetSize = Udp.parsePacket()));
//
//  if (packetSize)
//  {
//    Udp.read((char*)(&input_command), sizeof(command));
//    Serial.println(input_command.translational, 5);
//    Serial.println(input_command.rotational, 5);
//    Serial.println(input_command.mode);
//
//    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
//    Udp.write((char*)(&cur_info));
//    Udp.endPacket();
//  }
}

void loop()
{
//  if (millis() - info_time >= 100)
//  {
//    cur_info.odo[0] = 3.5;
//    cur_info.imu[0] = 6.1;
//    cur_info.head = 50.0;
//    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
//    Udp.write((char*)(&cur_info));
//    Udp.endPacket();
//    info_time = millis();
//  }
//  delay(1000);

  if (packetSize = receive_udp.parsePacket())
  {
    receive_udp.read((char*)(&input_command), sizeof(command));
    Serial.println(input_command.translational, 5);
    Serial.println(input_command.rotational, 5);
    Serial.println(input_command.mode);

    send_udp.beginPacket(receive_udp.remoteIP(), receive_udp.remotePort());
    send_udp.write((char*)(&cur_info));
    send_udp.endPacket();
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
