/*
  Part 2 - Adding another CT
  
  To measure the current flowing through another of the CT inputs you need to create
  a second instance of the EnergyMonitor class: 
  
  EnergyMonitor ct1, ct2;
  
  and set the second instance setup ct2.currentTX(*2*, 115.6); to the channel you wish to measure.

  -----------------------------------------
  Part of the openenergymonitor.org project
  Licence: GNU GPL V3
*/

#include "EmonLib.h"                   // Include Emon Library
EnergyMonitor ct1, ct2;                // Create two instances

void setup()
{  
  Serial.begin(9600);
  
  ct1.currentTX(1, 111.1);             // CT channel 1, calibration.
  ct2.currentTX(2, 111.1);             // CT channel 2, calibration.
}

void loop()
{
  double Irms1 = ct1.calcIrms(1480);   // Calculate RMS current 1
  double Irms2 = ct2.calcIrms(1480);   // Calculate RMS current 2
  
  Serial.print(Irms1);                 // Print Current 1   
  Serial.print(' '); 
  Serial.print(Irms2);                 // Print Current 2
  
  // By multiplying by the nominal voltage, we can indicate the approximate apparent power:
  Serial.print(' '); 
  Serial.print(Irms1*240.0);           // Print apparent power 1   
  Serial.print(' '); 
  Serial.println(Irms2*240.0);         // Print apparent power 2
}
