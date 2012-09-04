/*
  Part 4 – Measuring temperature

  By using direct addressing its possible to make sure that as you add temperature sensors the 
  temperature sensor to variable mapping will not change. To find the addresses of your temperature 
  sensors use the: **temperature_search sketch** : 
  
  emonTxFirmware > emonTx_temperature_examples > temperature_search

  -----------------------------------------
  Part of the openenergymonitor.org project
  Licence: GNU GPL V3  
*/ 

#include <OneWire.h>
#include <DallasTemperature.h>
                                                                                           
OneWire oneWire(4);                    // Setup one-wire on digital input pin 4
DallasTemperature sensors(&oneWire);   // Pass the oneWire reference to Dallas Temperature.

DeviceAddress address_T1 = { 0x28, 0x22, 0x70, 0xEE, 0x02, 0x00, 0x00, 0xB8 };

void setup() {
  Serial.begin(9600);
  sensors.begin();
}

void loop()
{ 
  sensors.requestTemperatures();
  
  double temperature = sensors.getTempC(address_T1);  // Get the temperature of the sensor
  Serial.println(temperature);                        // Print temperature
}
