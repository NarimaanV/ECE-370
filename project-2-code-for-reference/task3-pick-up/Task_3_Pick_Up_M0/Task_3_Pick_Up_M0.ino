#include <SPI.h>
#include <WiFi101.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <LSM303.h>
#include <math.h>

#define MOTOR_RIGHT_A   12    // Right motor pin
#define MOTOR_RIGHT_B   11    // Right motor pin
#define MOTOR_LEFT_A    10    // Left motor pin
#define MOTOR_LEFT_B    9    // Left motor pin

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
  float x;
  float y;
  float phi;
} cur_info;

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
int status = WL_IDLE_STATUS;     // the WiFi radio's status

unsigned int localPort = 5005;      // local port to listen on

float acc_z;

WiFiUDP Udp;
LSM303 compass;

void setup()
{
  pinMode(MOTOR_RIGHT_A, OUTPUT);
  pinMode(MOTOR_RIGHT_B, OUTPUT);
  pinMode(MOTOR_LEFT_A, OUTPUT);
  pinMode(MOTOR_LEFT_B, OUTPUT);
  analogWrite(MOTOR_RIGHT_A, 125);
  analogWrite(MOTOR_RIGHT_B, 0);
  analogWrite(MOTOR_LEFT_A, 0);
  analogWrite(MOTOR_LEFT_B, 0);
//  WiFi.setPins(8, 7, 4, 2);
  Serial.begin(9600);
  Wire.begin();
  compass.init();
  compass.enableDefault();
  compass.m_min = (LSM303::vector<int16_t>){-32767, -32767, -32767};
  compass.m_max = (LSM303::vector<int16_t>){+32767, +32767, +32767};
//  while (!Serial);
//  while ( status != WL_CONNECTED)
//  {
//    Serial.print("Attempting to connect to WPA SSID: ");
//    Serial.println(ssid);
//    // Connect to WPA/WPA2 network:
//    status = WiFi.begin(ssid, pass); 
//
//    // wait 10 seconds for connection:
//    delay(5000);
//  }
//
//  Serial.print("You're connected to the network");
//  printCurrentNet();
//  printWiFiData();
//
//  Serial.println("\nStarting connection to server...");
//  // if you get a connection, report back via serial:
//  Udp.begin(localPort);
//  delay(3000);
}

void loop()
{
//  int packetSize = Udp.parsePacket();
  compass.read();
  acc_z = (float)compass.a.z * 0.061 / 1000.0;
  Serial.println(acc_z);

  if (acc_z > 1.15f)
  {
    Serial.println("Triggered!");
    analogWrite(MOTOR_RIGHT_A, 0);
  }
    

//  if (packetSize)
//  {
//    Udp.read((char*)(&input_command), sizeof(command));
//    Serial.println(input_command.translational, 5);
//    Serial.println(input_command.rotational, 5);
//    Serial.println(input_command.mode);
//    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
//    Udp.write((char*)(&cur_info));
//    Udp.endPacket();
//  }

  delay(50);
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
