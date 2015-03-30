# Script for RaspberryPi with RFM12Pi 
# Automated upload an test 
# Upload code to emonTx and check for RF data received 

# By Glyn Hudson 3/03/13
# Part of the openenergymonitor.org project


import serial, sys, string, commands, time, subprocess
from subprocess import Popen, PIPE, STDOUT

print' '
print 'OpenEnergyMonitor emonTH Automated Upload & RF test'
print ' ' 
print 'Select desired frequency upload'

NodeID = 19

while(1):
	print ' ' 
	nb = input('Enter (4) for 433Mhz or (8) for 868Mhz>')
	#print ('Number%s \n' % (nb))


	if nb==4: 
		print 'emonTH 433Mhz'
		freq = '4b'
		
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
		print 'Attempting 433Mhz emonTH firmware upload....'
		cmd = 'avrdude -u  -c arduino -p ATMEGA328P -P /dev/ttyUSB0 -b 115200 -U flash:w:/home/pi/emonTH/emonTH_DHT22_DS18B20/emonTH_DHT22_DS18B20.cpp433.hex'
		subprocess.call(cmd, shell=True)

		ser = serial.Serial('/dev/ttyAMA0', 9600, timeout=1)
		linestr = ser.readline()
		#print linestr
		#print len(linestr)
		if (len(linestr)>0):
			if (int(linestr[1] + linestr[2])==NodeID): 
				print 'PASS!...RF RECEIVE SUCCESS from emonTH'
			else:
				print 'FAIL...RF received but not from the emonTH'
		else: 
			print 'FAIL...no RF received from emonTH'
		ser.close()

	if nb==8: 
		print 'emonTH 868Mhz'
		freq = '8b'
	
		print 'setting Rx module to receive on 833Mhz....'
		#time.sleep(2) #delay 2 seconds
		ser = serial.Serial('/dev/ttyAMA0', 9600, timeout=1)
		ser.write(freq)
		#time.sleep(0.5)
		linestr = ser.readline()
		#print linestr
		if freq in linestr: 		#check RFM12Pi responce to check it's set frequenct OK	
			print 'success..Rx Module set to receive on 868Mhz'
			#print freq  
		else:
			print 'error..Rx Module is not responding'

		ser.close()
		print 'Attempting 868Mhz emonTH firmware upload....'
		cmd = 'avrdude -u  -c arduino -p ATMEGA328P -P /dev/ttyUSB0 -b 115200 -U flash:w:/home/pi/emonTH/emonTH_DHT22_DS18B20/emonTH_DHT22_DS18B20.cpp868.hex'
		subprocess.call(cmd, shell=True)

		ser = serial.Serial('/dev/ttyAMA0', 9600, timeout=1)
		linestr = ser.readline()
		print linestr
		if (len(linestr)>0):
			if (int(linestr[1] + linestr[2])==NodeID): 
				print 'PASS!...RF RECEIVE SUCCESS from emonTH'
			else:
				print 'FAIL...RF received but not from the emonTH'
		else: 
			print 'FAIL...no RF received from emonTH'
		ser.close()

	if ((nb!=8) and (nb!=4)):
		print 'Invalid selection, please restart script and select 4 or 8'

