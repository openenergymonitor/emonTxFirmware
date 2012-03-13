                          _____    
                         |_   _|   
  ___ _ __ ___   ___  _ __ | |_  __
 / _ \ '_ ` _ \ / _ \| '_ \| \ \/ /
|  __/ | | | | | (_) | | | | |>  < 
 \___|_| |_| |_|\___/|_| |_\_/_/\_\
                                                  
Wireless energy-monitoring node 

http://openenergymonitor.org/emon/emontx
***********************************

Builds on JeeLabs software and compatiable with JeeNode hardware 
------------------------------------------------------------------------
Download the JeeLib library here (insert into Arduino librarys folder):
http://github.com/jcw/jeelib

Temperature monitoring needs the following: 
Temperature controll library: http://download.milesburton.com/Arduino/MaximTemperature/ (version 372 works with Arduino 1.0) 
and OneWire library: http://www.pjrc.com/teensy/td_libs_OneWire.html

-------------------------------------------------------------------------

emonTx_CT123 - Apparent Power Example - Use this example if only using CT sensors. Monitors AC current using one CT sensor and transmitt data via wireless using RFM12B to emonBase. 

emonTx_CT123_Voltage - Real Power - Use this example if using an AC-AC adapter with as well as CT sensors. AC-AC plug-in adapter to monitors AC RMS voltage and give real power and current direction readings. 

Both examples are setup to use one CT (CT1 port) by default. Uncomment commented out lines to enable reading from multiple CT's. 

Note: CT must be clipped round either the Live or Neutral wire, not both! 






 
