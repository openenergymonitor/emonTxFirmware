while :
	do
		echo "starting avrdude upload emonTxV3.4 RFM12B 868Mhz"
		avrdude  -u -c arduino -p ATMEGA328P -P /dev/ttyUSB0 -b 115200 -U flash:w:/home/oem/firmware/emonTxFirmware/emonTxV3/RFM/emonTxV3.4/emonTxV3_4_DiscreteSampling/V1_6_emonTxV3_RFM12B_DiscreteSampling.cpp_868.hex

		sleep 3
done
