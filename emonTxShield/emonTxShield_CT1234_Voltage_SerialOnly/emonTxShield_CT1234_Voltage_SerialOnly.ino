/*
  EmonTx CT123 Voltage Serial Only example
  
  Part of the openenergymonitor.org project
  Licence: GNU GPL V3
  
  Author: Trystan Lea
*/

#include "EmonLib.h"

// Create  instances for each CT channel
EnergyMonitor ct1,ct2,ct3, ct4;

// On-board emonTx LED
const int LEDpin = 9;                                                    

void setup() 
{
  Serial.begin(9600);
  // while (!Serial) {}
  // wait for serial port to connect. Needed for Leonardo only
  
  Serial.println("emonTX Shield CT123 Voltage Serial Only example"); 
  Serial.println("OpenEnergyMonitor.org");
  
  // Calibration factor = CT ratio / burden resistance = (100A / 0.05A) / 33 Ohms = 60.606
  ct1.current(1, 60.606);
  ct2.current(2, 60.606);                                     
  ct3.current(3, 60.606);
  ct4.current(4, 60.606); 
  
  // (ADC input, calibration, phase_shift)
  ct1.voltage(0, 300.6, 1.7);                                
  ct2.voltage(0, 300.6, 1.7);                                
  ct3.voltage(0, 300.6, 1.7);
  ct4.voltage(0, 300.6, 1.7);
  
  // Setup indicator LED
  pinMode(LEDpin, OUTPUT);                                              
  digitalWrite(LEDpin, HIGH);                                                                                  
}

void loop() 
{ 
  // Calculate all. No.of crossings, time-out 
  ct1.calcVI(20,2000);                                                  
  ct2.calcVI(20,2000);
  ct3.calcVI(20,2000);
  ct4.calcVI(20,2000);
    
  // Print power 
  Serial.print(ct1.realPower);     
  Serial.print(" "); 
  Serial.print(ct2.realPower);
  Serial.print(" "); 
  Serial.print(ct3.realPower);
  Serial.print(" "); 
  Serial.print(ct4.realPower);
  Serial.print(" "); 
  Serial.print(ct1.Vrms);
  Serial.println();
    
  // Available properties: ct1.realPower, ct1.apparentPower, ct1.powerFactor, ct1.Irms and ct1.Vrms

  delay(5000);
}
