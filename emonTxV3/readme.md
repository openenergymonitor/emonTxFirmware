# emonTx V3 - Low power wireless energy-monitoring node 

Part of the http://openenergymonitor.org project

emoTx V3 main documentaion page including quick start guide: 
http://openenergymonitor.org/emon/modules/emonTxV3

Technical emonTx V3 wiki ocumentation: http://wiki.openenergymonitor.org/index.php?title=EmonTx_V3

Open-hardware schematic and board files: http://solderpad.com/openenergymon/emontxv3/

## Libraries Needed
* RFu_JeeLib: https://github.com/openenergymonitor/RFu_jeelib
* Mains Voltage and current: https://github.com/openenergymonitor/EmonLib
* Temperature control library: http://download.milesburton.com/Arduino/MaximTemperature/ (version 372 works with Arduino 1.0) and OneWire library: http://www.pjrc.com/teensy/td_libs_OneWire.html
* ElsterMeterReader: https://github.com/openenergymonitor/ElsterMeterReader

**Note:** CT must be clipped round either the Live or Neutral wire, not both! 

## RFM12B Firmware

### MAIN EMONTX V3 FIRMWARE *emonTxV3_RFM12B_Discrete Sampling* - 
**TESTED AND CALIBRATED - all pre-assembled emonTx V3's are shipped with this firmware**

* Detection of AC-AC adapter sets Apparent Power / Real Power Sampling accordingly
* Detection of battery / USB 5V or AC > DC power method and sets sleep mode accordingly
* Detection of CT connections and samples only from the channels needed
* Detection of remote DS18B20 temperature sensor connection


### RFM12B Examples:
* **emonTxV3_3phase_Voltage** - 3-phase sketch 

* **emonTxV3_CurrentOnly** - Apparent Power Example - Use this example if only using CT sensors. Monitors AC current using one CT sensor and transmit data via wireless using RFM12B to emonBase.

* **emonTxV3_RealPower_Voltage** -Real Power - Use this example if using an AC-AC adapter with as well as CT sensors. AC-AC plug-in adapter to monitors AC RMS voltage and give real power and current direction readings.

* **emonTxV3_continuous** - PLL continuous sampling example contributed by MartinR, as used in PV controller MK2

* **emonTxV3_Pulse** - pulse counting example for interfacing with utility meter see [technical wiki documentation](http://wiki.openenergymonitor.org/index.php?title=EmonTx_V3#Utility_Meter_Interface)

## SRF Firmware
**Caution: These have not been tested or calibrated extensively** 

* SRF___Low_Power___Current_only_Apparent_Power

# License
emonTx firmware is released under the GNU GPL V3 license http://openenergymonitor.org/emon/node/4

The documentation is subject to GNU Free Documentation License 

The emonTx hardware designs follow the terms of the OSHW (Open-source hardware) Statement of Principles 1.0.
