while :
	do
		echo "starting avrdude upload emonTx V3 433Mhz"
		avrdude  -u -c arduino -p ATMEGA328P -P /dev/ttyUSB0 -b 115200 -U flash:w:/home/oem/firmware/emonTxFirmware/emonTxV3/RFM12B/emonTxV3_RFM12B_DiscreteSampling/emonTxV3_RFM12B_DiscreteSampling.cpp433.hex

		sleep 3
done
