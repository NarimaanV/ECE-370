import socket
from struct import *
from getkey import getkey, keys

UDP_IP = "192.168.1.74"
UDP_PORT = 4242

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # Internet, UDP

sock.connect((UDP_IP, UDP_PORT))

translational = 0.0
angle = 0.0
mode = 0

odo = [0.0, 0.0, 0.0, 0.0]
imu = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
head = 0.0

print_info = False

print "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"

while True:
    key = getkey()

    if key == keys.UP:
        if translational == 0.0:
            translational = 40.0
        else:
            translational += 15.0
        mode = 0
    elif key == keys.DOWN:
        if translational <= 40.0:
            translational = 0.0
        else:
            translational -= 15.0
        mode = 0
    elif key == keys.LEFT:
        angle -= 15.0
        mode = 0
    elif key == keys.RIGHT:
        angle += 15.0
        mode = 0
    elif key == keys.SPACE:
        translational = 0.0
        mode = 0
    elif key == 'w':
        angle = 0.0
        mode = 1
    elif key == 'a':
        angle = 270.0
        mode = 1
    elif key == 's':
        angle = 180.0
        mode = 1
    elif key == 'd':
        angle = 90.0
        mode = 1
    elif key == 'q':
        print_info = True
    elif key == 'r':
        pass
    else:
        pass

    sock.send(pack("=ddi", translational, angle, mode))
    input_buffer = sock.recv(80)
    (odo[0], odo[1], odo[2], imu[0], imu[1], imu[2], imu[3], imu[4], imu[5],
            head) = unpack("=dddddddddd", input_buffer)

    if print_info == True:
        print "Odometry: x = {} mm, y = {} mm, z = {} mm".format(odo[0], odo[1],
                odo[2])
        print "IMU Acceleration: x = {} m/s^2, y = {} m/s^2, z = {} m/s^2".format(imu[0],
                imu[1], imu[2])
        print "IMU Magnetism: x = {} gauss, y = {} gauss, z = {} gauss".format(imu[3],
                imu[4], imu[5])
        print "Heading: {} degrees\n".format(head)
        print_info = False
