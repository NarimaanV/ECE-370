from socket import *
from struct import *
import pygame

UDP_IP = "192.168.1.74"
UDP_PORT = 5005

translate = 0.0
rotate = 0.0
mode = 0

x = 0.0
y = 0.0
phi = 0.0

pygame.init()
screen = pygame.display.set_mode((1,1))
sock = socket(AF_INET, SOCK_DGRAM) # Internet, UDP

def sendCommand(translate, rotate, mode):
    # Pack inputs into packed command struct that's float, float, int
    command = pack("=ffi", float(translate), float(rotate), int(mode))

    # Send bytes from struct through UDP socket/port
    sock.sendto(command, (UDP_IP, UDP_PORT))

    return

def recvInfo():
    # Wait to recieve current position information struct from UDP socket/port
    info, addr = sock.recvfrom(12)

    # Unpack current position information struct into 3 individual floats
    x, y, phi = unpack("=fff", info)

    return

while True:
    # Store 3 inputs from command line (float float int)
    #translate, rotate, mode = input().split(' ')
    
    for event in pygame.event.get():
        if event.type == pygame.KEYDOWN:
            if event.key == pygame.K_UP:
                translate += 10.0
                sendCommand(translate, rotate, mode)
                recvInfo()
            if event.key == pygame.K_DOWN and translate >= 10.0:
                translate -= 10.0
                sendCommand(translate, rotate, mode)
                recvInfo()
            if event.key == pygame.K_LEFT:
                rotate += 10.0
                sendCommand(translate, rotate, mode)
                recvInfo()
            if event.key == pygame.K_RIGHT:
                rotate -= 10.0
                sendCommand(translate, rotate, mode)
                recvInfo()
            if event.key == pygame.K_q:
                print("Current X: {}, Current Y: {}, Current Phi: {}".format(x,
                    y, phi))
    pygame.event.pump()
