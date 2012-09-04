/*
  Part 3 – Measuring real power by adding AC Voltage measurement.
*/ 

#include "EmonLib.h"                   // Include Emon Library
EnergyMonitor ct1;                     // Create an instance

void setup()
{  
  Serial.begin(9600);
  
  ct1.voltageTX(238.5, 1.7);           // Calibration, phase_shift
  ct1.currentTX(1, 115.6);             // CT channel, calibration.
}

void loop()
{
  ct1.calcVI(20,2000);                 // Calculate all. No.of wavelengths, time-out
  ct1.serialprint();                   // Print out all variables
}
