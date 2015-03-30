
import serial, sys, string, commands, time, subprocess
from subprocess import Popen, PIPE, STDOUT

print' '
print 'Test RFM12Pi Comms'
print ' ' 

while(1):
  ser = serial.Serial('/dev/ttyAMA0', 9600, timeout=1)
  linestr = ser.readline()
  print linestr
  print len(linestr)
