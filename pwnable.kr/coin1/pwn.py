#! /usr/bin/python

import sys
import re
import socket
import telnetlib

N = 0
C = 0
left, right, mid = 0, 0, 0

try:
	s = socket.create_connection(("pwnable.kr", 9007))
except socket.gaierror as e:
	(n, q) = e

	sys.exit(-1)

# trash first output buffer
try:
	s.recv(8192)
except socket.error as e:
	(n, q) = e

	sys.exit(-1)


while True:
	try:
		curr_buf = s.recv(8192).rstrip(chr(0x0a))
	except socket.error as e:
		(n, q) = e

		sys.exit(-1)

	if "expire" in curr_buf:
		break

	if not(curr_buf.isdigit() or curr_buf.startswith("N=")):
		print '|', curr_buf

	if curr_buf.startswith("N="):
		N = int(curr_buf[curr_buf.index('=') + 1: curr_buf.index(' ')])
		C = int(curr_buf[curr_buf.rindex('=') + 1:])

		left = 0
		right = N
		mid = (right / 2) + (right % 2)

		q = ' '.join(str(x) for x in range(left, mid)) + chr(0x0a)

		try:
			s.send(q)
		except socket.error as e:
			(n, q) = e

			sys.exit(-1)

	if curr_buf.isdigit():
		weight = int(curr_buf)

		if weight < (mid - left) * 10:
			right = mid
		else:
			left = mid

		mid = left + (right - left) / 2 + (right - left) % 2

		q = ' '.join(str(x) for x in range(left, mid)) + chr(0x0a)

		try:
			s.send(q)
		except socket.error as e:
			(n, q) = e

			sys.exit(-1)
