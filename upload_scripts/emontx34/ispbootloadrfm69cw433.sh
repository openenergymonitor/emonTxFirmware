
		echo "starting ISP bootlaoder upload emonTx V3.4 433Mhz RFM69CW"
		sudo avrdude -V -u -p atmega328p -c avrispmkII -P usb -e -Ulock:w:0x3F:m -Uefuse:w:0x05:m -Uhfuse:w:0xDE:m -Ulfuse:w:0xFF:m -U flash:w:/home/oem/firmware/emonTxFirmware/emonTxV3/RFM/emonTxV3.4/emonTxV3_4_DiscreteSampling/V1_6_emonTxV3_RFM69CW_DiscreteSampling_433_bootloader.cpp.hex:i  -Ulock:w:0x0F:m


