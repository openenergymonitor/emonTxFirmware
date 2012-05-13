/*
 EmonTx CT123 + Voltage example
 
 An example sketch for the emontx module for
 CT only electricity monitoring.
 
 Part of the openenergymonitor.org project
 Licence: GNU GPL V3
 
 Authors: Glyn Hudson, Trystan Lea
 Builds upon JeeLabs RF12 library and Arduino
 
 emonTx documentation: http://openenergymonitor.org/emon/modules/emontx/
 emonTx firmware code explination: http://openenergymonitor.org/emon/modules/emontx/firmware
 emonTx calibration instructions: http://openenergymonitor.org/emon/modules/emontx/firmware/calibration
 
*/

//CT 1 is always enabled
const int CT2 = 1;                                                      // Set to 1 to enable CT channel 2
const int CT3 = 1;                                                      // Set to 1 to enable CT channel 3

#define freq RF12_433MHZ                                                // Frequency of RF12B module can be RF12_433MHZ, RF12_868MHZ or RF12_915MHZ. You should use the one matching the module you have.433MHZ, RF12_868MHZ or RF12_915MHZ. You should use the one matching the module you have.
const int nodeID = 10;                                                  // emonTx RFM12B node ID
const int networkGroup = 210;                                           // emonTx RFM12B wireless network group - needs to be same as emonBase and emonGLCD needs to be same as emonBase and emonGLCD

const int UNO = 1;                                                      // Set to 0 if your not using the UNO bootloader (i.e using Duemilanove) - All Atmega's shipped from OpenEnergyMonitor come with Arduino Uno bootloader
#include <avr/wdt.h>                                                    // the UNO bootloader 

#include <JeeLib.h>                                                     // Download JeeLib: http://github.com/jcw/jeelib
ISR(WDT_vect) { Sleepy::watchdogEvent(); }

#include "EmonLib.h"
EnergyMonitor ct1,ct2,ct3;                                              // Create  instances for each CT channel

typedef struct { int power1, power2, power3, Vrms; } PayloadTX;         // neat way of packaging data for RF comms
PayloadTX emontx;

const int LEDpin = 9;                                                   // On-board emonTx LED 

void setup() 
{
  Serial.begin(9600);
  Serial.println("emonTX CT123 Voltage example");
  Serial.println("OpenEnergyMonitor.org");
  Serial.print("Node: "); 
  Serial.print(nodeID); 
  Serial.print(" Freq: "); 
  if (freq == RF12_433MHZ) Serial.print("433Mhz");
  if (freq == RF12_868MHZ) Serial.print("868Mhz");
  if (freq == RF12_915MHZ) Serial.print("915Mhz"); 
  Serial.print(" Network: "); 
  Serial.println(networkGroup);
  
  ct1.voltageTX(234.26, 1.7);                                         // ct.voltageTX(calibration, phase_shift) - make sure to select correct calibration for AC-AC adapter  http://openenergymonitor.org/emon/modules/emontx/firmware/calibration
  ct1.currentTX(1, 111.1);                                            // Setup emonTX CT channel (channel (1,2 or 3), calibration)
                                                                      // CT Calibration factor = CT ratio / burden resistance
  ct2.voltageTX(234.26, 1.7);                                         // CT Calibration factor = (100A / 0.05A) x 18 Ohms
  ct2.currentTX(2, 111.1);
  
  ct3.voltageTX(234.26, 1.7);
  ct3.currentTX(3, 111.1);
  
  rf12_initialize(nodeID, freq, networkGroup);                          // initialize RF
  rf12_sleep(RF12_SLEEP);

  pinMode(LEDpin, OUTPUT);                                              // Setup indicator LED
  digitalWrite(LEDpin, HIGH);
  
  if (UNO) wdt_enable(WDTO_8S);                                         // Enable anti crash (restart) watchdog if UNO bootloader is selected. Watchdog does not work with duemilanove bootloader                                                             // Restarts emonTx if sketch hangs for more than 8s
}

void loop() 
{ 
  ct1.calcVI(20,2000);                                                  // Calculate all. No.of wavelengths, time-out 
  emontx.power1 = ct1.realPower;
  Serial.print(emontx.power1); 
  
  emontx.Vrms = ct1.Vrms*100;                                          // AC Mains rms voltage 
  
  if (CT2) {  
    ct2.calcVI(20,2000);                                               //ct.calcVI(number of wavelengths to sample across, time out (ms) if no waveform is detected)                                         
    emontx.power2 = ct2.realPower;
    Serial.print(" "); Serial.print(emontx.power2);
  }

  if (CT3) {
    ct3.calcVI(20,2000);
    emontx.power3 = ct3.realPower;
    Serial.print(" "); Serial.print(emontx.power3);
  }
  
  Serial.print(" "); Serial.print(ct1.Vrms);

  Serial.println(); delay(100);
 
  send_rf_data();                                                       // *SEND RF DATA* - see emontx_lib
  emontx_sleep(5);                                                      // sleep or delay in seconds - see emontx_lib
  digitalWrite(LEDpin, HIGH); delay(2); digitalWrite(LEDpin, LOW);      // flash LED
}
