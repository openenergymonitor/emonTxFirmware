# emonTx V3 3-phase Firmware

The emonTx have been designed for single-phase AC monitoring. 

The emonTx can monitor 'approximate' three-phase (assuming balanced phases) using [modified firmware](https://github.com/openenergymonitor/emonTxFirmware/tree/master/emonTxV3/RFM/emonTxV3.4/emonTxV3_4_3Phase_Voltage) and 3x CT sensors + 1 x AC-AC adapter. [Further reading](https://openenergymonitor.org/emon/buildingblocks/3-phase-power)

## Upload

Upload firmware to emonTx V3.4 using Raspberry Pi or Linux PC

Plug USB to UART adatper into Raspberry Pi and emonTx: http://shop.openenergymonitor.com/programmer-usb-to-serial-uart/

![programmer_emontx](http://openenergymonitor.org/emon/sites/default/files/emontxv3_USBtoUART.jpg)

Update script assumes USB to UART programmer is linked to /dev/ttyUSB0, you can check this by running dmesg after plugging in programmer. Adjust script if different tty is used. 

	$ dmesg

Clone this repo:

	$ git clone https://github.com/openenergymonitor/emonTxFirmware.git

or if you have cloned before pull in latest updates 

	$ git pull

	$ cd /home/pi/emonTxFirmware/emonTxV3/RFM/emonTxV3.4/emonTxV3_4_3Phase_Voltage

Ensure you have set the correct RF module in the sketch for your emonTx (RF module and frequeny), see here for help identify RF module: http://openenergymonitor.org/emon/buildingblocks/which-radio-module

Run update script e.g:

	$ avrdude  -u -c arduino -p ATMEGA328P -P /dev/ttyUSB0 -b 115200 -U flash:w:/emonTxV3_4_3Phase_Voltage.ino.hex


Check update has worked by viewing serial output of emonTx at statup:
Open up serial window, with minicom. Install if required

	$ sudo apt-get install minicom -y

	$ minicom -D /dev/ttyUSB0 -b 6900
