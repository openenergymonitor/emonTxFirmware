/*
 EmonTx CT123 example
 
 An example sketch for the emontx module for
 CT only electricity monitoring.
 
 Part of the openenergymonitor.org project
 Licence: GNU GPL V3
 
 Authors: Glyn Hudson, Trystan Lea
 Builds upon JeeLabs RF12 library and Arduino
 
 emonTx documentation: 	http://openenergymonitor.org/emon/modules/emontx/
 emonTx firmware code explination: http://openenergymonitor.org/emon/modules/emontx/firmware
 emonTx calibration instructions: http://openenergymonitor.org/emon/modules/emontx/firmware/calibration

 THIS SKETCH REQUIRES:

 Libraries in the standard arduino libraries folder:
	- JeeLib		https://github.com/jcw/jeelib
	- EmonLib		https://github.com/openenergymonitor/EmonLib.git

 Other files in project directory (should appear in the arduino tabs above)
	- emontx_lib.ino
 
*/
#define FILTERSETTLETIME 5000                                           //  Time (ms) to allow the filters to settle before sending data

const int CT1 = 1; 
const int CT2 = 1;                                                      // Set to 0 to disable CT channel 2
const int CT3 = 1;                                                      // Set to 0 to disable CT channel 3

#define freq RF12_868MHZ                                                // Frequency of RF12B module can be RF12_433MHZ, RF12_868MHZ or RF12_915MHZ. You should use the one matching the module you have.
const int nodeID = 10;                                                  // emonTx RFM12B node ID
const int networkGroup = 210;                                           // emonTx RFM12B wireless network group - needs to be same as emonBase and emonGLCD

const int UNO = 1;                                                      // Set to 0 if your not using the UNO bootloader (i.e using Duemilanove) - All Atmega's shipped from OpenEnergyMonitor come with Arduino Uno bootloader
#include <avr/wdt.h>                                                     

#include <JeeLib.h>                                                     // Download JeeLib: http://github.com/jcw/jeelib
ISR(WDT_vect) { Sleepy::watchdogEvent(); }                              // Attached JeeLib sleep function to Atmega328 watchdog -enables MCU to be put into sleep mode inbetween readings to reduce power consumption 

#include "EmonLib.h"
EnergyMonitor ct1,ct2,ct3;                                              // Create  instances for each CT channel

typedef struct { int power1, power2, power3, battery; } PayloadTX;      // create structure - a neat way of packaging data for RF comms
PayloadTX emontx;                                                       

const int LEDpin = 9;                                                   // On-board emonTx LED 

boolean settled = false;

void setup() 
{
  Serial.begin(9600);
  Serial.println("emonTX CT123 example"); 
  Serial.println("OpenEnergyMonitor.org");
  Serial.print("Node: "); 
  Serial.print(nodeID); 
  Serial.print(" Freq: "); 
  if (freq == RF12_433MHZ) Serial.print("433Mhz");
  if (freq == RF12_868MHZ) Serial.print("868Mhz");
  if (freq == RF12_915MHZ) Serial.print("915Mhz"); 
 Serial.print(" Network: "); 
  Serial.println(networkGroup);
             
  if (CT1) ct1.currentTX(1, 111.1);                                     // Setup emonTX CT channel (ADC input, calibration)
  if (CT2) ct2.currentTX(2, 111.1);                                     // Calibration factor = CT ratio / burden resistance
  if (CT3) ct3.currentTX(3, 111.1);                                     // Calibration factor = (100A / 0.05A) / 33 Ohms
  
  rf12_initialize(nodeID, freq, networkGroup);                          // initialize RFM12B
  rf12_sleep(RF12_SLEEP);

  pinMode(LEDpin, OUTPUT);                                              // Setup indicator LED
  digitalWrite(LEDpin, HIGH);
  
  if (UNO) wdt_enable(WDTO_8S);                                         // Enable anti crash (restart) watchdog if UNO bootloader is selected. Watchdog does not work with duemilanove bootloader                                                             // Restarts emonTx if sketch hangs for more than 8s
}

void loop() 
{ 
  if (CT1) {
    emontx.power1 = ct1.calcIrms(1480) * 240.0;                         //ct.calcIrms(number of wavelengths sample)*AC RMS voltage
    Serial.print(emontx.power1);                                         
  }
  
  if (CT2) {
    emontx.power2 = ct2.calcIrms(1480) * 240.0;
    Serial.print(" "); Serial.print(emontx.power2);
  } 

  if (CT3) {
    emontx.power3 = ct3.calcIrms(1480) * 240.0;
    Serial.print(" "); Serial.print(emontx.power3);
  } 
  
  emontx.battery = ct1.readVcc();                                      //read emonTx battey (supply voltage)
  
  Serial.print(" "); Serial.print(emontx.battery);
  Serial.println(); delay(100);

  // because millis() returns to zero after 50 days ! 
  if (!settled && millis() > FILTERSETTLETIME) settled = true;

  if (settled)                                                            // send data only after filters have settled
  { 
    send_rf_data();                                                       // *SEND RF DATA* - see emontx_lib
    digitalWrite(LEDpin, HIGH); delay(2); digitalWrite(LEDpin, LOW);      // flash LED
    emontx_sleep(1);                                                      // sleep or delay in seconds - see emontx_lib
  }
}
