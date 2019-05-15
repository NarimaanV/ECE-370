import socket
from struct import *
from getkey import getkey, keys

UDP_IP = "192.168.1.74"
UDP_PORT = 4242

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # Internet, UDP

sock.connect((UDP_IP, UDP_PORT))

translational = 0.0
rotational = 0.0
mode = 0

x = 0.0
y = 0.0
phi = 0.0

print_info = False

print "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"

while True:
    key = getkey()

    if key == keys.UP:
        if translational == 0.0:
            translational = 1500.0
        else:
            translational += 1000.0
    elif key == keys.DOWN:
        if translational <= 1500.0:
            translational = 0.0
        else:
            translational -= 1000.0
    elif key == keys.LEFT:
        rotational -= 1.0
    elif key == keys.RIGHT:
        rotational += 1.0
    elif key == 'q':
        print_info = True
    else:
        pass

    sock.send(pack("=ddi", translational, rotational, mode))
    input_buffer = sock.recv(24)
    (x, y, phi) = unpack("=ddd", input_buffer)

    if print_info == True:
        print "Heading: {2:.2f} degrees".format(x, y, phi)

        print_info = False

