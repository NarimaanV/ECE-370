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
  double heading;
} cur_info = {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0, 0.0, 0.0}, 0.0};

char ssid[] = "Narimaan-M0";
int status = WL_IDLE_STATUS;

unsigned long info_time;

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
  status = WiFi.beginAP(ssid);
  if (status != WL_AP_LISTENING) {
    Serial.println("Creating access point failed");
    // don't continue
    while (true);
  }

  info_time = millis();
}

void loop()
{

}
