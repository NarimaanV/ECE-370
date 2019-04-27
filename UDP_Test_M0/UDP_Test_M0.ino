#include <SPI.h>
#include <WiFi101.h>
#include <WiFiUdp.h>
#include <math.h>

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

WiFiUDP Udp;

void setup()
{
  pinMode(12, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(9, OUTPUT);
  WiFi.setPins(8, 7, 4, 2);
  Serial.begin(9600);
  while (!Serial);
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
  int packetSize = Udp.parsePacket();

  if (packetSize)
  {
    Udp.read((char*)(&input_command), sizeof(command));
    Serial.println(input_command.translational, 5);
    Serial.println(input_command.rotational, 5);
    Serial.println(input_command.mode);
    
    cur_info.x = 1.1;
    cur_info.y = 2.2;
    cur_info.phi = 3.3;

    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write((char*)(&cur_info));
    Udp.endPacket();
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
