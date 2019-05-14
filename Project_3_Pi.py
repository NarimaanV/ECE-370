import socket
from struct import *
from getkey import getkey, keys

UDP_IP = "192.168.1.74"
UDP_REC_PORT = 4242

rec_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # Internet, UDP

rec_sock.connect((UDP_IP, UDP_REC_PORT))

translational = 0.0
angle = 0.0
mode = 0

odo = [0.0, 0.0, 0.0, 0.0]
imu = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
head = 0.0

print_info = False

while True:
    key = getkey()

    if key == keys.UP:
        if translational == 0.0:
            translational = 50.0
        else:
            translational += 25.0
    elif key == keys.DOWN:
        if translational <= 50.0:
            translational = 0.0
        else:
            translational -= 25.0
    elif key == keys.LEFT:
        angle -= 15.0
    elif key == keys.RIGHT:
        angle += 15.0
    elif key == keys.SPACE:
        translational = 0.0
    elif key == 'w':
        angle = 0.0
    elif key == 'a':
        angle = 270.0
    elif key == 's':
        angle = 180.0
    elif key == 'd':
        angle = 90.0
    elif key == 'q':
        print_info = True
    elif key == 'r':
        pass
    else:
        pass

    rec_sock.send(pack("=ddi", translational, angle, mode))
    input_buffer = rec_sock.recv(80)
    (odo[0], odo[1], odo[2], imu[0], imu[1], imu[2], imu[3], imu[4], imu[5],
            head) = unpack("=dddddddddd", input_buffer)
    if print_info == True:
        print "Odometry: x = {}, y = {}, phi = {}".format(odo[0], odo[1],
                odo[2])
        print "IMU Acceleration: x = {} g, y = {} g, z = {} g".format(imu[0],
                imu[1], imu[2])
        print "IMU Magnetism: x = {} gauss, y = {} gauss, z = {} gauss".format(imu[3],
                imu[4], imu[5])
        print "Heading: {} degrees".format(head)
        print_info = False
