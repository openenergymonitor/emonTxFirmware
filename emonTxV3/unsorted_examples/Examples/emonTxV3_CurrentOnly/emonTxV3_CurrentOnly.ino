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

#define emonTxV3                                                      // Tell emonLib this is the emonTx V3 - don't read Vcc assume Vcc = 3.3V as is always the case on emonTx V3 eliminates bandgap error and need for calibration http://harizanov.com/2013/09/thoughts-on-avr-adc-accuracy/

#include <RFu_JeeLib.h>                                               // Special modified version of the JeeJib library to work with the RFu328 https://github.com/openenergymonitor/RFu_jeelib        
ISR(WDT_vect) { Sleepy::watchdogEvent(); }                            // Attached JeeLib sleep function to Atmega328 watchdog -enables MCU to be put into sleep mode inbetween readings to reduce power consumption 

#include "EmonLib.h"                                                  // Include EmonLib energy monitoring library https://github.com/openenergymonitor/EmonLib
EnergyMonitor ct1, ct2, ct3, ct4;       

#define FILTERSETTLETIME 5000                                         // Time (ms) to allow the filters to settle before sending data



//----------------------------emonTx V3 Settings---------------------------------------------------------------------------------------------------------------
const byte Vrms=240;                                                   // Vrms for apparent power readings (when no AC-AC voltage sample is present)
const byte TIME_BETWEEN_READINGS=10;                                   //Time between readings   
const float Ical=90.9;
const float Ical4=16.6;
const int no_of_samples=1480; 
//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------RFM12B SETTINGS----------------------------------------------------------------------------------------------------
#define RF_freq RF12_433MHZ                                              // Frequency of RF12B module can be RF12_433MHZ, RF12_868MHZ or RF12_915MHZ. You should use the one matching the module you have.
const int nodeID = 10;                                                // emonTx RFM12B node ID
const int networkGroup = 210;  
typedef struct { int power1, power2, power3, power4; } PayloadTX;     // create structure - a neat way of packaging data for RF comms
  PayloadTX emontx; 
//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------


//Random Variables 
boolean settled = false;
const byte LEDpin=6;                                                   // emonTx V3 LED
boolean CT1, CT2, CT3, CT4, ACAC, debug; 



void setup()
{ 
  pinMode(LEDpin, OUTPUT);
  digitalWrite(LEDpin, HIGH); 
  
  rf12_initialize(nodeID,RF_freq,networkGroup);    // initialize RFM12B
  rf12_sleep(RF12_SLEEP) ;                      
  
  if (analogRead(1) > 0) CT1 = 1;               //check to see if CT is connected to CT1 input, if so enable that channel
  if (analogRead(2) > 0) CT2 = 1;               //check to see if CT is connected to CT2 input, if so enable that channel
  if (analogRead(3) > 0) CT3 = 1;               //check to see if CT is connected to CT3 input, if so enable that channel
  if (analogRead(4) > 0) CT4 = 1;               //check to see if CT is connected to CT4 input, if so enable that channel
  
  if (Serial) debug = 1; else debug=0;          //if serial UART to USB is connected show debug O/P. If not then disable serial
  
  if (debug==1)
  {
    Serial.begin(9600);
    Serial.println("emonTx V3 Current Only Example");
    Serial.println("OpenEnergyMonitor.org");
    Serial.print("Vrms assumed to be "); Serial.print(Vrms); Serial.println("V");
    Serial.print("CT 1-3 Calibration: "); Serial.println(Ical);
    Serial.print("CT 4 Calibration: "); Serial.println(Ical4);
    Serial.print("Node: "); Serial.print(nodeID); 
    Serial.print(" Freq: "); 
    if (RF_freq == RF12_433MHZ) Serial.print("433Mhz");
    if (RF_freq == RF12_868MHZ) Serial.print("868Mhz");
    if (RF_freq == RF12_915MHZ) Serial.print("915Mhz"); 
    Serial.print(" Network: "); Serial.println(networkGroup);
  }
  
  if (CT1) ct1.current(1, Ical);             // CT ADC channel 1, calibration.  calibration (2000 turns / 22 Ohm burden resistor = 90.909)
  if (CT2) ct2.current(2, Ical);             // CT ADC channel 2, calibration.
  if (CT3) ct3.current(3, Ical);             // CT ADC channel 3, calibration. 
  //CT 3 is high accuracy @ low power -  4.5kW Max @ 240V 
  if (CT4) ct4.current(4, Ical4);                                      // CT channel ADC 4, calibration.    calibration (2000 turns / 120 Ohm burden resistor = 16.66)
  
 
 delay(5000); digitalWrite(LEDpin, LOW);                                //turn on then off LED to indicate power up has finished
  
}


void loop()
{
  
  if (CT1) {
  emontx.power1 = ct1.calcIrms(no_of_samples)*Vrms;                               // Calculate Apparent Power 1  1480 is  number of samples
  if (debug==1) Serial.print(emontx.power1); Serial.print(" ");

  }
  
  if (CT2) {
  emontx.power2 = ct2.calcIrms(no_of_samples)*Vrms;                               // Calculate Apparent Power 2 1480 is  number of samples
  if (debug==1) Serial.print(emontx.power2); Serial.print(" ");  
  }
  
  if (CT3) {
  emontx.power3 = ct3.calcIrms(no_of_samples)*Vrms;                               // Calculate Apparent Power 3 - 1480 is  number of samples
  if (debug==1) Serial.print(emontx.power3); Serial.print(" "); 
  }
  
  if (CT4) {
  emontx.power4 = ct4.calcIrms(no_of_samples)*Vrms;                               // Calculate Apparent Power 4 - 1480 is  number of samples
  if (debug==1) Serial.print(emontx.power4); Serial.print(" "); 
  }
  
  if (debug==1) {Serial.println(); delay(20);}
  
  // because millis() returns to zero after 50 days ! 
  if (!settled && millis() > FILTERSETTLETIME) settled = true;

  if (settled)                                                            // send data only after filters have settled
  { 
    send_rf_data();                                                       // *SEND RF DATA* - see emontx_lib
    //digitalWrite(LEDpin, HIGH); delay(5); digitalWrite(LEDpin, LOW);    // flash LED - turn off to save power
    emontx_sleep(TIME_BETWEEN_READINGS);                                  // sleep or delay in seconds 
  }  
 
}

void send_rf_data()
{
  rf12_sleep(RF12_WAKEUP);                                   
  rf12_sendNow(0, &emontx, sizeof emontx);                           //send temperature data via RFM12B using new rf12_sendNow wrapper
  rf12_sendWait(2);
  rf12_sleep(RF12_SLEEP);
}

void emontx_sleep(int seconds) {
  Sleepy::loseSomeTime(seconds*1000);
}
