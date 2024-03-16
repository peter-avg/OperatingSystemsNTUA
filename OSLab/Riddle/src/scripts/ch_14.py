#!/bin/python3

import os
from pwn import *

riddle = process("./riddle", shell="True")
sleep(1)
script = process("truncate --size=30K .hello_there", shell="True")
riddle.interactive()


