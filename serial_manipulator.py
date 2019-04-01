import numpy as np
from math import sin, cos, pi

# Position matrix values (in AU)
p_x = 0
p_y = 0
p_z = 0

# Length matrix values (in AU)
l_0 = 0.3
l_1 = 0.2
l_2 = 0.1

# Angle matrix values (in radians)
theta_0 = 0.4
theta_1 = 0.6
theta_2 = 1.2

# Print given values
print("Given Values:")
print("\t l_0 = %0.1f AU" % l_0)
print("\t l_1 = %0.1f AU" % l_1)
print("\t l_2 = %0.1f AU" % l_2)
print("\t theta_0 = %0.1f radians" % theta_0)
print("\t theta_1 = %0.1f radians" % theta_1)
print("\t theta_2 = %0.1f radians\n" % theta_2)

#____________________The Long Hand Method____________________________
# First point
e_x0 = l_0 * cos(theta_0)
e_y0 = l_0 * sin(theta_0)
e_z0 = 0

# Second point
e_x1 = e_x0 + l_1 * cos(theta_0 + theta_1)
e_y1 = e_y0 + l_1 * sin(theta_0 + theta_1)
e_z1 = 0

# Third point
e_x2 = e_x1 + l_2 * cos(theta_0 + theta_1 + theta_2)
e_y2 = e_y1 + l_2 * sin(theta_0 + theta_1 + theta_2)
e_z2 = 0

# Total angle
theta_z = theta_0 + theta_1 + theta_2
theta_z *= (180.0 / pi)

# Print final values
print("The Long Hand Method:")
print("\te_x = %0.5f AU" % e_x2)
print("\te_y = %0.5f AU" % e_y2)
print("\te_z = %0.5f AU" % e_z2)
print("\ttheta_x = theta_y = 0 because only moving in XY plane")
print("\ttheta_z = %0.2f degrees\n" % theta_z)

#__________________The Matrix Method_________________________________

# T_0 transformation matrix
T_0 = np.array([[1, 0, 0, p_x], [0, 1, 0, p_y], [0, 0, 1, p_z], [0, 0, 0, 1]])

# T_0_1 transformation matrix
T_0_1 = np.array([[cos(theta_0), -sin(theta_0), 0, 0], \
                  [sin(theta_0), cos(theta_0), 0, 0], \
                  [0, 0, 1, 0], \
                  [0, 0, 0, 1]])
l_0_mat = np.array([[1, 0, 0, l_0], [0, 1, 0, 0], [0, 0, 1, 0], [0, 0, 0, 1]])
T_0_1 = T_0_1 @ l_0_mat

# T_1_2 transformation matrix
T_1_2 = np.array([[cos(theta_1), -sin(theta_1), 0, 0], \
                  [sin(theta_1), cos(theta_1), 0, 0], \
                  [0, 0, 1, 0], \
                  [0, 0, 0, 1]])
l_1_mat = np.array([[1, 0, 0, l_1], [0, 1, 0, 0], [0, 0, 1, 0], [0, 0, 0, 1]])
T_1_2 = T_1_2 @ l_1_mat

#T_2_2 transformation matrix
T_2_3 = np.array([[cos(theta_2), -sin(theta_2), 0, 0], \
                  [sin(theta_2), cos(theta_2), 0, 0], \
                  [0, 0, 1, 0], \
                  [0, 0, 0, 1]])
l_2_mat = np.array([[1, 0, 0, l_2], [0, 1, 0, 0], [0, 0, 1, 0], [0, 0, 0, 1]])
T_2_3 = T_2_3 @ l_2_mat

# T transformation matrix
T = T_0 @ T_0_1 @ T_1_2 @ T_2_3

# Extracting final e_x and e_y values from T
e_x = T[0][3]
e_y = T[1][3]
e_z = T[2][3]

# Calculating final theta trajectory (in degrees)
theta_z = theta_0 + theta_1 + theta_2
theta_z *= (180.0 / pi)

# Set print options to print nicely and print all information
print("The Matrix Method:")
np.set_printoptions(precision = 5, floatmode = "fixed")
print("\tT = " + str(T).replace('\n', "\n\t    "))
print("\te_x = %0.5f AU" % e_x)
print("\te_y = %0.5f AU" % e_y)
print("\te_z = %0.5f AU" % e_z)
print("\ttheta_x = theta_y = 0 because only moving in XY plane")
print("\ttheta_z = %0.2f degrees" % theta_z)
