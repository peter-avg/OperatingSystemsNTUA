#!/bin/python3
import os
import signal

os.popen("~/Downloads/riddle-20231009-0/riddle")
pid = os.system("ps aux | grep riddle | head -1 | awk '{ print $2 }'")
os.kill(int(pid), signal.SIGCONT)
