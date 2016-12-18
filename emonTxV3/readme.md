# emonTx V3 - Low power wireless energy-monitoring node 

Part of the [OpenEnergyMonitor.org](https://openenergymonitor.org) project

- [Getting started guide](https://guide.openenergymonitor.org/setup/emontx)
- [Technical Wiki (emonTx V3.4)](https://wiki.openenergymonitor.org/index.php?title=EmonTx_V3.4)
- [emonTx V3 Resouces](https://guide.openenergymonitor.org/technical/resources/#emontx)

## Current Firmware

**TESTED AND CALIBRATED - all pre-assembled emonTx V3's are shipped with this firmware, available on github as .ino and pre-complied .hex**

**Current firmware key features:**

* Detection of AC-AC adapter sets Apparent Power / Real Power Sampling accordingly
* Detection of battery / USB 5V or AC > DC power method and sets sleep mode accordingly
* Detection of CT connections and samples only from the channels needed
* Detection of remote DS18B20 temperature sensor connection
* Low power battery opperation supported
* DIP switch 1 (closes to RF module) to select node ID. (Switch off node ID =10, switch on node ID = 9)
* DIP switch 2 to select UK/EU or USA AC-AC adapter calibration (Switch off = UK/EU, Switch on = USA)
* [Serial RF nodeID config](https://community.openenergymonitor.org/t/emontx-v3-configure-rf-settings-via-serial-released-fw-v2-6-0/2064)

### RFM Examples:
* **emonTxV3_3phase_Voltage** - Approximate 3-phase sketch. [Now moved to emonTx 3-phase repo](https://github.com/openenergymonitor/emontx-3phase)

* **emonTxV3_CurrentOnly** - Apparent Power Example - Use this example if only using CT sensors. Monitors AC current using one CT sensor and transmit data via wireless using RFM12B to emonBase.

* **emonTxV3_RealPower_Voltage** -Real Power - Use this example if using an AC-AC adapter with as well as CT sensors. AC-AC plug-in adapter to monitors AC RMS voltage and give real power and current direction readings.

* **emonTxV3_continuous** - continuous sampling example contributed by Robin Emley as used in his Mk2 PV Router design

* **emonTxV3_continuous_kwhtotals_noeeprom** - same as emonTxV3_continuous but also keeps track of current Kwh totals

* **emonTxV3_continuous_reciever** Receiver example for Continuous Sampling

* **EmonTxV3HeatpumpMonitor** - Example for monitoring a heatpump with 4 x DS18B20 temperature sensors and 3 x CT current sensors and AC adapter. Power values are in J instead of W

* **emonTxV3_Pulse** - pulse counting example for interfacing with utility meter see [technical wiki documentation](http://wiki.openenergymonitor.org/index.php?title=EmonTx_V3#Utility_Meter_Interface)

* **emonTxV3_RFM12B_DiscreteSampling_with_pulse** - Same as emonTx V3 pulse example but also includes CT power discreatre sampling code


### No RF Examples:
There are example sketches which do not use or require an RF module 

* **emonTxV3_DirectSerial** - Serial output of power readings. Useful for direct connection to RaspberryPi see [technical Wiki](http://wiki.openenergymonitor.org/index.php?title=EmonTx_V3#Direct_connection_emonTx_V3_.3E_Raspberry_Pi_GPIO) 

* **LED_DigitalMeter** - emonTx V3 to emulate solid state digital power meter by pulsing on-board red LED every 0.1Wh (default)

* **MVHRMonitor** - Example for monitoring a mechanical ventilation heat recovery unit (MVHR)

* **HeatpumpMonitorSerial** -  Example for monitoring a heatpump with 4 x DS18B20 temperature sensors and 3 x CT current sensors and AC adapter. With serial output for data for direct serial connection to baseStation

* **voltageFailureDetector** - A test pad for the development of AC voltage sample failure logic


# License

```
emonTx firmware is released under the GNU GPL V3 license
The documentation is subject to GNU Free Documentation License 
The emonTx hardware designs follow the terms of the OSHW (Open-source hardware) Statement of Principles 1.0.
```
