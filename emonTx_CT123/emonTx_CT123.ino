// emonTX CT123 example
// Part of the openenergymonitor.org project
// Licence: GNU GPL V3
// Authors: Glyn Hudson, Trystan Lea
// Builds upon JeeLabs RF12 library and Arduino
//
// When doing a long term instal its worth deleting
// the Serial.begin and print statements

#define freq RF12_433MHZ                                                //Frequency of RF12B module can be RF12_433MHZ, RF12_868MHZ or RF12_915MHZ. You should use the one matching the module you have.
#define nodeID 10                                                       //emonTx node ID
#define networkGroup 210                                                //emonTx RFM12B wireless network group - needs to be same as emonBase and emonGLCD

#define UNO                                                             // Uncomment this line if your not using the UNO bootloader (i.e using Duemilanove) - All Atmega's shipped from OpenEnergyMonitor come witj Arduino Uno bootloader
#include <avr/wdt.h>                                                    // 

#include <JeeLib.h>                                                     // Download JeeLib: http://github.com/jcw/jeelib
ISR(WDT_vect) { Sleepy::watchdogEvent(); }                              //Attached JeeLib sleep function to Atmega328 watchdog -enables MCU to be put into sleep mode inbetween readings to reduce power consumption 

#include "EmonLib.h"
EnergyMonitor emon1;                                                    // Create  instances for each CT channel
// EnergyMonitor emon2;                                                 // Uncomment for 2 CT's
// EnergyMonitor emon3;                                                 // Uncomment for 3 CT's

typedef struct {                                                        // Create emontx data structure
  int power1;
  // int power2;                                                        // Uncomment for 2 CT's
  // int power3;                                                        // Uncomment for 3 CT's
  int battery;
} PayloadTX;
PayloadTX emontx;                                                       // neat way of packaging data for RF comms

# define LEDpin 9                                                       //On-board emonTx LED 

void setup() 
{
  Serial.begin(9600);
  Serial.println("emonTX CT123 example"); Serial.println("OpenEnergyMonitor.org");
             
  emon1.currentTX(1, 115.6);                                            // Setup emonTX CT channel (channel, calibration)
  // emon2.currentTX(2, 115.6);                                         // Uncomment for 2 CT's
  // emon3.currentTX(3, 115.6);                                         // Uncomment for 3 CT's
  
  rf12_initialize(nodeID, freq, networkGroup);                          // initialize RFM12B
  rf12_sleep(RF12_SLEEP);

  pinMode(LEDpin, OUTPUT);                                              // Setup indicator LED
  digitalWrite(LEDpin, HIGH);
  
  #ifdef UNO
  wdt_enable(WDTO_8S);                                                  // Enable anti crash (restart) watchdog if UNO bootloader is selected. Watchdog does not work with duemilanove bootloader
  #endif                                                                // Restarts emonTx if sketch hangs for more than 8s
}

void loop() 
{ 
  emontx.power1 = emon1.calcIrms(1480) * 240.0;                         // Calculate CT 1 power
  // emontx.power2 = emon2.calcIrms(1480) * 240.0;                      // Uncomment for 2 CT's
  // emontx.power3 = emon3.calcIrms(1480) * 240.0;                      // Uncomment for 3 CT's
  
  emontx.battery = emon1.readVcc();
  
  Serial.print(emontx.power1);                                          // Output to serial
  // Serial.print(" "); Serial.print(emontx.power2);                    // Uncomment to print 2nd CT
  // Serial.print(" "); Serial.print(emontx.power3);                    // Uncomment to print 3rd CT
  Serial.println();
 
  send_rf_data();                                                       // *SEND RF DATA* - see emontx_lib
  emontx_sleep(10);                                                     // sleep or delay in seconds - see emontx_lib
  digitalWrite(LEDpin, HIGH); delay(2); digitalWrite(LEDpin, LOW);      // flash LED
}
