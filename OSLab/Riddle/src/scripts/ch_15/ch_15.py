#!/bin/python3

import os
from pwn import *

process = process("sudo echo 30000 > /proc/sys/kernel/ns_last_pid", shell="True")
riddle = process("./src/scripts/ch_15/a.out", shell="True")
