#! /usr/bin/python

import sys
import struct
import socket
import telnetlib

if sys.byteorder == 'little':
	Q = lambda x: struct.pack("<I", x)
	rQ = lambda x: struct.unpack("<I", x)[0]
elif sys.byteorder == 'big':
	Q = lambda x: struct.pack(">I", x)
	rQ = lambda x: struct.unpack(">I", x)[0]

try:
	s = socket.create_connection(('vortex.labs.overthewire.org', 5842))
except socket.gaierror as e:
	(n, q) = e

	sys.exit(-1)

try:
	tuple = [];

	for i in range(4):
		tuple.append(rQ(s.recv(4)))
except socket.error as e:
	(n, q) = e

	sys.exit(-1)

try:
	s.send(Q(reduce(lambda x, y: x + y, tuple)) + chr(0x0a))
except socket.error as e:
	(n, q) = e

	sys.exit(-1)

t = telnetlib.Telnet()
t.sock = s
t.interact()
