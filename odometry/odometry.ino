// Constant values for pin numbers and robot dimensions
#define RADIUS      30  // Wheel radii in mm
#define BASELINE    80  // Transaxial distance between center of each wheels
#define MOTOR_RIGHT 12  // Right motor pin
#define MOTOR_LEFT  11  // Left motor pin
#define IR_RIGHT    6   // Right wheel IR sensor pin
#define IR_LEFT     5   // Left wheel IR sensor pin

// Preprocessor flag for switching between analytical and matrix methods of odometry
#define MATRIX_METHOD

void setup()
{
  // Set pin directions for motors and IR sensors
  pinMode(MOTOR_RIGHT, OUTPUT);
  pinMode(MOTOR_LEFT, OUTPUT);
  pinMode(IR_RIGHT, INPUT);
  pinMode(IR_LEFT, INPUT);

  // Choose ISRs for certain method dependong in preprocessor flag
  #ifdef MATRIX_METHOD
    attachInterrupt(digitalPinToInterrupt(IR_RIGHT), matrix_odometry_right, CHANGE);
    attachInterrupt(digitalPinToInterrupt(IR_LEFT), matrix_odometry_left, CHANGE);
  #else
    attachInterrupt(digitalPinToInterrupt(IR_RIGHT), analytical_odometry_right, CHANGE);
    attachInterrupt(digitalPinToInterrupt(IR_LEFT), analytical_odometry_left, CHANGE);
  #endif
}

void loop()
{
  // Do robot stuff
}

// ISR for calculating odometry of right wheel based on IR sensor signal using analytical method
void analytical_odometry_right()
{
  
}

// ISR for calculating odometry of left wheel based on IR sensor signal using analytical method
void analytical_odometry_left()
{
  
}

// ISR for calculating odometry of right wheel based on IR sensor signal using matrix method
void matrix_odometry_right()
{
  
}

// ISR for calculating odometry of left wheel based on IR sensor signal using matrix method
void matrix_odometry_left()
{
  
}
