#define MOTOR_A 12
#define MOTOR_B 11
#define IR_A 9
#define IR_B 10
#define KP 4

/*
 * 4 IR ticks/input_revolution
 * 75.81 input_revolution/output_revolution
 * 360 degrees/output_revolution
 * Therefore, (4 ticks/input_rev * 75.81 input_rev/output_rev)/360 degrees/output_rev = 0.8423 ticks/degree
 * and 360 degrees/output_rev/(4 ticks/input_rev * 75.81 input_rev/output_rev) = 1.1872 degrees/tick
 */
const double ticks_per_degree = (4.0 * 75.81) / 360.0,  // 0.8423
             degrees_per_tick = 360.0 / (4.0 * 75.81);  // 1.1872
long desired_angle, angle, desired_ticks, ticks, error;
unsigned long tick_time, tock_time, desired_control_time = 20000;
short new_input = 1;

int set_speed(float s);
void set_velocity(float v);

enum dir
{
  CW = -1,
  CCW = 1
} volatile dir_cur;

struct state
{
  short a;
  short b;
} state_prev, state_cur;

void setup()
{
  Serial.begin(9600);
  pinMode(MOTOR_A, OUTPUT);
  pinMode(MOTOR_B, OUTPUT);
  pinMode(IR_A, INPUT);
  pinMode(IR_B, INPUT);
  attachInterrupt(digitalPinToInterrupt(IR_A), ir_isr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(IR_B), ir_isr, CHANGE);
  state_prev.a = digitalRead(IR_A);
  state_prev.b = digitalRead(IR_B);
}

void loop()
{
  while(1)
  {
    tick_time = micros();
    if (new_input)
    {
      if (Serial.available())
        desired_angle = Serial.parseInt();
      desired_ticks = (int)((double)desired_angle * ticks_per_degree);
      ticks = 0;
      new_input = 0;
    }

    else
    {
      error = desired_ticks - ticks;
      set_velocity(error*KP*dir_cur);
      if (error < 5)
        new_input = 1;
    }
    tock_time = micros();
    delayMicroseconds(desired_control_time - (tock_time - tick_time));
  }
}

// Maps s to corresponding int value on 0-255 scale (clamps values bigger/smaller than 1.0/0.5 to 1.0/0.5, respectively).
int set_speed(float s)
{
  if (s < 0.5f)
    s = 0.5f;
  else if (s > 1.0f)
    s = 1.0f;
  return (int)(s * 255.0f);
}

// Outputs PWM on either MOTOR_A or MOTOR_B pins depending on sign
void set_velocity(float v)
{
  if (v < 0.0f)
  {
    analogWrite(MOTOR_A, set_speed(-1.0f * v));
    analogWrite(MOTOR_B, 0);
  }

  else if (v > 0.0f)
  {
    analogWrite(MOTOR_A, 0);
    analogWrite(MOTOR_B, set_speed(v));
  }

  else
  {
    analogWrite(MOTOR_A, 0);
    analogWrite(MOTOR_B, 0);
  }
}

void ir_isr()
{
  state_prev.a = state_cur.a;
  state_prev.b = state_cur.b;
  state_cur.a = digitalRead(IR_A);
  state_cur.b = digitalRead(IR_B);
  if (desired_ticks < 0.0)
    ticks--;
  else
    ticks++;

  if (!state_prev.a && !state_prev.b)
  {
    if (!state_cur.a && state_cur.b)
      dir_cur = CCW;
    else
      dir_cur = CW;
  }

  else if (!state_prev.a && state_prev.b)
  {
    if (state_cur.a && state_cur.b)
      dir_cur = CCW;
    else
      dir_cur = CW;
  }

  else if (state_prev.a && !state_prev.b)
  {
    if (!state_cur.a && !state_cur.b)
      dir_cur = CCW;
    else
      dir_cur = CW;
  }

  else
  {
    if (state_cur.a && !state_cur.b)
      dir_cur = CCW;
    else
      dir_cur = CW;
  }
}
