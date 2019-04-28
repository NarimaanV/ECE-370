#include <BasicLinearAlgebra.h>

// Constant values for pin numbers and robot dimensions
#define RADIUS          15.7f // Wheel radii in mm 31.4/2
#define BASELINE        82.4f // Transaxial distance between center of each wheels 82.4
#define GEAR_RATIO      75.81f // Gear ratio of motor gearbox (input/output) 75.81
#define TICKS_PER_ROT   2.0f  // How many ticks will register per rotation of encoder wheel (depends on both encoder design AND what edge(s) trigger interrupts)
#define MOTOR_RIGHT     12    // Right motor pin
#define MOTOR_LEFT      11    // Left motor pin
#define IR_RIGHT        6     // Right wheel IR sensor pin
#define IR_LEFT         5     // Left wheel IR sensor pin

// Preprocessor flag for switching between analytical and matrix methods of odometry (comment out for analytical method)
#define MATRIX_METHOD

// Since only one library for matrices is being used, this eliminates needing to preface all matrix-related operations with "BLA::"
using namespace BLA;

// Current global-referenced values for x, y, and phi (all start at 0.0);
float x_global = 0.0f,
       y_global = 0.0f,
       phi_global = 0.0f;

// Phi angle radians per single tick
// (RADIUS/BASELINE) phi radians/wheel radians 
// 2pi wheel radians/wheel rotation
// 1/GEAR_RATIO wheel rotations/encoder rotation
// 1/TICKS_PER_ROT encoder rotation/tick
// So phi radians / tick = ((RADIUS/BASELINE) phi radians/wheel radians)*(2pi wheel radians/wheel rotation)*(1/GEAR_RATIO wheel rotations/encoder rotation)*(1/TICKS_PER_ROT encoder rotation/tick)
const float phi_radians_per_tick = (RADIUS / BASELINE) * (2.0f * PI) * (1.0f / 75.81f) * (1.0f / TICKS_PER_ROT);

// Transformation matrices 
Matrix<4, 4> T = {1.0, 0.0, 0.0, 0.0, // Current transformation matrix between global origin and robot center (identity matrix since initial position is assumed to be x, y, phi = 0.0, 0.0, 0.0)
                  0.0, 1.0, 0.0, 0.0,
                  0.0, 0.0, 1.0, 0.0,
                  0.0, 0.0, 0.0, 1.0},
                                            
                                  translate_right = {1.0, 0.0, 0.0, 0.0, // Right wheel translational transformation matrix (never changes)
                                                     0.0, 1.0, 0.0, -BASELINE,
                                                     0.0, 0.0, 1.0, 0.0,
                                                     0.0, 0.0, 0.0, 1.0},

                                  translate_left = {1.0, 0.0, 0.0, 0.0, // Left wheel translational transformation matrix (never changes)
                                                    0.0, 1.0, 0.0, BASELINE,
                                                    0.0, 0.0, 1.0, 0.0,
                                                    0.0, 0.0, 0.0, 1.0},

                                  rotate_right = {cos(phi_radians_per_tick), -sin(phi_radians_per_tick), 0.0, 0.0,  // Right wheel rotational transformation matrix (never changes)
                                                  sin(phi_radians_per_tick), cos(phi_radians_per_tick),  0.0, 0.0,
                                                  0.0,                       0.0,                        1.0, 0.0,
                                                  0.0,                       0.0,                        0.0, 1.0},
                                                    
                                  rotate_left = {cos(-phi_radians_per_tick), -sin(-phi_radians_per_tick), 0.0, 0.0,   // Left wheel rotational transformation matrix (never changes)
                                                 sin(-phi_radians_per_tick), cos(-phi_radians_per_tick),  0.0, 0.0,
                                                 0.0,                        0.0,                        1.0, 0.0,
                                                 0.0,                        0.0,                        0.0, 1.0};

