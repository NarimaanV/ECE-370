#define MOTOR_A 12
#define MOTOR_B 11

int set_speed(float s);

void set_velocity(float v);

void setup()
{
  
}

void loop()
{
  
}

// Maps s to corresponding int value on 0-255 scale (clamps values bigger/smaller than 1.0/0.0 to 1.0/0.0, respectively).
int set_speed(float s)
{
  if (s < 0.0f)
    s = 0.0f;
  else if (s > 1.0f)
    s = 1.0f;
  return (int)(s * 255.0f);
}

// Outputs PWM on either MOTOR_A or MOTOR_B pins depending on sign
void set_velocity(float v)
{
  if (v > 0.0f)
  {
    analogWrite(MOTOR_A, set_speed(v));
    analogWrite(MOTOR_B, 0);
  }

  else if (v < 0.0f)
  {
    analogWrite(MOTOR_A, 0);
    analogWrite(MOTOR_B, set_speed(-1.0f * v));
  }

  else
  {
    analogWrite(MOTOR_A, 0);
    analogWrite(MOTOR_B, 0);
  }
}
