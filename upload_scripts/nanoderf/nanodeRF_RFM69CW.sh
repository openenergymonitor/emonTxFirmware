while :
	do
		echo "starting avrdude upload nanodeRF Test"
		avrdude -u -c arduino -p ATMEGA328P -P /dev/ttyUSB0 -b 115200 -U flash:w:/home/oem/firmware/NanodeRF/NanodeRF_Test/NanodeRF_Test_RFM69CW.cpp.hex
		sleep 5
done
