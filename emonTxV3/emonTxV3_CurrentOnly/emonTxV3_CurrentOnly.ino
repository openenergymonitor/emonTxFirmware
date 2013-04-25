/*
  
  emonTxV3 Current Only (Apparent Power) Example - Read from the four CT channels and Tx values via RFM12B wireless 

   -----------------------------------------
  Part of the openenergymonitor.org project
  Licence: GNU GPL V3
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

#include <RFu_JeeLib.h>                        //https://github.com/openenergymonitor/RFu_jeelib        
ISR(WDT_vect) { Sleepy::watchdogEvent(); }     // Attached JeeLib sleep function to Atmega328 watchdog -enables MCU to be put into sleep mode inbetween readings to reduce power consumption 

#include "EmonLib.h"                     // Include Emon Library
EnergyMonitor ct1, ct2, ct3, ct4;        // Create two instances

#define FILTERSETTLETIME 5000         //  Time (ms) to allow the filters to settle before sending data

#define freq RF12_433MHZ                                                        // Frequency of RF12B module can be RF12_433MHZ, RF12_868MHZ or RF12_915MHZ. You should use the one matching the module you have.
const int nodeID = 10;                                                          // emonTx RFM12B node ID
const int networkGroup = 210;  

typedef struct { int power1, power2, power3, power4, battery; } PayloadTX;     // create structure - a neat way of packaging data for RF comms
  PayloadTX emontx; 

boolean settled = false;
const int LEDpin=6;                                                            //emonTx V3 LED
boolean CT1, CT2, CT3, CT4; 



void setup()
{  
  rf12_initialize(nodeID,freq,networkGroup);    // initialize RFM12B
  rf12_sleep(RF12_SLEEP) ;                       //rf12 sleep seems to cause issue on the RFu, not sure why? Need to look into this
  
  if (analogRead(1) > 0) CT1 = 1;               //check to see if CT is connected to CT1 input, if so enable that channel
  if (analogRead(2) > 0) CT2 = 1;               //check to see if CT is connected to CT2 input, if so enable that channel
  if (analogRead(3) > 0) CT3 = 1;               //check to see if CT is connected to CT3 input, if so enable that channel
  if (analogRead(4) > 0) CT4 = 1;               //check to see if CT is connected to CT4 input, if so enable that channel
  
  Serial.begin(9600);
  Serial.println("emonTx V3 Current Only Example");
  
  if (CT1) ct1.current(1, 90.909);             // CT channel 1, calibration.  calibration (2000 turns / 22 Ohm burden resistor = 90.909)
  if (CT2) ct2.current(2, 90.909);             // CT channel 2, calibration.
  if (CT3) ct3.current(3, 90.909);             // CT channel 3, calibration. 
  //CT 3 is high accuracy @ low power -  4.5kW Max 
  if (CT4) ct4.current(4, 16.66);             // CT channel 4, calibration.    calibration (2000 turns / 120 Ohm burden resistor = 16.66)
  
 
  pinMode(LEDpin, OUTPUT);
  digitalWrite(LEDpin, HIGH); delay(5000); digitalWrite(LEDpin, LOW);      //turn on then off LED to indicate power up
  
}





void loop()
{
  
  if (CT1) {
  emontx.power1 = ct1.calcIrms(1480)*240.0;   // Calculate Apparent Power 1 assuming 240Vrms AC - 1480 is  number of samples
  Serial.print(emontx.power1);  
  }
  
  if (CT2) {
  emontx.power2 = ct2.calcIrms(1480)*240.0;   // Calculate Apparent Power 2 assuming 240Vrms AC - 1480 is  number of samples
  Serial.print(emontx.power2);  
  }
  
  if (CT3) {
  emontx.power3 = ct3.calcIrms(1480)*240.0;   // Calculate Apparent Power 3 assuming 240Vrms AC - 1480 is  number of samples
  Serial.print(emontx.power3);  
  }
  
  if (CT4) {
  emontx.power4 = ct4.calcIrms(1480)*240.0;   // Calculate Apparent Power 4 assuming 240Vrms AC - 1480 is  number of samples
  Serial.print(emontx.power4);  
  }
  
  emontx.battery = ct1.readVcc();                                        //read emonTx battey (supply voltage after MPC1700 3V3 vreg on emonTxV3)
  
  Serial.print(" "); Serial.print(emontx.battery);
  Serial.println(); delay(100);
  
  // because millis() returns to zero after 50 days ! 
  if (!settled && millis() > FILTERSETTLETIME) settled = true;

  if (settled)                                                            // send data only after filters have settled
  { 
    send_rf_data();                                                       // *SEND RF DATA* - see emontx_lib
    digitalWrite(LEDpin, HIGH); delay(5); digitalWrite(LEDpin, LOW);      // flash LED
    emontx_sleep(5);                                                      // sleep or delay in seconds - see emontx_lib
  }
  
 
}

void send_rf_data()
{
  rf12_sleep(RF12_WAKEUP);                                   //TO DO need to test if RFM12 sleep works on RFu
  rf12_sendNow(0, &emontx, sizeof emontx);                     //send temperature data via RFM12B using new rf12_sendNow wrapper
  // set the sync mode to 2 if the fuses are still the Arduino default
  // mode 3 (full powerdown) can only be used with 258 CK startup fuses
  rf12_sendWait(2);
  rf12_sleep(RF12_SLEEP);
}

void emontx_sleep(int seconds) {
  Sleepy::loseSomeTime(seconds*1000);
}
