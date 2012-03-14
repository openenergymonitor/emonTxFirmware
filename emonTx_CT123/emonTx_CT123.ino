/*
 EmonTx CT123 example
 
 An example sketch for the emontx module for
 CT only electricity monitoring.
 
 Part of the openenergymonitor.org project
 Licence: GNU GPL V3
 
 Authors: Glyn Hudson, Trystan Lea
 Builds upon JeeLabs RF12 library and Arduino
 
*/

const int CT2 = 1;
const int CT3 = 1;

#define freq RF12_433MHZ                                                // Frequency of RF12B module can be RF12_433MHZ, RF12_868MHZ or RF12_915MHZ. You should use the one matching the module you have.
const int nodeID = 10;                                                  // emonTx node ID
const int networkGroup = 210;                                           // emonTx RFM12B wireless network group - needs to be same as emonBase and emonGLCD

const int UNO = 1;                                                      // Set to 0 if your not using the UNO bootloader (i.e using Duemilanove) - All Atmega's shipped from OpenEnergyMonitor come witj Arduino Uno bootloader
#include <avr/wdt.h>                                                    // 

#include <JeeLib.h>                                                     // Download JeeLib: http://github.com/jcw/jeelib
ISR(WDT_vect) { Sleepy::watchdogEvent(); }                              // Attached JeeLib sleep function to Atmega328 watchdog -enables MCU to be put into sleep mode inbetween readings to reduce power consumption 

#include "EmonLib.h"
EnergyMonitor ct1,ct2,ct3;                                              // Create  instances for each CT channel

typedef struct { int power1, power2, power3, battery; } PayloadTX;      // create structure - a neat way of packaging data for RF comms
PayloadTX emontx;                                                       

const int LEDpin = 9;                                                   // On-board emonTx LED 

void setup() 
{
  Serial.begin(9600);
  Serial.println("emonTX CT123 example"); 
  Serial.println("OpenEnergyMonitor.org");
             
  ct1.currentTX(1, 115.6);                                              // Setup emonTX CT channel (channel, calibration)
  if (CT2) ct2.currentTX(2, 115.6);
  if (CT3) ct3.currentTX(3, 115.6);
  
  rf12_initialize(nodeID, freq, networkGroup);                          // initialize RFM12B
  rf12_sleep(RF12_SLEEP);

  pinMode(LEDpin, OUTPUT);                                              // Setup indicator LED
  digitalWrite(LEDpin, HIGH);
  
  if (UNO) wdt_enable(WDTO_8S);                                         // Enable anti crash (restart) watchdog if UNO bootloader is selected. Watchdog does not work with duemilanove bootloader                                                             // Restarts emonTx if sketch hangs for more than 8s
}

void loop() 
{ 
  emontx.power1 = ct1.calcIrms(1480) * 240.0;                         // Calculate CT 1 power
  Serial.print(emontx.power1);                                        // Output to serial  
    
  if (CT2) {
    emontx.power2 = ct2.calcIrms(1480) * 240.0;
    Serial.print(" "); Serial.print(emontx.power2);
  } 

  if (CT3) {
    emontx.power3 = ct3.calcIrms(1480) * 240.0;
    Serial.print(" "); Serial.print(emontx.power3);
  } 
  
  emontx.battery = ct1.readVcc();
  Serial.println(); delay(100);
 
  send_rf_data();                                                       // *SEND RF DATA* - see emontx_lib
  emontx_sleep(5);                                                     // sleep or delay in seconds - see emontx_lib
  digitalWrite(LEDpin, HIGH); delay(2); digitalWrite(LEDpin, LOW);      // flash LED
}
