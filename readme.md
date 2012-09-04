# EmonTx - Wireless energy-monitoring node 

Main EmonTx page: 
http://openenergymonitor.org/emon/emontx

Builds on JeeLabs software and compatible with JeeNode hardware

# Libraries Needed
* https://github.com/openenergymonitor/EmonLib
* http://github.com/jcw/jeelib

Temperature monitoring needs the following: 
Temperature control library: http://download.milesburton.com/Arduino/MaximTemperature/ (version 372 works with Arduino 1.0) and OneWire library: http://www.pjrc.com/teensy/td_libs_OneWire.html

# Full EmonTx Firmware's
* **emonTx_CT123** - Apparent Power Example - Use this example if only using CT sensors. Monitors AC current using one CT sensor and transmit data via wireless using RFM12B to emonBase. 

* **emonTx_CT123_Voltage** - Real Power - Use this example if using an AC-AC adapter with as well as CT sensors. AC-AC plug-in adapter to monitors AC RMS voltage and give real power and current direction readings. 

* **emonTx_Pulse** - Use for counting pulses from pulse output utility meter (flashing LED). Optical sensor can be used to detect pulses. 

* **emonTx_Temperature** - For using multiple DS18B20 temperature sensors on a one-wire bus with emonTx. Uses direct addressing method, run the 'temperature search' sketch to find the addresses of the DS18B20 sensors and insert into main example. http://openenergymonitor.org/emon/buildingblocks/DS18B20-temperature-sensing

**Note:** CT must be clipped round either the Live or Neutral wire, not both! 

When the example has been successfully uploaded the green LED should blink quickly once very 10's






 
