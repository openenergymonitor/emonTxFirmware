# emonTx V3 - Low power wireless energy-monitoring node 

Part of the openenergymonitor.org project

Main emonTx V3 documentation: 
[http://openenergymonitor.org/emon/modules/emonTxV3]


## RFM12B Firmware

### *emonTxV3_RFM12B_Discrete Sampling* - MAIN EMONTX V3 FIRMWARE DECEMBER 2013
**TESTED AND CALIBRATED - all pre-assembled emonTx V3's are shpped with this firmware**

* Detection of AC-AC adapter sets Apparent Power / Real Power Sampling accordingly
* Detection of battery / USB 5V or AC > DC power method and sets sleep mode accordingly
* Detection of CT connections and samples only from the channels needed
* Detection of remote DS18B20 temperature sensor connection


### RFM12B Examples:
**Caution: These have not been tested or calibrated extensively** 

* *emonTxV3_4chan_continuous*
* *emonTxV3_CurrentOnly*
* *emonTxV3_RealPower_Voltage*
* *emonTxV3_Pulse* - pulse counting example for interfacing with utility meter see [technical wiki documentation](http://wiki.openenergymonitor.org/index.php?title=EmonTx_V3#Utility_Meter_Interface)

## SRF Firmware
**Caution: These have not been tested or calibrated extensively** 

* SRF___Low_Power___Current_only_Apparent_Power

The emonTx firmware is released under the GNU GPL V3 license

The documentation is subject to GNU Free Documentation License 

The emonTx hardware designs follow the terms of the OSHW (Open-source hardware) Statement of Principles 1.0.
