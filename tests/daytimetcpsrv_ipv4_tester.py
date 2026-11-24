#!/usr/bin/env python
import socket

IP      = '127.0.0.1'
PORT    = 13
BUFSIZE = 64

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((IP, PORT))
time = s.recv(BUFSIZE)
s.close()

print("Current time is:", time)

