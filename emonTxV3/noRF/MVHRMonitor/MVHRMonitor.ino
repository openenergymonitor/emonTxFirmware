// EmonTx example for monitoring a mechanical ventilation heat recovery unit (MVHR)

// Connected sensors:
// 2x DS18B20 temperature sensors
// 2x DHT22 temperature and humidity sensors 
// 1x AC Real Power measurement

// Authors: Trystan Lea
// 14th Feb 2014

#include <avr/wdt.h>                                                  // Watchdog
#include <RFu_JeeLib.h>                                               // Special modified version of the JeeJib library to work with the RFu328 https://github.com/openenergymonitor/RFu_jeelib        
ISR(WDT_vect) { Sleepy::watchdogEvent(); }                            // Attached JeeLib sleep function to Atmega328 watchdog -enables MCU to be put into sleep mode inbetween readings to reduce power consumption 

// Default RF settings
#define freq RF12_433MHZ                                              // Frequency of RF12B module can be RF12_433MHZ, RF12_868MHZ or RF12_915MHZ. You should use the one matching the module you have.
const int nodeID = 10;                                                // emonTx RFM12B node ID
const int networkGroup = 210;  

// POWER MONITORING
#include "EmonLib.h"
EnergyMonitor ct1;
const float Ical1=                16.6;                                 // (2000 turns / 120 Ohm burden) = 16.6
const float Vcal=                 276.9;                                // (230V x 13) / (9V x 1.2) = 276.9
const float phase_shift=          1.7;
const int no_of_half_wavelengths= 20;
const int timeout=                2000;
#define FILTERSETTLETIME          10000 

// TEMPERATURE MONITORING
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT22.h>

#define ONE_WIRE_BUS 5
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// TEMPERATURE AND HUMIDITY MONITORING
#define DHT22_PIN_A 19
#define DHT22_PIN_B 2
DHT22 DHT22A (DHT22_PIN_A);
DHT22 DHT22B (DHT22_PIN_B);

// DATA STRUCTURE FOR MONITORED DATA
typedef struct {
  int temp1;
  int temp2;
  int temp3;
  int temp4;
  int humidity1;
  int humidity2;
  int power;
} PayloadTX;

PayloadTX emontx;

const byte LEDpin = 6; // emonTx V3 LED

unsigned long lastreading;

// SETUP 
void setup(void)
{
  rf12_initialize(nodeID, freq, networkGroup);
  
  pinMode(LEDpin, OUTPUT); 
  digitalWrite(LEDpin,HIGH);
  
  Serial.begin(9600);
  Serial.println("EmonTX setup for MVHR Monitoring, 4x temperature sensors, 2x humidity, 1x fan power");
 
  Serial.println("RFM12B Initiated: ");
  Serial.print("Node: "); Serial.print(nodeID); 
  Serial.print(" Freq: "); 
  if (freq == RF12_433MHZ) Serial.print("433Mhz");
  if (freq == RF12_868MHZ) Serial.print("868Mhz");
  if (freq == RF12_915MHZ) Serial.print("915Mhz"); 
  Serial.print(" Network: "); Serial.println(networkGroup);
  delay(100);  

  sensors.begin();
  
  ct1.current(4, Ical1);
  ct1.voltage(0, Vcal, phase_shift);
  
  lastreading = millis();
  
  ct1.calcVI(no_of_half_wavelengths,timeout);
  
  wdt_enable(WDTO_8S);
}

// MAIN LOOP
void loop(void)
{ 
  
  if ((millis()-lastreading)>10000)
  {
    lastreading = millis();

    ct1.calcVI(no_of_half_wavelengths,timeout); 
    emontx.power = ct1.realPower;
    
    DHT22_ERROR_t errorCode;
    
    errorCode = DHT22A.readData();
    if (errorCode==DHT_ERROR_NONE) {
      emontx.temp1 = DHT22A.getTemperatureC() * 100;
      emontx.humidity1 = DHT22A.getHumidity() * 100;
    }
    
    errorCode = DHT22B.readData();
    if (errorCode==DHT_ERROR_NONE) {
      emontx.temp2 = DHT22B.getTemperatureC() * 100;
      emontx.humidity2 = DHT22B.getHumidity() * 100;
    }
    
    sensors.requestTemperatures();
    emontx.temp3 = sensors.getTempCByIndex(0) * 100;
    emontx.temp4 = sensors.getTempCByIndex(1) * 100;
  
    rf12_sendNow(0, &emontx, sizeof emontx);
    rf12_sendWait(2);
    
    Serial.print(emontx.temp1);
    Serial.print(' ');
    Serial.print(emontx.temp2);
    Serial.print(' ');
    Serial.print(emontx.temp3);
    Serial.print(' ');
    Serial.print(emontx.temp4);
    Serial.print(' ');
    Serial.print(emontx.humidity1);
    Serial.print(' ');
    Serial.print(emontx.humidity2);  
    Serial.print(' ');
    Serial.print(emontx.power);  
    Serial.println();

  }
  
  wdt_reset();
  delay(1000);

}
