while :
	do
		echo "starting avrdude upload emonTH 433Mhz"
		avrdude  -u -c arduino -p ATMEGA328P -P /dev/ttyUSB0 -b 115200 -U flash:w:/home/oem/firmware/emonTH/emonTH_DHT22_DS18B20/emonTH_DHT22_DS18B20.cpp433.hex
		sleep 5
done
