#include <SPI.h>
#include <WiFi101.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <LSM303.h>
#include <math.h>

#define MOTOR_RIGHT_FORWARD   12    // Right motor pin
#define MOTOR_RIGHT_REVERSE   11    // Right motor pin
#define MOTOR_LEFT_FORWARD    10    // Left motor pin
#define MOTOR_LEFT_REVERSE    9    // Left motor pin
#define NORTH           0.0f  // North heading
#define SOUTH           180.0f  // South heading
#define EAST            90.0f  // East heading
#define WEST            270.0f  // West heading

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
unsigned long tick_time, tock_time, desired_control_time = 20;

float desired_angle, error, control, K_p = 4.0;
float angles[5] = {0.0, 180.0, 90.0, 270.0, 0.0}; // North, South, East, West, North
unsigned long time_a, time_b;
unsigned int current = 0;

WiFiUDP Udp;
LSM303 compass;

void setup()
{
  pinMode(MOTOR_RIGHT_FORWARD, OUTPUT);
  pinMode(MOTOR_RIGHT_REVERSE, OUTPUT);
  pinMode(MOTOR_LEFT_FORWARD, OUTPUT);
  pinMode(MOTOR_LEFT_REVERSE, OUTPUT);
  analogWrite(MOTOR_RIGHT_FORWARD, 0);
  analogWrite(MOTOR_RIGHT_REVERSE, 0);
  analogWrite(MOTOR_LEFT_FORWARD, 0);
  analogWrite(MOTOR_LEFT_REVERSE, 0);
  WiFi.setPins(8, 7, 4, 2);
  Serial.begin(9600);
  Wire.begin();
  compass.init();
  compass.enableDefault();
  compass.m_min = (LSM303::vector<int16_t>){-1415, -3237, -5057};
  compass.m_max = (LSM303::vector<int16_t>){+5450, +2711, +1409};
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

  time_a = millis();
  time_b = millis();
}

void loop()
{
  
  
  while (1)
  {
    // Get time at beginning of P controller loop
    tick_time = millis();
    time_b = millis();
    if (time_b - time_a >= 10000 && current <= 4)
    {
      time_a = time_b;
      current++;
    }

    desired_angle = angles[current];
    
    int packetSize = Udp.parsePacket();
    compass.read();
    cur_info.phi = compass.heading();
    
    if (packetSize)
    {
      Udp.read((char*)(&input_command), sizeof(command));
//      Serial.println(input_command.translational, 5);
//      Serial.println(input_command.rotational, 5);
//      Serial.println(input_command.mode);
  
      Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
      Udp.write((char*)(&cur_info));
      Udp.endPacket();
    }

    error = abs(desired_angle - compass.heading());

    if (error > 180.0f)
      error = 360.0f - error;
    else
      error = desired_angle - compass.heading();
    
    control = error * K_p;

    Serial.print(desired_angle);
    Serial.print(" ");
    Serial.print(compass.heading());
    Serial.print(" ");
    Serial.println(error);

    rotate(control);

    // Get time at end of P controller loop
    tock_time = millis();

    // Ensure at least the minimum delay has occured for P controller loop
    delay(desired_control_time - (tock_time - tick_time));
  }
}

void rotate(float omega)
{
  float s = abs(omega);
  if (s < 40.0f)
    s = 40.0f;
  else if (s > 255.0f)
    s = 255.0f;
  
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
