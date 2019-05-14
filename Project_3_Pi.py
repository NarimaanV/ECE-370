import socket
from ctypes import *

UDP_IP = "192.168.1.74"
UDP_SEND_PORT = 5005
UDP_REC_PORT = 4242

send_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # Internet, UDP
rec_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # Internet, UDP

send_sock.connect((UDP_IP, UDP_SEND_PORT))
rec_sock.connect((UDP_IP, UDP_REC_PORT))

class Command(Structure):
    _pack_ = 1
    _fields_ = [
            ("translational", c_double),
            ("angle", c_double),
            ("mode", c_int)
            ]

class Robot_Info(Structure):
    _pack_ = 1
    _fields_ = [
            ("odo", c_double * 3),
            ("imu", c_double * 6),
            ("head", c_double)
            ]

input_command = None
cur_info = None

while True:
    input_buffer, addr = rec_sock.recvfrom(sizeof(Robot_Info))
    cur_info = Robot_Info.from_buffer_copy(input_buffer)
    print("{}, {}, {}, {}, {}, {}, {}, {}, {}, {}".format(cur_info.odo[0],
        cur_info.odo[1], cur_info.odo[2], cur_info.imu[0], cur_info.imu[1],
        cur_info.imu[2], cur_info.imu[3], cur_info.imu[4], cur_info.imu[5],
        cur_info.head))
    #print("received!")
