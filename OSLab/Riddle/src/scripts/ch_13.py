#!/bin/python3

import os
from pwn import *

process = process("./riddle 2>&1 | tee text.txt", shell="True")
# os.system("./riddle 2>&1 | tee text.txt")

riddle = os.popen("ls -t /tmp | grep 'riddle' | head -1").read().split("\n")[0]

with open("text.txt", "r") as fd:
    for line in fd:
        if "char" in line:
            char = line[25]

fd = os.open("/tmp/"+riddle, os.O_RDWR)
os.lseek(fd, 0x6f, 0)
os.write(fd, str.encode(char))
process.interactive()
