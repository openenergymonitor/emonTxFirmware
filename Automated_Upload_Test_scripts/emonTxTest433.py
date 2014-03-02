
import serial, sys, string, commands, time, subprocess
from subprocess import Popen, PIPE, STDOUT

freq = '4b'
group = '210g'

print 'emonTx V3 433Mhz'

print 'setting Rx module to receive on 433Mhz....'
time.sleep(2) #delay 2 seconds
ser = serial.Serial('/dev/ttyAMA0', 9600)
ser.write(freq)
time.sleep(0.5)
linestr = ser.readline()
print linestr
if freq in linestr: 		#check RFM12Pi responce to check it's set frequenct OK	
	print 'success..Rx Module set to receive on'
	print freq  
else:
	print 'error..Rx Module is not responding'
linestr=''
time.sleep(1)
ser.write(group)
time.sleep(1)
linestr = ser.readline()
print linestr
if group in linestr: 		#check RFM12Pi responce to check it's set frequenct OK	
	print 'success..Rx Module set to receive on group'
	print group  
else:
	print 'error..Rx Module is not responding'

ser.close()




	

#print commands.getstatusoutput('avrdude -v  -c arduino -p ATMEGA328P -P /dev/ttyUSB0 -b 115200 -U flash:w:emonTxFirmware/emonTxV3/RFM12B/emonTxV3_RFM12B_DiscreteSampling/emonTxV3_RFM12B_DiscreteSampling.cpp433.hex')

cmd = 'avrdude -u  -c arduino -p ATMEGA328P -P /dev/ttyUSB0 -b 115200 -U flash:w:emonTxFirmware/emonTxV3/RFM12B/emonTxV3_RFM12B_DiscreteSampling/emonTxV3_RFM12B_DiscreteSampling.cpp433.hex'
subprocess.call(cmd, shell=True)

#p = Popen('svnadmin verify /var/svn/repos/config', stdout = PIPE, 
 #       stderr = STDOUT, shell = True)
#time.sleep(2)	# 2 second delay
#print "starting avrdude upload emonTx V3 433Mhz"

ser = serial.Serial('/dev/ttyAMA0', 9600)
linestr = ser.readline()
print linestr
ser.close()
