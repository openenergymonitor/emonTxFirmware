# Script for RaspberryPi with RFM12Pi 
# Automated upload an test 
# Upload code to emonTx and check for RF data received 

# By Glyn Hudson 3/03/13
# Part of the openenergymonitor.org project


import serial, sys, string, commands, time, subprocess
from subprocess import Popen, PIPE, STDOUT

print' '
print 'OpenEnergyMonitor emonTx V3 Automated Upload & RF test'
print ' ' 
print 'Select desired frequency upload'



while(1):
	print ' ' 
	nb = input('Enter (4) for 433Mhz or (8) for 868Mhz>')
	#print ('Number%s \n' % (nb))


	if nb==4: 
		print 'emonTx V3 433Mhz'
		freq = '4b'
		NodeID = 10
		print 'setting Rx module to receive on 433Mhz....'
		#time.sleep(1) #delay 2 seconds
		ser = serial.Serial('/dev/ttyAMA0', 9600, timeout=1)
		ser.write(freq)
		#time.sleep(0.5)
		linestr = ser.readline()
		#print linestr
		if freq in linestr: 		#check RFM12Pi responce to check it's set frequenct OK	
			print 'success..Rx Module set to receive on 433Mhz'
			#print freq  
		else:
			print 'error..Rx Module is not responding'
		ser.close()
		print 'Attempting 433Mhz emonTx V3 firmware upload....'
		cmd = 'avrdude -u  -c arduino -p ATMEGA328P -P /dev/ttyUSB0 -b 115200 -U flash:w:/home/pi/emonTxFirmware/emonTxV3/RFM12B/emonTxV3_RFM12B_DiscreteSampling/emonTxV3_RFM12B_DiscreteSampling.cpp433.hex'
		subprocess.call(cmd, shell=True)

		ser = serial.Serial('/dev/ttyAMA0', 9600, timeout=1)
		linestr = ser.readline()
		print linestr
		#print len(linestr)
		if (len(linestr)>0):
			if (int(linestr[1] + linestr[2])==NodeID): 
				print 'PASS!...RF RECEIVE SUCCESS from emonTx'
			else:
				print 'FAIL...RF received but not from the emonTx'
		else: 
			print 'FAIL...no RF received from emonTx'
		ser.close()

	if nb==8: 
		print 'emonTx V3 868Mhz'
		freq = '8b'
		NodeID = 10
		print 'setting Rx module to receive on 833Mhz....'
		#time.sleep(2) #delay 2 seconds
		ser = serial.Serial('/dev/ttyAMA0', 9600, timeout=1)
		ser.write(freq)
		#time.sleep(0.5)
		linestr = ser.readline()
		print linestr
		if freq in linestr: 		#check RFM12Pi responce to check it's set frequenct OK	
			print 'success..Rx Module set to receive on 868Mhz'
			print freq  
		else:
			print 'error..Rx Module is not responding'

		ser.close()
		print 'Attempting 868Mhz emonTx V3 firmware upload....'
		cmd = 'avrdude -u  -c arduino -p ATMEGA328P -P /dev/ttyUSB0 -b 115200 -U flash:w:/home/pi/emonTxFirmware/emonTxV3/RFM12B/emonTxV3_RFM12B_DiscreteSampling/emonTxV3_RFM12B_DiscreteSampling.cpp868.hex'
		subprocess.call(cmd, shell=True)

		ser = serial.Serial('/dev/ttyAMA0', 9600, timeout=1)
		linestr = ser.readline()
		print linestr
		if (len(linestr)>0):
			if (int(linestr[1] + linestr[2])==NodeID): 
				print 'PASS!...RF RECEIVE SUCCESS from emonTx'
			else:
				print 'FAIL...RF received but not from the emonTx'
		else: 
			print 'FAIL...no RF received from emonTx'
		ser.close()

	if ((nb!=8) and (nb!=4)):
		print 'Invalid selection, please restart script and select 4 or 8'
