/*
  Part 1 - Measuring current with a single CT
  
  The code below is the code needed to measure how much current is flowing through a CT plugged into the emontx.
  
  For details on the rms current calculations that occur in EmonLib.h see this page on:
  AC Power Theory - Arduino maths for an overview. http://openenergymonitor.org/emon/buildingblocks/ac-power-arduino-maths

  -----------------------------------------
  Part of the openenergymonitor.org project
  Licence: GNU GPL V3
*/

#include "EmonLib.h"                   // Include Emon Library:  https://github.com/openenergymonitor/EmonLib
EnergyMonitor ct1;                     // Create an instance

void setup()
{  
  Serial.begin(9600);
  ct1.currentTX(1, 111.1);             // CT channel (see emontx PCB), calibration (2000 turns / 18 Ohm burden resistor = 111.1)
}

void loop()
{
  double Irms = ct1.calcIrms(1480);    // Calculate RMS current (1480: no. of samples)
  Serial.println(Irms);		       // Print to serial Irms
}
