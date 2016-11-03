/*
 EmonTx CT123 + Voltage example. noRF

 An example sketch for the emontx module for
 CT and AC voltage sample electricity monitoring. Enables real power and Vrms calculations.

 Part of the openenergymonitor.org project
 Licence: GNU GPL V3

 Authors: Glyn Hudson, Trystan Lea
 Builds upon JeeLabs RF12 library and Arduino

 emonTx documentation: http://openenergymonitor.org/emon/modules/emontxshield/
 emonTx firmware code explination: http://openenergymonitor.org/emon/modules/emontx/firmware
 emonTx calibration instructions: http://openenergymonitor.org/emon/modules/emontx/firmware/calibration

 THIS SKETCH REQUIRES:

 Libraries in the standard arduino libraries folder:
	- EmonLib		https://github.com/openenergymonitor/EmonLib.git

 Other files in project directory (should appear in the arduino tabs above)
	- emontx_lib.ino

*/

/*Recommended node ID allocation
------------------------------------------------------------------------------------------------------------
-ID-	-Node Type-
0	- Special allocation in JeeLib RFM12 driver - reserved for OOK use
1-4     - Control nodes
5-10	- Energy monitoring nodes
11-14	--Un-assigned --
15-16	- Base Station & logging nodes
17-30	- Environmental sensing nodes (temperature humidity etc.)
31	- Special allocation in JeeLib RFM12 driver - Node31 can communicate with nodes on any network group
-------------------------------------------------------------------------------------------------------------
*/

//CT 1 is always enabled
const int CT2 = 1;                                                      // Set to 1 to enable CT channel 2
const int CT3 = 1;                                                      // Set to 1 to enable CT channel 3

#include "EmonLib.h"
EnergyMonitor ct1,ct2,ct3;                                              // Create  instances for each CT channel

typedef struct { int power1, power2, power3, Vrms; } PayloadTX;         // neat way of packaging data for RF comms
PayloadTX emontx;

const int LEDpin = 9;                                                   // On-board emonTx LED


void setup()
{
  Serial.begin(115200);
  Serial.println("emonTX CT123 no RF Voltage example");
  Serial.println("OpenEnergyMonitor.org");

  ct1.voltageTX(251.6, 1.7);                                         // ct.voltageTX(calibration, phase_shift) - make sure to select correct calibration for AC-AC adapter  http://openenergymonitor.org/emon/modules/emontx/firmware/calibration. Default is set for Ideal Power voltage adapter.
  ct1.currentTX(1, 111.1);                                            // Setup emonTX CT channel (channel (1,2 or 3), calibration)
                                                                      // CT Calibration factor = CT ratio / burden resistance
  ct2.voltageTX(234.26, 1.7);                                         // CT Calibration factor = (100A / 0.05A) x 18 Ohms
  ct2.currentTX(2, 111.1);

  ct3.voltageTX(234.26, 1.7);
  ct3.currentTX(3, 111.1);

  pinMode(LEDpin, OUTPUT);                                              // Setup indicator LED
  digitalWrite(LEDpin, HIGH);
  delay(500);
}

void loop()
{
  ct1.calcVI(20,2000);                                                  // Calculate all. No.of crossings, time-out
  emontx.power1 = ct1.realPower;

  emontx.Vrms = ct1.Vrms*100;                                          // AC Mains rms voltage

  if (CT2) {
    ct2.calcVI(20,2000);                                               //ct.calcVI(number of crossings to sample, time out (ms) if no waveform is detected)
    emontx.power2 = ct2.realPower;
  }

  if (CT3) {
    ct3.calcVI(20,2000);
    emontx.power3 = ct3.realPower;
  }

  Serial.print("ct1:"); Serial.print(emontx.power1);
  if (CT2){ Serial.print(",ct2:"); Serial.print(emontx.power2);}
  if (CT3){ Serial.print(",ct3:"); Serial.print(emontx.power3);}
  Serial.print(",vrms:"); Serial.print(int(emontx.Vrms*0.01));
  Serial.println();

  digitalWrite(LEDpin, HIGH); delay(100); digitalWrite(LEDpin, LOW);      // flash LED
  delay(2000);      // sleep or delay in seconds - see emontx_lib

}
