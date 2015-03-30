while :
	do
		echo "starting avrdude upload emonTx V3 no RF"
		avrdude  -u -c arduino -p ATMEGA328P -P /dev/ttyUSB0 -b 115200 -U flash:w:/home/oem/firmware/emonTxFirmware/emonTxV3/noRF/emonTxV3_DirectSerial/emonTxV3_DirectSerial.cpp.hex

		sleep 3
done