void setup()
{
  Serial.begin(9600);
  // Set pin directions for motors and IR sensors
  pinMode(MOTOR_RIGHT, OUTPUT);
  pinMode(MOTOR_LEFT, OUTPUT);
  pinMode(IR_RIGHT, INPUT);
  pinMode(IR_LEFT, INPUT);

  // Choose ISRs for certain method dependong in preprocessor flag
  #ifdef MATRIX_METHOD
    attachInterrupt(digitalPinToInterrupt(IR_RIGHT), matrix_odometry_right, RISING);
    attachInterrupt(digitalPinToInterrupt(IR_LEFT), matrix_odometry_left, RISING);
  #else
    attachInterrupt(digitalPinToInterrupt(IR_RIGHT), analytical_odometry_right, RISING);
    attachInterrupt(digitalPinToInterrupt(IR_LEFT), analytical_odometry_left, RISING);
  #endif  // MATRIX_METHOD

//  delay(5000);
//  char out[50];
//  
//  Serial.print("\nMatrix Initial: X = ");
//  Serial.print(x_global, 5);
//  Serial.print(", Y = ");
//  Serial.print(y_global, 5);
//  Serial.print(", Phi = ");
//  Serial.println(phi_global, 5);
//  for (int i = 0; i < 80; i++)
//  {
//    matrix_odometry_right();
////    matrix_odometry_left();
//  }
//  Serial.print("Matrix Final: X = ");
//  Serial.print(x_global, 5);
//  Serial.print(", Y = ");
//  Serial.print(y_global, 5);
//  Serial.print(", Phi = ");
//  Serial.println(phi_global, 5);
}

void loop()
{
  Serial.print("Current: X = ");
  Serial.print(x_global, 5);
  Serial.print(", Y = ");
  Serial.print(y_global, 5);
  Serial.print(", Phi = ");
  Serial.println(phi_global, 5);

  delay(500);
}

// ISR for calculating odometry of right wheel based on IR sensor signal using analytical method
void analytical_odometry_right()
{
  float delta_x, delta_y;  // Local delta x and delta y values
  float temp_x, temp_y;
  
  phi_global += phi_radians_per_tick; // Calculate new global phi angle based on ratio between phi radians and ticks
  
  delta_x = (BASELINE / 2.0) * sin(phi_radians_per_tick); // Calculate local delta x based on local phi
  delta_y = (BASELINE / 2.0) - ((BASELINE / 2.0) * cos(phi_radians_per_tick)); // Calculate local delta y based on local phi
  
  x_global += (delta_x * cos(phi_global)) + (delta_y * sin(phi_global));  // Update global x based on global phi and local delta x and y
  y_global += (delta_x * sin(phi_global)) + (delta_y * cos(phi_global));  // Update global y based on global phi and local delta x and y
}

// ISR for calculating odometry of left wheel based on IR sensor signal using analytical method
void analytical_odometry_left()
{
  float delta_x, delta_y;  // Local delta x and delta y values
  
  phi_global -= phi_radians_per_tick; // Calculate new global phi angle based on ratio between phi radians and ticks
  
  delta_x = (BASELINE / 2.0) * sin(phi_radians_per_tick); // Calculate local delta x based on local phi
  delta_y = (BASELINE / 2.0) - ((BASELINE / 2.0) * cos(phi_radians_per_tick)); // Calculate local delta y based on local phi
  
  x_global += (delta_x * cos(phi_global)) + (delta_y * sin(phi_global));  // Update global x based on global phi and local delta x and y
  y_global -= (delta_x * sin(phi_global)) + (delta_y * cos(phi_global));  // Update global y based on global phi and local delta x and y
}

// ISR for calculating odometry of right wheel based on IR sensor signal using matrix method
void matrix_odometry_right()
{
  T = T * rotate_right * translate_right; // Multiply transformation matrices to update current global x and y positions
  x_global = T(0, 3); // Assign global x based on value in new transformation matrix
  y_global = T(1, 3); // Assign global y based on value in new transformation matrix
  phi_global += phi_radians_per_tick; // Calculate new global phi angle based on ratio between phi radians and ticks
}

// ISR for calculating odometry of left wheel based on IR sensor signal using matrix method
void matrix_odometry_left()
{
  T = T * rotate_left * translate_left; // Multiply transformation matrices to update current global x and y positions
  x_global = T(0, 3); // Assign global x based on value in new transformation matrix
  y_global = T(1, 3); // Assign global y based on value in new transformation matrix
  phi_global -= phi_radians_per_tick; // Calculate new global phi angle based on ratio between phi radians and ticks
}
