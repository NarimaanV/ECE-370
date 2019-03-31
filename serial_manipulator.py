import numpy as np
from math import sin, cos, atan

p_x = 0
p_y = 0
p_z = 0

l_0 = 0.3
l_1 = 0.2
l_2 = 0.1

theta_0 = 0.4
theta_1 = 0.6
theta_2 = 1.2

T_0 = np.array([[1, 0, 0, p_x], [0, 1, 0, p_y], [0, 0, 1, p_z], [0, 0, 0, 1]])

T_0_1 = np.array([[cos(theta_0), -sin(theta_0), 0, 0], \
                  [sin(theta_0), cos(theta_0), 0, 0], \
                  [0, 0, 1, 0], \
                  [0, 0, 0, 1]])
l_0_mat = np.array([[1, 0, 0, l_0], [0, 1, 0, 0], [0, 0, 1, 0], [0, 0, 0, 1]])
T_0_1 = T_0_1 @ l_0_mat

T_1_2 = np.array([[cos(theta_1), -sin(theta_1), 0, 0], \
                  [sin(theta_1), cos(theta_1), 0, 0], \
                  [0, 0, 1, 0], \
                  [0, 0, 0, 1]])
l_1_mat = np.array([[1, 0, 0, l_1], [0, 1, 0, 0], [0, 0, 1, 0], [0, 0, 0, 1]])
T_1_2 = T_1_2 @ l_1_mat

T_2_3 = np.array([[cos(theta_2), -sin(theta_2), 0, 0], \
                  [sin(theta_2), cos(theta_2), 0, 0], \
                  [0, 0, 1, 0], \
                  [0, 0, 0, 1]])
l_2_mat = np.array([[1, 0, 0, l_2], [0, 1, 0, 0], [0, 0, 1, 0], [0, 0, 0, 1]])
T_2_3 = T_2_3 @ l_2_mat

T = T_0 @ T_0_1 @ T_1_2 @ T_2_3
e_x = T[0][3]
e_y = T[1][3]
theta = atan(e_y/e_x)

print("T =")
print(T)
print("e_x = " + str(e_x))
print("e_y = " + str(e_y))
print("theta = " + str(theta))
