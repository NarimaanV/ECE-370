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

float desired_angle, error, control, K_p = 0.5;

WiFiUDP Udp;
LSM303 compass;
LSM303::vector<int16_t> running_min = {32767, 32767, 32767}, running_max = {-32768, -32768, -32768};

void setup()
{
  pinMode(MOTOR_RIGHT_A, OUTPUT);
  pinMode(MOTOR_RIGHT_B, OUTPUT);
  pinMode(MOTOR_LEFT_A, OUTPUT);
  pinMode(MOTOR_LEFT_B, OUTPUT);
  analogWrite(MOTOR_RIGHT_A, 0);
  analogWrite(MOTOR_RIGHT_B, 0);
  analogWrite(MOTOR_LEFT_A, 0);
  analogWrite(MOTOR_LEFT_B, 0);
  WiFi.setPins(8, 7, 4, 2);
  Serial.begin(9600);
  Wire.begin();
  compass.init();
  compass.enableDefault();
  compass.m_min = (LSM303::vector<int16_t>){-32767, -32767, -32767};
  compass.m_max = (LSM303::vector<int16_t>){+32767, +32767, +32767};
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
}

void loop()
{
  desired_angle = SOUTH;
  
  while (1)
  {
    // Get time at beginning of P controller loop
    tick_time = millis();
    
    int packetSize = Udp.parsePacket();
    compass.read();
    
    running_min.x = min(running_min.x, compass.m.x);
    running_min.y = min(running_min.y, compass.m.y);
    running_min.z = min(running_min.z, compass.m.z);
  
    running_max.x = max(running_max.x, compass.m.x);
    running_max.y = max(running_max.y, compass.m.y);
    running_max.z = max(running_max.z, compass.m.z);
  
    if (packetSize)
    {
      Udp.read((char*)(&input_command), sizeof(command));
      Serial.println(input_command.translational, 5);
      Serial.println(input_command.rotational, 5);
      Serial.println(input_command.mode);
  
      Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
      Udp.write((char*)(&cur_info));
      Udp.endPacket();
    }

    
    error = desired_angle - compass.heading();
    control = error * K_p;
    
    Serial.print(compass.heading());
    Serial.print(" ");
    Serial.println(control);

//    rotate(control);

    // Get time at end of P controller loop
    tock_time = millis();

    // Ensure at least the minimum delay has occured for P controller loop
    delay(desired_control_time - (tock_time - tick_time));
  }
}

void rotate(float omega)
{
//  float s = abs(omega);
//  if (s < 51.0f)
//    s = 51.0f;
//  else if (s > 255.0f)
//    s = 255.0f;
  
  if (omega > 0.0f)
  {
    analogWrite(MOTOR_RIGHT_A, 0);
    analogWrite(MOTOR_LEFT_A, (int)omega);
  }

  else if (omega < 0.0f)
  {
    analogWrite(MOTOR_LEFT_A, 0);
    analogWrite(MOTOR_RIGHT_A, (int)omega);
  }

  else
  {
    analogWrite(MOTOR_RIGHT_A, 0);
    analogWrite(MOTOR_LEFT_A, 0);
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
