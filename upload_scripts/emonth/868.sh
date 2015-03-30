while :
	do
		echo "starting avrdude upload emonTH 868Mhz"
		avrdude  -u -c arduino -p ATMEGA328P -P /dev/ttyUSB0 -b 115200 -U flash:w:/home/oem/firmware/emonTH/emonTH_DHT22_DS18B20/emonTH_DHT22_DS18B20.cpp868.hex
		sleep 1
done
