// EmonESP example for the EmonTx v3
// Supports CT + ACAC power monitoring and pulse counting.
// Licence: GPLv3

#include <avr/wdt.h>

#define FirmwareVersion = 1.0
#define DEBUG 0
#define emonTxV3 1
#define PULSE_ENABLE 0

#include "EmonLib.h"                   // Include Emon Library:  https://github.com/openenergymonitor/EmonLib
EnergyMonitor ct1;                     // Create an instance
EnergyMonitor ct2;                     // Create an instance
EnergyMonitor ct3;                     // Create an instance
EnergyMonitor ct4;                     // Create an instance

float Vcal =          268.97;          // (230V x 13) / (9V x 1.2) = 276.9 Calibration for UK AC-AC adapter 77DB-06-09 
const float Ical1 =   90.9;            // (2000 turns / 22 Ohm burden) = 90.9
const float Ical2 =   90.9;            // (2000 turns / 22 Ohm burden) = 90.9
const float Ical3 =   90.9;            // (2000 turns / 22 Ohm burden) = 90.9
const float Ical4 =   16.67;           // (2000 turns / 120 Ohm burden) = 16.67

const float phase_shift = 1.7;

unsigned long last = 0;
unsigned long now = 0;
unsigned long lastwdtreset = 0;

// pulseCounting
long pulseCount = 0;

int joules_CT1 = 0;
int joules_CT2 = 0;
int joules_CT3 = 0;
int joules_CT4 = 0;
unsigned long CT1_Wh = 0;
unsigned long CT2_Wh = 0;
unsigned long CT3_Wh = 0;
unsigned long CT4_Wh = 0;

unsigned long msgnum = 0;

bool firstrun = true;
unsigned long last_reading = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Startup");

  ct1.voltage(0, Vcal, phase_shift);
  ct1.current(1, Ical1);
  ct2.voltage(0, Vcal, phase_shift);
  ct2.current(2, Ical2);
  ct3.voltage(0, Vcal, phase_shift);
  ct3.current(3, Ical3);
  ct4.voltage(0, Vcal, phase_shift);
  ct4.current(4, Ical4);

  delay(100);
  if (PULSE_ENABLE) attachInterrupt(1, onPulse, FALLING);

  CT1_Wh = 0;
  CT2_Wh = 0;
  CT3_Wh = 0;
  CT4_Wh = 0;
  
  wdt_enable(WDTO_8S);
  wdt_reset();

}

void loop() {
  now = millis();

  if ((now - last) >= 9800 || firstrun) {
    wdt_reset();
    last = now; firstrun = false;

    // Reading of CT sensors needs to go here for stability
    // need to double check the reason for this.
    for (int i = 0; i < 10; i++) {
      analogRead(0); analogRead(1); analogRead(2);
    }
    delay(200);
    ct1.calcVI(30, 2000);
    wdt_reset();
    int OEMct1 = ct1.realPower;
    ct2.calcVI(30, 2000);
    wdt_reset();
    int OEMct2 = ct2.realPower;
    ct2.calcVI(30, 2000);
    wdt_reset();
    int OEMct3 = ct3.realPower;
    ct4.calcVI(30, 2000);
    wdt_reset();
    int OEMct4 = ct4.realPower;

    // Accumulating Watt hours
    int interval = millis() - last_reading;
    last_reading = millis();

    if (ct1.realPower > 0) {
      joules_CT1 += (ct1.realPower * interval * 0.001);
      CT1_Wh += joules_CT1 / 3600;
      joules_CT1 = joules_CT1 % 3600;
    }

    if (ct2.realPower > 0) {
      joules_CT2 += (ct2.realPower * interval * 0.001);
      CT2_Wh += joules_CT2 / 3600;
      joules_CT2 = joules_CT2 % 3600;
    }

    if (ct3.realPower > 0) {
      joules_CT3 += (ct3.realPower * interval * 0.001);
      CT3_Wh += joules_CT3 / 3600;
      joules_CT3 = joules_CT3 % 3600;
    }

    if (ct4.realPower > 0) {
      joules_CT4 += (ct4.realPower * interval * 0.001);
      CT4_Wh += joules_CT4 / 3600;
      joules_CT4 = joules_CT4 % 3600;
    }
    wdt_reset();

    msgnum++;
    Serial.print("MSG:"); Serial.print(msgnum);
    Serial.print(",P1:"); Serial.print(OEMct1);
    Serial.print(",P2:"); Serial.print(OEMct2);
    Serial.print(",P3:"); Serial.print(OEMct3);
    Serial.print(",P4:"); Serial.print(OEMct4);
    Serial.print(",E1:"); Serial.print(CT1_Wh);
    Serial.print(",E2:"); Serial.print(CT2_Wh);
    Serial.print(",E3:"); Serial.print(CT3_Wh);
    Serial.print(",E4:"); Serial.print(CT4_Wh);
    
    if (PULSE_ENABLE) {
      Serial.print(",PULSE:"); Serial.print(pulseCount);
    }
    Serial.println();
    delay(200);
  }

  if ((millis() - lastwdtreset) > 1000) {
    lastwdtreset = millis();
    wdt_reset();
  }
}

// The interrupt routine - runs each time a falling edge of a pulse is detected
void onPulse()
{
  pulseCount++;                                               // count pulse
}
