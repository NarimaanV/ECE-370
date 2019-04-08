#include <BasicLinearAlgebra.h>

// Constant values for pin numbers and robot dimensions
#define RADIUS          30.0  // Wheel radii in mm
#define BASELINE        80.0  // Transaxial distance between center of each wheels
#define MOTOR_RIGHT     12    // Right motor pin
#define MOTOR_LEFT      11    // Left motor pin
#define IR_RIGHT        6     // Right wheel IR sensor pin
#define IR_LEFT         5     // Left wheel IR sensor pin
#define GEAR_RATIO      75.81 // Gear ratio of motor gearbox (input/output)
#define TICKS_PER_ROT   2     // How many ticks will register per rotation of encoder wheel (depends on both encoder design AND what edge(s) trigger interrupts)

// Preprocessor flag for switching between analytical and matrix methods of odometry
#define MATRIX_METHOD

// Since only one library for matrices is being used, this eliminates needing to preface all matrix-related operations with "BLA::"
using namespace BLA;

// Transformation matrices 
Matrix<4, 4, Array<4, 4, double>> T_right = {1.0, 0.0, 0.0, 0.0, // Right wheel current transformation matrix (identity matrix since initial position is assumed to be x, y, phi = 0.0, 0.0, 0.0)
                                             0.0, 1.0, 0.0, 0.0,
                                             0.0, 0.0, 1.0, 0.0,
                                             0.0, 0.0, 0.0, 1.0},
                                             
                                  T_left = {1.0, 0.0, 0.0, 0.0,  // Left wheel current transformation matrix (identity matrix since initial position is assumed to be x, y, phi = 0.0, 0.0, 0.0)
                                            0.0, 1.0, 0.0, 0.0,
                                            0.0, 0.0, 1.0, 0.0,
                                            0.0, 0.0, 0.0, 1.0},
                                            
                                  translate_right = {1.0, 0.0, 0.0, 0.0, // Right wheel translational transformation matrix (never changes)
                                                     0.0, 1.0, 0.0, -L,
                                                     0.0, 0.0, 1.0, 0.0,
                                                     0.0, 0.0, 0.0, 1.0};

                                  translate_left = {1.0, 0.0, 0.0, 0.0, // Right wheel translational transformation matrix (never changes)
                                                    0.0, 1.0, 0.0, L,
                                                    0.0, 0.0, 1.0, 0.0,
                                                    0.0, 0.0, 0.0, 1.0};

// Current global-referenced values for x, y, and phi (all start at 0.0);
double x_global = 0.0,
       y_global = 0.0,
       phi_global = 0.0;

// Phi angle radians per single tick
// (RADIUS/BASELINE) phi radians/wheel radians 
// 2pi wheel radians/wheel rotation
// 1/GEAR_RATIO wheel rotations/encoder rotation
// 1/TICKS_PER_ROT encoder rotation/tick
// So phi radians / tick = ((RADIUS/BASELINE) phi radians/wheel radians)*(2pi wheel radians/wheel rotation)*(1/GEAR_RATIO wheel rotations/encoder rotation)*(1/TICKS_PER_ROT encoder rotation/tick)
const double phi_radians_per_tick = (RADIUS / BASELINE) * (2.0 * PI) * (1.0 / 75.81) * (1.0 / TICKS_PER_ROT);

void setup()
{
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
}

void loop()
{
  // Do robot stuff
}

// ISR for calculating odometry of right wheel based on IR sensor signal using analytical method
void analytical_odometry_right()
{
  double delta_x, delta_y;  // Local delta x and delta y values
  
  phi_global += phi_radians_per_tick; // Calculate new global phi angle based on ratio between phi radians and ticks
  
  delta_x = (BASELINE / 2.0) * sin(phi_radians_per_tick); // Calculate local delta x based on local phi
  delta_y = (BASELINE / 2.0) - ((BASELINE / 2.0) * cos(phi_radians_per_tick)); // Calculate local delta y based on local phi
  
  x_global += (delta_x * cos(phi_global)) + (delta_y * sin(phi_global));  // Update global x based on global phi and local delta x and y
  y_global += (delta_x * sin(phi_global)) + (delta_y * cos(phi_global));  // Update global y based on global phi and local delta x and y
}

// ISR for calculating odometry of left wheel based on IR sensor signal using analytical method
void analytical_odometry_left()
{
  double delta_x, delta_y;  // Local delta x and delta y values
  
  phi_global -= phi_radians_per_tick; // Calculate new global phi angle based on ratio between phi radians and ticks
  
  delta_x = (BASELINE / 2.0) * sin(phi_radians_per_tick); // Calculate local delta x based on local phi
  delta_y = (BASELINE / 2.0) - ((BASELINE / 2.0) * cos(phi_radians_per_tick)); // Calculate local delta y based on local phi
  
  x_global += (delta_x * cos(phi_global)) + (delta_y * sin(phi_global));  // Update global x based on global phi and local delta x and y
  y_global -= (delta_x * sin(phi_global)) + (delta_y * cos(phi_global));  // Update global y based on global phi and local delta x and y
}

// ISR for calculating odometry of right wheel based on IR sensor signal using matrix method
void matrix_odometry_right()
{
  
}

// ISR for calculating odometry of left wheel based on IR sensor signal using matrix method
void matrix_odometry_left()
{
  
}
