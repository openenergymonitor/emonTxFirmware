/*
  Part 3 – Measuring real power by adding AC Voltage measurement.
  
  By measuring the AC voltage waveform in addition to the current waveform
  its possible to measure real-power which is the actual quantity domestic 
  households are usually billed for.
 
  An AC voltage measurement also allows for direction of power flow measurement
  which is especially useful when measuring if you are importing or exporting
  power from a Solar PV system.
  
  We configure the voltage measurement by setting the voltage calibration (see calibration)
  http://openenergymonitor.org/emon/buildingblocks/ct-and-ac-power-adaptor-installation-and-calibration-theory
  
  ct1.voltageTX(234.26, 1.7);
  
  and then run the calculation function that calculates realpower, apparent power, 
  Vrms, Irms and power factor by calling: 
  
  ct1.calcVI(20,2000);
  
  EmonLib has a function to print out all of the calculated values called serialprint()
  
  if you'd like to access the variables individually 
  
  Serial.println( ct1.realpower );
  
  REAL POWER: ct1.realPower
  APPARENT POWER ct1.apparentPower
  RMS VOLTAGE: ct1.Vrms
  RMS CURRENT: ct1.Irms
  POWER FACTOR: ct1.powerFactor  

  For details on the calculations that occur in EmonLib.h see this page on:
  AC Power Theory - Arduino maths for an overview. http://openenergymonitor.org/emon/buildingblocks/ac-power-arduino-maths

  -----------------------------------------
  Part of the openenergymonitor.org project
  Licence: GNU GPL V3
*/ 

#include "EmonLib.h"                   // Include Emon Library
EnergyMonitor ct1;                     // Create an instance

void setup()
{  
  Serial.begin(9600);
  
  ct1.voltageTX(228.268, 1.7);          // Calibration, phase_shift
  ct1.currentTX(1, 111.1);             // CT channel, calibration.
}

void loop()
{
  ct1.calcVI(20,2000);                 // Calculate all. No.of half wavelengths (crossings), time-out
  ct1.serialprint();                   // Print out all variables
}
