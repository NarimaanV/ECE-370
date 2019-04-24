#include <Wire.h>
#include <LSM303.h>

LSM303 compass;

void setup()
{
  Serial.begin(9600);
  Wire.begin();
  compass.init();
  compass.enableDefault();
  compass.m_min = (LSM303::vector<int16_t>){-32767, -32767, -32767};
  compass.m_max = (LSM303::vector<int16_t>){+32767, +32767, +32767};
}

void loop()
{
  compass.read();

  Serial.print("Accelerometer: X = ");
  Serial.print((double)compass.a.x * 0.061 / 1000.0);
  Serial.print("g, Y = ");
  Serial.print((double)compass.a.y * 0.061 / 1000.0);
  Serial.print("g, Z = ");
  Serial.print((double)compass.a.z * 0.061 / 1000.0);
  Serial.print("g        ");
  Serial.print("Magnetometer: X = ");
  Serial.print((double)compass.m.x * 0.160 / 1000.0);
  Serial.print(" gauss, Y = ");
  Serial.print((double)compass.m.y * 0.160 / 1000.0);
  Serial.print(" gauss, Z = ");
  Serial.print((double)compass.m.z * 0.160 / 1000.0);
  Serial.print(" gauss        ");
  Serial.print("Heading: ");
  Serial.println(compass.heading());


  delay(100);
}
