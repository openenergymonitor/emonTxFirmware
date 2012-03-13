// emonTX CT123 example
// Part of the openenergymonitor.org project
// Licence: GNU GPL V3
// Authors: Glyn Hudson, Trystan Lea
// Builds upon JeeLabs RF12 library and Arduino
//
// When doing a long term instal its worth deleting
// the Serial.begin and print statements 

#define UNO                                                             // Uncomment this line if your not using
#include <avr/wdt.h>                                                    // the UNO bootloader 

#include <JeeLib.h>                                                     // Download JeeLib: http://github.com/jcw/jeelib
ISR(WDT_vect) { Sleepy::watchdogEvent(); }

#include "EmonLib.h"
EnergyMonitor emon1;                                                    // Create  instances for each CT channel
EnergyMonitor emon2;
EnergyMonitor emon3;

typedef struct { int power1, power2, power3, battery; } PayloadTX;      // Create emontx data structure
PayloadTX emontx;                                                       // neat way of packaging data for RF comms

# define LEDpin 9

void setup() 
{
  Serial.begin(9600);
  Serial.println("emonTX CT123 example");
             
  emon1.currentTX(1, 115.6);                                            // Setup emonTX CT channel (channel, calibration)
  emon2.currentTX(2, 115.6);
  emon3.currentTX(3, 115.6);
  
  rf12_initialize(10, RF12_433MHZ, 210);                                // initialize RF
  rf12_sleep(RF12_SLEEP);

  pinMode(LEDpin, OUTPUT);                                              // Setup indicator LED
  digitalWrite(LEDpin, HIGH);
  
  #ifdef UNO
  wdt_enable(WDTO_8S);                                                  // Enable watchdog if UNO
  #endif
}

void loop() 
{ 
  emontx.power1 = emon1.calcIrms(1480) * 240.0;                         // Calculate CT 1 power
  emontx.power2 = emon2.calcIrms(1480) * 240.0;
  emontx.power3 = emon3.calcIrms(1480) * 240.0;
  emontx.battery = emon1.readVcc();
  
  Serial.print(emontx.power1);                                          // Output to serial
  Serial.print(" ");
  Serial.print(emontx.power2);
  Serial.print(" ");
  Serial.println(emontx.power3);
 
  send_rf_data();                                                       // *SEND RF DATA* - see emontx_lib
  emontx_sleep(10);                                                     // sleep or delay in seconds - see emontx_lib
  digitalWrite(LEDpin, HIGH); delay(2); digitalWrite(LEDpin, LOW);      // flash LED
}
