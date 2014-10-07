# emonTx V2 - Wireless energy-monitoring node 

Part of the openenergymonitor.org project

Main emonTx V2 page: 
http://openenergymonitor.org/emon/emontx

Builds on JeeLabs software and compatible with JeeNode hardware

## Libraries Needed
* RFM12: http://github.com/jcw/jeelib
* Mains Voltage and current: https://github.com/openenergymonitor/EmonLib
* Temperature control library: http://download.milesburton.com/Arduino/MaximTemperature/ (version 372 works with Arduino 1.0) and OneWire library: http://www.pjrc.com/teensy/td_libs_OneWire.html
* ElsterMeterReader: https://github.com/openenergymonitor/ElsterMeterReader

## emonTx V2 Code guide
The EmonTx code guide goes through main components required to put a full emontx firmware together. It's recommended that you work through these examples first so that you have a good understanding of how the full firmware's work.
* [01 - Single CT](https://github.com/openenergymonitor/emonTxFirmware/tree/master/emonTxV2/Guide/a_SingleCT/a_SingleCT.ino)
* [02 - Second CT](https://github.com/openenergymonitor/emonTxFirmware/tree/master/emonTxV2/Guide/b_SecondCT/b_SecondCT.ino)
* [03 - AC Voltage](https://github.com/openenergymonitor/emonTxFirmware/tree/master/emonTxV2/Guide/c_ACVoltage/c_ACVoltage.ino)
* [04 - Temperature](https://github.com/openenergymonitor/emonTxFirmware/tree/master/emonTxV2/Guide/d_Temperature/d_Temperature.ino)
* [05 - Pulse Counting](https://github.com/openenergymonitor/emonTxFirmware/tree/master/emonTxV2/Guide/e_PulseCounting/e_PulseCounting.ino)
* [06 - Elster Meter](https://github.com/openenergymonitor/emonTxFirmware/tree/master/emonTxV2/Guide/f_ElsterMeter/f_ElsterMeter.ino)
* [07 - Transmitting Data](https://github.com/openenergymonitor/emonTxFirmware/tree/master/emonTxV2/Guide/g_TransmittingData/g_TransmittingData.ino)
* [08 - Watchdog](https://github.com/openenergymonitor/emonTxFirmware/tree/master/emonTxV2/Guide/h_watchdog/h_watchdog.ino)

## Full emonTx V2 Firmware's

* **emonTx_CT123** - Apparent Power Example - Use this example if only using CT sensors. Monitors AC current using one CT sensor and transmit data via wireless using RFM12B to emonBase. See also: [Apparent power](http://openenergymonitor.org/emon/buildingblocks/ac-power-introduction), [Electricity monitoring](http://openenergymonitor.org/emon/applications/homeenergy)

* **emonTx_CT123_Voltage** - Real Power - Use this example if using an AC-AC adapter with as well as CT sensors. AC-AC plug-in adapter to monitors AC RMS voltage and give real power and current direction readings. See also: [Apparent power](http://openenergymonitor.org/emon/buildingblocks/ac-power-introduction), [Electricity monitoring](http://openenergymonitor.org/emon/applications/homeenergy), [SolarPV](http://openenergymonitor.org/emon/applications/solarpv)

* **emonTx_CT123_Voltage_Temp** - Voltage and current based electricity measurement and 4 × temperature sensors ideal for heatpump monitoring or solar hot water system monitoring. See also: Application note: [Heatpump](http://openenergymonitor.org/emon/applications/heatpump), Building blocks: [DS18B20 temperature sensing](http://openenergymonitor.org/emon/buildingblocks/DS18B20-temperature-sensing), [EmonTx Temperature sensor connection reference](http://openenergymonitor.org/emon/emontx/reference%20)

* **emonTx_Pulse** - Use for counting pulses from pulse output utility meters (flashing LED). An optical sensor can be used to detect pulses. See also: [EmonTx Pulse input connection reference](http://openenergymonitor.org/emon/emontx/reference%20), Building blocks: [Introduction to pulse counting](http://openenergymonitor.org/emon/buildingblocks/introduction-to-pulse-counting), [Gas metering](http://openenergymonitor.org/emon/buildingblocks/gas-meter-monitoring)

* **emonTx_Temperature** - For using multiple DS18B20 temperature sensors on a one-wire bus with emonTx. Uses direct addressing method, run the 'temperature search' sketch to find the addresses of the DS18B20 sensors and insert into main example. http://openenergymonitor.org/emon/buildingblocks/DS18B20-temperature-sensing

* **Elster meter interface only** - Note: RF trasmittion needs to be added to this firmware. Application note: [Elster meter reader](http://openenergymonitor.blogspot.co.uk/2012/08/reading-watt-hour-data-from-elster.html)


**Note:** CT must be clipped round either the Live or Neutral wire, not both! 

### RF Network Settings
At the top of each firmware example you will see the following three lines:

    #define freq RF12_433MHZ
    const int nodeID = 10;
    const int networkGroup = 210;

These set the RF network configuration. The frequency set in the firmware needs to be set to the frequency of the hardware modules. If you have 868MHz hardware modules change the first line to: #define freq RF12_868MHZ. The nodeID needs to be unique for each node on the network and the network group needs to be the same for each node on the network.

Read more about the RFM12B the wireless transceiver module used here: [Sending data between modules with the RFM12B](http://openenergymonitor.org/emon/buildingblocks/rfm12b2)


# License
The emonTx hardware designs (schematics and CAD files) are licensed under a Creative Commons Attribution-ShareAlike 3.0 Unported License.

The emonTx firmware is released under the GNU GPL V3 license

The documentation is subject to GNU Free Documentation License 

The emonTx hardware designs follow the terms of the OSHW (Open-source hardware) Statement of Principles 1.0.

