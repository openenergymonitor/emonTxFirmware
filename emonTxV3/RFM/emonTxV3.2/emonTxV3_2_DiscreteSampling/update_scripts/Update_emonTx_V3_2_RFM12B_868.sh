# !/bin/bash


echo "starting avrdude upload emonTx V3.2 RFM12B 868Mhz"
avrdude  -u -c arduino -p ATMEGA328P -P /dev/ttyUSB0 -b 115200 -U flash:w:/home/pi/emonTxFirmware/emonTxV3/RFM/emonTxV3.4/emonTxV3_4_DiscreteSampling/compiled/Latest_emonTxV3_2_DiscreteSampling_868.hex
