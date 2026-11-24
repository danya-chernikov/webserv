#!/usr/bin/env python
import socket
import sys

if len(sys.argv) != 3:
    print("Usage: ", sys.argv[0], "<IP_address> <Port>")

IP      = sys.argv[1]
PORT    = int(sys.argv[2])
BUFSIZE = 64

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((IP, PORT))
time = s.recv(BUFSIZE)
s.close()

print("Current time is:", time)
