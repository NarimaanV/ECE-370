#define MOTOR_A 12
#define MOTOR_B 11
#define IR_A 9
#define IR_B 10

/*
 * 4 IR ticks/input_revolution
 * 75.81 input_revolution/output_revolution
 * 360 degrees/output_revolution
 * Therefore, (4 ticks/input_rev * 75.81 input_rev/output_rev)/360 degrees/output_rev = 0.8423 ticks/degree
 * and 360 degrees/output_rev/(4 ticks/input_rev * 75.81 input_rev/output_rev) = 1.1872 degrees/tick
 */
const double ticks_per_degree = (8.0 * 75.81) / 360.0,  // 0.8423
             degrees_per_tick = 360.0 / (8.0 * 75.81),  // 1.1872
             K_p = -1;

long  desired_ticks, ticks = 0, error, cur_speed;
unsigned long tick_time, tock_time, desired_control_time = 50000, i_wait, i_wait2 = 0, angle_pos = 0;
short new_input = 1;

short angles[3] = {90.0, 135.0, 0.0};

float desired_angle = 0.0, angle;

int set_speed(float s);
void set_velocity(float v);

// Enumeration of possible rotation directions
volatile enum dir
{
  CW = -1,
  CCW = 1
} dir_cur;

// Struct to store values of both IR sensors for a given state
struct state
{
  short a;
  short b;
} state_prev, state_cur;

void setup()
{
//  Serial.begin(9600);
  pinMode(MOTOR_A, OUTPUT);
  pinMode(MOTOR_B, OUTPUT);
  pinMode(IR_A, INPUT);
  pinMode(IR_B, INPUT);
  attachInterrupt(digitalPinToInterrupt(IR_A), ir_isr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(IR_B), ir_isr, CHANGE);
  state_cur.a = digitalRead(IR_A);
  state_cur.b = digitalRead(IR_B);
}

void loop()
{
  while(1)
  {
    // Get time at beginning of P controller loop
    tick_time = micros();

//    if (Serial.available())
//      desired_angle = Serial.parseFloat();
    i_wait2++;
    if (i_wait2 == 200)
    {
      i_wait2 = 0;
      desired_angle = angles[angle_pos % 3];
      angle_pos++;
    }

    angle = ticks * degrees_per_tick;
    
//    set_velocity(0.0);

    float e = desired_angle - angle;
    float c = K_p * e;
    
    set_velocity(c);
    i_wait++;
//    if (i_wait == 10)
//    {
//      i_wait = 0;
//      Serial.print(angle);
//      Serial.print(" ");
//      Serial.print(c);
//      Serial.println();
//    }
//    Serial.print(angle);
//    Serial.print(" ");
//    Serial.print(c);
//    Serial.println();

    // Get time at end of P controller loop
    tock_time = micros();

    // Ensure at least the minimum delay has occured for P controller loop
//    delayMicroseconds(desired_control_time - (tock_time - tick_time));
    delay(50);
  }
}

// Maps s to corresponding int value on 0-255 scale (clamps values bigger/smaller than 1.0/0.5 to 1.0/0.5, respectively).
int set_speed(float s)
{
  if (s < 0.0f)
    s = 0.0f;
  else if (s > 1.0f)
    s = 1.0f;
//  Serial.println((int)(s * 255.0f));
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

// ISR for every edge of the IR sensors that also uses a state machine to determine current direction.
void ir_isr()
{
  // Shift current state to previous state
  state_prev.a = state_cur.a;
  state_prev.b = state_cur.b;

  // Save new current state
  state_cur.a = digitalRead(IR_A);
  state_cur.b = digitalRead(IR_B);
  if (dir_cur == CW)
    ticks++;
  else
    ticks--;

  if (!state_prev.a && !state_prev.b)
  {
    if (!state_cur.a && state_cur.b)
      dir_cur = CW;
    else
      dir_cur = CCW;
  }

  else if (!state_prev.a && state_prev.b)
  {
    if (state_cur.a && state_cur.b)
      dir_cur = CW;
    else
      dir_cur = CCW;
  }

  else if (state_prev.a && !state_prev.b)
  {
    if (!state_cur.a && !state_cur.b)
      dir_cur = CW;
    else
      dir_cur = CCW;
  }

  else
  {
    if (state_cur.a && !state_cur.b)
      dir_cur = CW;
    else
      dir_cur = CCW;
  }
}
