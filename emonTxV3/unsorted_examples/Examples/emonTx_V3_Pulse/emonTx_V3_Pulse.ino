/*
  emonTx V3 Pulse example -----------------------------------------

  Many meters have pulse outputs, including electricity meters: single phase, 3-phase, 
  import, export.. Gas meters, Water flow meters etc

  The pulse output may be a flashing LED or a switching relay (usually solid state) or both.

  In the case of an electricity meter a pulse output corresponds to a certain amount of 
  energy passing through the meter (Kwhr/Wh). For single-phase domestic electricity meters
  (eg. Elster A100c) each pulse usually corresponds to 1 Wh (1000 pulses per kwh).  

  The code below detects the falling edge of each pulse and increment pulseCount
  
  It calculated the power by the calculating the time elapsed between pulses.
  
  Read more about pulse counting here:
  http://openenergymonitor.org/emon/buildingblocks/introduction-to-pulse-counting
 
 -----------------------------------------emonTx V3 Hardware Connections-----------------------------
 
 Connect the pulse input into emonTx V3 terminal block port 4 (IRQ 0 / Digital 2)
 If your connecting a hardwired pulse output you may need to add a pull-down resistor onto the PCB (R31)
 
 If your using an optical counter (e.g TSL256) you should connecting the power pin direct to the 3.3V or 5V (if running off 5V USB)
 
 emonTx V3.2 Terminal block: 
 port 1: 5V
 port 2: 3.3V
 port 3: GND
 port 4: IRQ 0 / Dig2

 emonTx V3.4 Terminal block: 
 port 1: 5V
 port 2: 3.3V
 port 3: GND
 port 4: IRQ 1 / Dig3
 
 We recomend powering the emonTx V3 from 5V USB when using for pulse counting opperation. 
 
 
 
 -----------------------------------------

  -----------------------------------------
  Part of the openenergymonitor.org project
  Licence: GNU GPL V3
 
  Authors: Glyn Hudson, Trystan Lea
  Builds upon JeeLabs RF12 library and Arduino

  THIS SKETCH REQUIRES:

  Libraries in the standard arduino libraries folder:
 	- RFu JeeLib		https://github.com/openenergymonitor/rfu_jeelib

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

#define RF_freq RF12_433MHZ                                                // Frequency of RF12B module can be RF12_433MHZ, RF12_868MHZ or RF12_915MHZ. You should use the one matching the module you have.433MHZ, RF12_868MHZ or RF12_915MHZ. You should use the one matching the module you have.
const int nodeID = 10;                                                  // emonTx RFM12B node ID
const int networkGroup = 210;                                           // emonTx RFM12B wireless network group - needs to be same as emonBase and emonGLCD needs to be same as emonBase and emonGLCD

const int UNO = 1;                                                      // Set to 0 if your not using the UNO bootloader (i.e using Duemilanove) - All Atmega's shipped from OpenEnergyMonitor come with Arduino Uno bootloader
#include <avr/wdt.h>                                                    // the UNO bootloader 

#include <RFu_JeeLib.h>                                                     // Download JeeLib: http://github.com/jcw/jeelib
ISR(WDT_vect) { Sleepy::watchdogEvent(); }
  
typedef struct { int power, pulse;} PayloadTX;
PayloadTX emontx;                                                        // neat way of packaging data for RF comms

const int LEDpin = 6;                                                   //emonTx V3 LED pin
const int INT_PIN = 0; 				 		        // emonTx V3.2
// const int INT_PIN = 1; 				 		// emonTx V3.4


// Pulse counting settings 
long pulseCount = 0;                                                    // Number of pulses, used to measure energy.
unsigned long pulseTime,lastTime;                                       // Used to measure power.
double power, elapsedWh;                                                // power and energy
int ppwh = 1;                                                           // 1000 pulses/kwh = 1 pulse per wh - Number of pulses per wh - found or set on the meter.


void setup() 
{
  Serial.begin(9600);
  Serial.println("emonTX V3 Pulse example");
  delay(100);
             
  rf12_initialize(nodeID, RF_freq, networkGroup);                          // initialize RF
  rf12_sleep(RF12_SLEEP);

  pinMode(LEDpin, OUTPUT);                                              // Setup indicator LED
  digitalWrite(LEDpin, HIGH);
  
  attachInterrupt(INT_PIN, onPulse, FALLING);                                 // KWH interrupt attached to IRQ 0  = Digita 2 - hardwired to emonTx V3 terminal block 
  
  if (UNO) wdt_enable(WDTO_8S);  
}

void loop() 
{ 
  emontx.pulse = pulseCount; pulseCount=0; 
  send_rf_data();  // *SEND RF DATA* - see emontx_lib

  Serial.print(emontx.power);
  Serial.print("W ");
  Serial.println(emontx.pulse);

  emontx_sleep(10);                                                     // sleep or delay in seconds - see emontx_lib
  digitalWrite(LEDpin, HIGH); delay(2); digitalWrite(LEDpin, LOW);      // flash LED
}


// The interrupt routine - runs each time a falling edge of a pulse is detected
void onPulse()                  
{
  lastTime = pulseTime;        //used to measure time between pulses.
  pulseTime = micros();
  pulseCount++;                                                      //pulseCounter               
  emontx.power = int((3600000000.0 / (pulseTime - lastTime))/ppwh);  //Calculate power
}
