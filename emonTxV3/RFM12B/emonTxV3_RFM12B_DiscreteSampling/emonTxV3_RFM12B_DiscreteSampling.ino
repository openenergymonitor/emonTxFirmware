/*
  
  emonTxV3 Discrete Sampling
  
  If AC-AC adapter is detected assume emonTx is also powered from adapter (jumper shorted) and take Real Power Readings and disable sleep mode to keep load on power supply constant
  If AC-AC addapter is not detected assume powering from battereis / USB 5V AC sample is not present so take Apparent Power Readings and enable sleep mode
  
  Transmitt values via RFM12B radio
  
   -----------------------------------------
  Part of the openenergymonitor.org project
  
  Authors: Glyn Hudson & Trystan Lea 
  Builds upon JCW JeeLabs RF12 library and Arduino 
  
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
const byte Vrms=                  240;                                                   // Vrms for apparent power readings (when no AC-AC voltage sample is present)
const byte TIME_BETWEEN_READINGS= 10;                                   //Time between readings   
const float Ical=                 85.75996;
const float Ical4=                16.66;
const float Vcal=                 270.89;
const float phase_shift=          1.7;
const int no_of_samples=          1480; 
const int no_of_half_wavelengths= 20;
const int timeout=                2000;
//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------


//-----------------------RFM12B SETTINGS----------------------------------------------------------------------------------------------------
#define freq RF12_433MHZ                                              // Frequency of RF12B module can be RF12_433MHZ, RF12_868MHZ or RF12_915MHZ. You should use the one matching the module you have.
const int nodeID = 10;                                                // emonTx RFM12B node ID
const int networkGroup = 210;  
typedef struct { int power1, power2, power3, power4, Vrms; } PayloadTX;     // create structure - a neat way of packaging data for RF comms
  PayloadTX emontx; 
//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------


//Random Variables 
boolean settled = false;
const byte LEDpin=6;                                                   // emonTx V3 LED
boolean CT1, CT2, CT3, CT4, ACAC, debug; 



void setup()
{ 
 
  rf12_initialize(nodeID,freq,networkGroup);    // initialize RFM12B and send test sequency
  
  delay(20); emontx.power1=1; 
  rf12_sendNow(0, &emontx, sizeof emontx);
  delay(10); emontx.power1=2; 
  rf12_sendNow(0, &emontx, sizeof emontx);
  delay(10); emontx.power1=3; 
  rf12_sendNow(0, &emontx, sizeof emontx);
  rf12_sendWait(2);
  
  rf12_sleep(RF12_SLEEP) ;                      
  
  if (analogRead(1) > 0) CT1 = 1;               //check to see if CT is connected to CT1 input, if so enable that channel
  if (analogRead(2) > 0) CT2 = 1;               //check to see if CT is connected to CT2 input, if so enable that channel
  if (analogRead(3) > 0) CT3 = 1;               //check to see if CT is connected to CT3 input, if so enable that channel
  if (analogRead(4) > 0) CT4 = 1;               //check to see if CT is connected to CT4 input, if so enable that channel
  if (analogRead(0) > 500) ACAC=1;              //check for presence of AC-AC adapter 
  
  if (Serial) debug = 1; else debug=0;          //if serial UART to USB is connected show debug O/P. If not then disable serial
  
  if (debug==1)
  {
    Serial.begin(9600);
    Serial.println("emonTx V3 Current Only Example");
    Serial.println("OpenEnergyMonitor.org");
    Serial.print("CT 1-3 Calibration: "); Serial.println(Ical);
    Serial.print("CT 4 Calibration: "); Serial.println(Ical4);
    if (ACAC) 
    {
      Serial.print("AC-AC adapter detected - Real Power measurements enabled");
      Serial.println("assuming powering from AC-AC adapter (jumper closed)");
      Serial.print("Vcal: "); Serial.println(Vcal);
      Serial.print("Phase Shift: "); Serial.println(phase_shift);
    }
     else 
     {
       Serial.println("AC-AC adapter NOT detected - Apparent Power measurements enabled");
       Serial.print("Assuming VRMS to be "); Serial.print(Vrms); Serial.println("V");
       Serial.println("Assuming powering from batteries / 5V USB - power saving mode enabled");
     }  

       
    if (CT1) Serial.println("CT 1 detected");
    if (CT2) Serial.println("CT 2 detected");
    if (CT3) Serial.println("CT 3 detected");
    if (CT4) Serial.println("CT 4 detected");
    
    Serial.print("Node: "); Serial.print(nodeID); 
    Serial.print(" Freq: "); 
    if (freq == RF12_433MHZ) Serial.print("433Mhz");
    if (freq == RF12_868MHZ) Serial.print("868Mhz");
    if (freq == RF12_915MHZ) Serial.print("915Mhz"); 
    Serial.print(" Network: "); Serial.println(networkGroup);
  }
  
  
    
  if (CT1) ct1.current(1, Ical);             // CT ADC channel 1, calibration.  calibration (2000 turns / 22 Ohm burden resistor = 90.909)
  if (CT2) ct2.current(2, Ical);             // CT ADC channel 2, calibration.
  if (CT3) ct3.current(3, Ical);             // CT ADC channel 3, calibration. 
  //CT 3 is high accuracy @ low power -  4.5kW Max @ 240V 
  if (CT4) ct4.current(4, Ical4);                                      // CT channel ADC 4, calibration.    calibration (2000 turns / 120 Ohm burden resistor = 16.66)
  
  if (ACAC)
  {
    if (CT1) ct1.voltage(0, Vcal, phase_shift);          // ADC pin, Calibration, phase_shift
    if (CT2) ct2.voltage(0, Vcal, phase_shift);          // ADC pin, Calibration, phase_shift
    if (CT3) ct3.voltage(0, Vcal, phase_shift);          // ADC pin, Calibration, phase_shift
    if (CT4) ct4.voltage(0, Vcal, phase_shift);          // ADC pin, Calibration, phase_shift
  }
 
 pinMode(LEDpin, OUTPUT);  
 
 if (ACAC) 
 {
    for (int i; i <=5; i++) 
    { 
      digitalWrite(LEDpin, HIGH); delay(1000);
      digitalWrite(LEDpin, LOW); delay(1000);
    }
 }
 else 
 {
   digitalWrite(LEDpin, HIGH); delay(2000); digitalWrite(LEDpin, LOW);                               
 }
}

void loop()
{
  
  if (ACAC) delay(200);                                //if powering from AC-AC allow time for power supply to settle                       
  if (CT1) 
  {
   if (ACAC) 
   {
     ct1.calcVI(no_of_half_wavelengths,timeout); emontx.power1=ct1.realPower;
   }
   else
     emontx.power1 = ct1.calcIrms(no_of_samples)*Vrms;                               // Calculate Apparent Power 1  1480 is  number of samples
   if (debug==1) Serial.print(emontx.power1); 

  }
  
  if (CT1) 
  {
   if (ACAC) 
   {
     ct2.calcVI(no_of_half_wavelengths,timeout); emontx.power2=ct2.realPower;
   }
   else
     emontx.power2 = ct2.calcIrms(no_of_samples)*Vrms;                               // Calculate Apparent Power 1  1480 is  number of samples
   if (debug==1) Serial.print(emontx.power2); 

  }

  if (CT3) 
  {
   if (ACAC) 
   {
     ct3.calcVI(no_of_half_wavelengths,timeout); emontx.power3=ct3.realPower;
   }
   else
     emontx.power3 = ct3.calcIrms(no_of_samples)*Vrms;                               // Calculate Apparent Power 1  1480 is  number of samples
   if (debug==1) Serial.print(emontx.power3); 

  }
  

  if (CT4) 
  {
   if (ACAC) 
   {
     ct4.calcVI(no_of_half_wavelengths,timeout); emontx.power4=ct4.realPower;
   }
   else
     emontx.power4 = ct4.calcIrms(no_of_samples)*Vrms;                               // Calculate Apparent Power 1  1480 is  number of samples
   if (debug==1) Serial.print(emontx.power4); 

  }
  
  
  if (ACAC) emontx.Vrms = ct1.Vrms*100;                                             //AC RMS voltage - convert to integer ready for RF transmission (divide by 0.1 using emoncms input process to convert back to one decimal places) 
 
  if (debug==1) {Serial.println(); delay(20);}
  
  // because millis() returns to zero after 50 days ! 
  if (!settled && millis() > FILTERSETTLETIME) settled = true;

  if (settled)                                                            // send data only after filters have settled
  { 
    send_rf_data();                                                       // *SEND RF DATA* - see emontx_lib
    
    if (ACAC)
    {
     delay(TIME_BETWEEN_READINGS*1000);
     digitalWrite(LEDpin, HIGH); delay(10); digitalWrite(LEDpin, LOW);    // flash LED - turn off to save power
    }
    
    else
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
