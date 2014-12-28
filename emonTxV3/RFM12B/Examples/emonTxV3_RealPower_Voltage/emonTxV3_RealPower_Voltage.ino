/*
  
  emonTxV3 Real Power Example - Read from the four CT channels and Tx values via RFM12B 
  Requires connection of 9V AC-AC adapter for AC voltage sample
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

#define emonTxV3                                                      // Tell emonLib this is the emonTx V3 - don't read Vcc assume Vcc = 3.3V as is always the case on emonTx V3 eliminates bandgap error and need for calibration http://harizanov.com/2013/09/thoughts-on-avr-adc-accuracy/

#include <RFu_JeeLib.h>                        //https://github.com/openenergymonitor/RFu_jeelib        
ISR(WDT_vect) { Sleepy::watchdogEvent(); }     // Attached JeeLib sleep function to Atmega328 watchdog -enables MCU to be put into sleep mode inbetween readings to reduce power consumption 

#include "EmonLib.h"                     // Include Emon Library
EnergyMonitor ct1, ct2, ct3, ct4;        // Create two instances

#define FILTERSETTLETIME 5000         //  Time (ms) to allow the filters to settle before sending data

#define RF_freq RF12_433MHZ                                                        // Frequency of RF12B module can be RF12_433MHZ, RF12_868MHZ or RF12_915MHZ. You should use the one matching the module you have.
const int nodeID = 10;                                                          // emonTx RFM12B node ID
const int networkGroup = 210;  

typedef struct { int power1, power2, power3, power4, Vrms; } PayloadTX;     // create structure - a neat way of packaging data for RF comms
  PayloadTX emontx; 

boolean settled = false;
const int LEDpin=6;                                                            //emonTx V3 LED
boolean CT1, CT2, CT3, CT4; 
                                                 //time between readings in s



void setup()
{  
  rf12_initialize(nodeID,RF_freq,networkGroup);    // initialize RFM12B
  rf12_sleep(RF12_SLEEP) ;                       //rf12 sleep seems to cause issue on the RFu, not sure why? Need to look into this
  
  //emonTxV3 CT channel will read ADC 0 when nothing is connected due to switched jack plugs connecting ADC to 0V when no jack is inserted
  if (analogRead(1) > 0) CT1 = 1;               //check to see if CT is connected to CT1 input, if so enable that channel 
  if (analogRead(2) > 0) CT2 = 1;               //check to see if CT is connected to CT2 input, if so enable that channel
  if (analogRead(3) > 0) CT3 = 1;               //check to see if CT is connected to CT3 input, if so enable that channel
  if (analogRead(4) > 0) CT4 = 1;               //check to see if CT is connected to CT4 input, if so enable that channel
  
  Serial.begin(9600);
  Serial.println("emonTx V3 Real Power Example");
  
  // (230V x 13) / (9V * 1.2) = 276.9
  // 13: voltage divider factor
  // 1.2: 20% output voltage increase due to open circuit)
  ct1.voltage(0, 276.9, 1.7);          // Calibration, phase_shift 
  ct2.voltage(0, 276.9, 1.7);          // Calibration, phase_shift
  ct3.voltage(0, 276.9, 1.7);          // Calibration, phase_shift
  ct4.voltage(0, 276.9, 1.7);          // Calibration, phase_shift
  
  ct1.current(1, 90.9);             // CT channel 1, calibration.  calibration (2000 turns / 22 Ohm burden resistor = 90.909)
  ct2.current(2, 90.9);             // CT channel 2, calibration.
  ct3.current(3, 90.9);             // CT channel 3, calibration. 
  //CT 4 is high accuracy @ low power -  4.5kW Max 
  ct4.current(4, 16.6);             // CT channel 4, calibration.    calibration (2000 turns / 120 Ohm burden resistor = 16.66)
  
   
  pinMode(LEDpin, OUTPUT);
  digitalWrite(LEDpin, HIGH); delay(5000); digitalWrite(LEDpin, LOW);      //long flash LED to indicate power up success
  
}





void loop()
{
  if (CT1) {
  ct1.calcVI(20,2000);                 // Calculate all. No.of half wavelengths (crossings), time-out  
  emontx.power1 = ct1.realPower;
  Serial.print(emontx.power1); Serial.print(" "); 
  //Serial.print(ct1.apparentPower);
  //Serial.print(ct1.powerFactor);
  //Serial.print(ct1.apparentIrms);
  }
  
  if (CT2) {
  ct2.calcVI(20,2000);                 // Calculate all. No.of half wavelengths (crossings), time-out  
  emontx.power2 = ct2.realPower;
  Serial.print(emontx.power2); Serial.print(" "); 
}
  
  if (CT3) {
  ct3.calcVI(20,2000);                 // Calculate all. No.of half wavelengths (crossings), time-out  
  emontx.power3 = ct3.realPower; 
  Serial.print(emontx.power3); Serial.print(" "); 
}
  
  if (CT4) {
  ct4.calcVI(20,2000);                 // Calculate all. No.of half wavelengths (crossings), time-out  
  emontx.power4 = ct4.realPower;  
  Serial.print(emontx.power4); Serial.print(" ");   
}
  
  emontx.Vrms = ct1.Vrms*100;         //AC RMS voltage - convert to integer ready for RF transmission (multiply by 0.01 using emoncms input process to convert back to two decimal places)
  

  
  Serial.println(emontx.Vrms);
  delay(20);
  
  // because millis() returns to zero after 50 days ! 
  if (!settled && millis() > FILTERSETTLETIME) settled = true;

  if (settled)                                                            // send data only after filters have settled
  { 
    send_rf_data();                                                       // *SEND RF DATA* - see emontx_lib
    digitalWrite(LEDpin, HIGH); delay(10); digitalWrite(LEDpin, LOW);     // flash LED
    delay(10000);                                    // use delay instead of sleep since we're powering from AC
  }
  
 
}

void send_rf_data()
{
  rf12_sleep(RF12_WAKEUP);                                   //TO DO need to test if RFM12 sleep works on RFu
  rf12_sendNow(0, &emontx, sizeof emontx);                   //send temperature data via RFM12B using new rf12_sendNow wrapper
  // set the sync mode to 2 if the fuses are still the Arduino default
  // mode 3 (full powerdown) can only be used with 258 CK startup fuses
  rf12_sendWait(2);
  rf12_sleep(RF12_SLEEP);
}

