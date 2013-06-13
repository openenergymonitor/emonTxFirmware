# EmonTx - Wireless energy-monitoring node 

Part of the openenergymonitor.org project

Main EmonTx page: 
http://openenergymonitor.org/emon/emontx

Builds on JeeLabs software and compatible with JeeNode hardware

## Libraries Needed
* RFM12: http://github.com/jcw/jeelib
* Mains Voltage and current: https://github.com/openenergymonitor/EmonLib
* Temperature control library: http://download.milesburton.com/Arduino/MaximTemperature/ (version 372 works with Arduino 1.0) and OneWire library: http://www.pjrc.com/teensy/td_libs_OneWire.html
* ElsterMeterReader: https://github.com/openenergymonitor/ElsterMeterReader

## EmonTx Code guide
The EmonTx code guide goes through main components required to put a full emontx firmware together. It's recommended that you work through these examples first so that you have a good understanding of how the full firmware's work.
* [01 - Single CT](https://github.com/openenergymonitor/emonTxFirmware/blob/master/Guide/a_SingleCT/a_SingleCT.ino)
* [02 - Second CT](https://github.com/openenergymonitor/emonTxFirmware/blob/master/Guide/b_SecondCT/b_SecondCT.ino)
* [03 - AC Voltage](https://github.com/openenergymonitor/emonTxFirmware/blob/master/Guide/c_ACVoltage/c_ACVoltage.ino)
* [04 - Temperature](https://github.com/openenergymonitor/emonTxFirmware/blob/master/Guide/d_Temperature/d_Temperature.ino)
* [05 - Pulse Counting](https://github.com/openenergymonitor/emonTxFirmware/blob/master/Guide/e_PulseCounting/e_PulseCounting.ino)
* [06 - Elster Meter](https://github.com/openenergymonitor/emonTxFirmware/blob/master/Guide/f_ElsterMeter/f_ElsterMeter.ino)
* [07 - Transmitting Data](https://github.com/openenergymonitor/emonTxFirmware/blob/master/Guide/g_TransmittingData/g_TransmittingData.ino)
* [08 - Watchdog](https://github.com/openenergymonitor/emonTxFirmware/blob/master/Guide/h_watchdog/h_watchdog.ino)

## Full EmonTx Firmware's
* **emonTx_CT123** - Apparent Power Example - Use this example if only using CT sensors. Monitors AC current using one CT sensor and transmit data via wireless using RFM12B to emonBase. 

* **emonTx_CT123_Voltage** - Real Power - Use this example if using an AC-AC adapter with as well as CT sensors. AC-AC plug-in adapter to monitors AC RMS voltage and give real power and current direction readings. 

* **emonTx_Pulse** - Use for counting pulses from pulse output utility meter (flashing LED). Optical sensor can be used to detect pulses. 

* **emonTx_Temperature** - For using multiple DS18B20 temperature sensors on a one-wire bus with emonTx. Uses direct addressing method, run the 'temperature search' sketch to find the addresses of the DS18B20 sensors and insert into main example. http://openenergymonitor.org/emon/buildingblocks/DS18B20-temperature-sensing

**Note:** CT must be clipped round either the Live or Neutral wire, not both! 

When the example has been successfully uploaded the green LED should blink quickly once every 10's


# License
The emonTx hardware designs (schematics and CAD files) are licensed under a Creative Commons Attribution-ShareAlike 3.0 Unported License.

The emonTx firmware is released under the GNU GPL V3 license

The documentation is subject to GNU Free Documentation License 

The emonTx hardware designs follow the terms of the OSHW (Open-source hardware) Statement of Principles 1.0.






 
