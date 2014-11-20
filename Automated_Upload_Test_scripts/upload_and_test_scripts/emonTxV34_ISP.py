# Script for RaspberryPi with RFM12Pi 
# Automated upload and test 
# Upload code to emonTx via ISP and check for RF data received 

# By Glyn Hudson 19/11/2014
# Part of the openenergymonitor.org project


import serial, sys, string, commands, time, subprocess
from subprocess import Popen, PIPE, STDOUT

print' '
print 'OpenEnergyMonitor emonTx V3.4 Automated Upload & RF test 19/11/2014'
print ' ' 
print 'Select RF module:'

NodeID = 10

while(1):
	print ' ' 
	nb = raw_input('Enter (b) for RFM12B or (w) for RFM69CW>')
	#print ('Number%s \n' % (nb))
        print(nb)


	if nb=='b': 
		print 'RFM12B 433Mhz'
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
		print 'Attempting RFM12B 433Mhz emonTx firmware upload via ISP....'
		cmd = 'sudo avrdude -V -u -p atmega328p -c avrispmkII -P usb -e -Ulock:w:0x3F:m -Uefuse:w:0x05:m -Uhfuse:w:0xDE:m -Ulfuse:w:0xFF:m -U flash:w:/home/pi/emonTxFirmware/emonTxV3/RFM12B/emonTxV3_4_RFM12B_DiscreteSampling/emonTxV3_4_RFM12B_DiscreteSampling_433.cpp.hex:i  -Ulock:w:0x0F:m'
		subprocess.call(cmd, shell=True)
                #time.sleep(2)
		ser = serial.Serial('/dev/ttyAMA0', 9600, timeout=1)
		linestr = ser.readline()
		print linestr
		#print len(linestr)
		if (len(linestr)>0):
			if (int(linestr[1] + linestr[2])==NodeID): 
				print 'PASS!...RF RECEIVE SUCCESS from emonTx RFM12B'
			else:
				print 'FAIL...RF received but not from the emonTx on node 10 (check DIP switch setting?)'
		else: 
			print 'FAIL...no RF received'
		ser.close()

	if nb=='w': 
		print 'RFM69CW 433Mhz'
		freq = '4b'
	
		print 'setting Rx module to receive on 433Mhz....'
		#time.sleep(2) #delay 2 seconds
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
		print 'Attempting RFM69CW 433Mhz emonTx firmware upload via ISP....'
		cmd = 'sudo avrdude -V -u -p atmega328p -c avrispmkII -P usb -e -Ulock:w:0x3F:m -Uefuse:w:0x05:m -Uhfuse:w:0xDE:m -Ulfuse:w:0xFF:m -U flash:w:/home/pi/emonTxFirmware/emonTxV3/RFM69CW/emonTxV3_RFM69CW_DiscreteSampling/emonTxV3_RFM69CW_DiscreteSampling_433_bootloader.cpp.hex:i  -Ulock:w:0x0F:m'
		subprocess.call(cmd, shell=True)
                time.sleep(1)
		ser = serial.Serial('/dev/ttyAMA0', 9600, timeout=1)
		linestr = ser.readline()
		print linestr
		if (len(linestr)>0):
			if (int(linestr[1] + linestr[2])==NodeID): 
				print 'PASS!...RF RECEIVE SUCCESS from emonTx RFM69CW'
			else:
				print 'FAIL...RF received but not from the emonTx on node 10 (Check DIP switch setting?)'
		else: 
			print 'FAIL...no RF received'
		ser.close()

	#if ((nb!=8) and (nb!=4)):
	#	print 'Invalid selection, please restart script and select b or w'

