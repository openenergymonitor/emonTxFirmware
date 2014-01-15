/*
  
  emonTxV3 Current Only (Apparent Power) Example - Read from the four CT channels and Tx values via SRF wireless 
  This is a low power sketch designed to sleep inbetween readings to converve power when running of batteries
  
  *BETA CODE* - THIS EXAMPLE HAS NOT BEEN TESTED MUCH AND IS INCOMPLEATE 
  Any improvements / contributions welcome
  
   -----------------------------------------
  Part of the openenergymonitor.org project
  Licence: GNU GPL V3
  
  Uses the Ciseco LLAPSerial library
*/

/*Recommended node ID allocation
------------------------------------------------------------------------------------------------------------
Not yet definef for SRF
-------------------------------------------------------------------------------------------------------------
*/



#include "EmonLib.h"                     // http://github.com/openenergymonitor/emonlib
#define emonTxV3                                                      // Tell emonLib this is the emonTx V3 - don't read Vcc assume Vcc = 3.3V as is always the case on emonTx V3 eliminates bandgap error and need for calibration http://harizanov.com/2013/09/thoughts-on-avr-adc-accuracy/

#include <LLAPSerial.h>                  // https://github.com/CisecoPlc/LLAPSerial



#define DEVICEID "10"	                        // this is the LLAP device ID - 10 to match emonTx - must be numerical to work with emoncms
EnergyMonitor ct1, ct2, ct3, ct4;        // Create emonLib instances for each channel

#define FILTERSETTLETIME 5000      //  Time (ms) to allow the filters to settle before sending data



boolean settled = false;                                                     //emonTx V3 LED
boolean CT1, CT2, CT3, CT4; 
int Irms1, Irms2, Irms3, Irms4, Power1, Power2, Power3, Power4;

const int SLEEP_TIME_BETWEEN_READINGS=5;               //In seconds
const int Vrms=230;                                   //Hard coded VRMS
//-------------------------Define emonTx V3 hardwired connections -----------------
const byte  LEDpin=6;     
const byte ADC_AC=0;
const byte ADC_CT1=1;
const byte ADC_CT2=2;
const byte ADC_CT3=3;
const byte ADC_CT4=4;

const byte SRF_EN_PIN=8;    //SRF enable
const byte SRF_SLEEP_PIN=4 ;   //SRF Sleep - pull sleep pin high - sleep 2 disabled
//---------------------------------------------------------------------------------


void setup()
{  
  
  pinMode(LEDpin, OUTPUT); 
  if (analogRead(ADC_CT1) > 0) CT1 = 1;               //check to see if CT is connected to CT1 input, if so enable that channel
  if (analogRead(ADC_CT2) > 0) CT2 = 1;               //check to see if CT is connected to CT2 input, if so enable that channel
  if (analogRead(ADC_CT3) > 0) CT3 = 1;               //check to see if CT is connected to CT3 input, if so enable that channel
  if (analogRead(ADC_CT4) > 0) CT4 = 1;               //check to see if CT is connected to CT4 input, if so enable that channel
  
  Serial.begin(115200);
  Serial.println("emonTx V3 Current Only - SRF LLAP Example");
  
  ct1.current(ADC_CT1, 90.9);             // CT channel 1, calibration.  calibration (2000 turns / 22 Ohm burden resistor = 90.909)
  if (CT2) ct2.current(ADC_CT2, 90.9);             // CT channel 2, calibration.
  if (CT3) ct3.current(ADC_CT3, 90.9  );             // CT channel 3, calibration. 
  //CT 4 is high accuracy @ low power -  4.5kW Max 
  if (CT4) ct4.current(ADC_CT4, 16.6);             // CT channel 4, calibration.    calibration (2000 turns / 120 Ohm burden resistor = 16.66)
  
  //-------------Start UP SRF, enable SRF sleep mode 2------------------------------
  //http://openmicros.org/index.php/articles/88-ciseco-product-documentation/260-srf-configuration       
        pinMode(SRF_EN_PIN,OUTPUT);		// switch on the SRF radio
	digitalWrite(SRF_EN_PIN,HIGH);
	delay(1000);	
        pinMode(SRF_SLEEP_PIN,OUTPUT);           // hardwired XinoRF / RFu328 SRF sleep pin 
        digitalWrite(SRF_SLEEP_PIN,LOW);         // pull sleep pin high - sleep 2 disabled
	Serial.print("+++");                     // enter AT command mode
        delay(1500);                             // delay 1.5s
        Serial.println("ATSM2");                 // enable sleep mode 2 <0.5uA
        delay(2000);
        Serial.println("ATDN");                  // exit AT command mode*/
        delay(2000);
        LLAP.init(DEVICEID);                     // Start up LLAP
        LLAP.sendMessage("STARTED");
        Serial.print("ABCDEFGHIJKLMNOPQRSTUVWX");
        digitalWrite(SRF_SLEEP_PIN,HIGH);        //Send SRF to sleep
//---------------------------------------------------------------------------------
  
//-----------Initiate LLAP--------------------------------------------------
   LLAP.init(DEVICEID);
   LLAP.sendMessage("STARTED");
   Serial.print("ABCDEFGHIJKLMNOPQRSTUVWX");
 //-----------------------------------------------------------------------------
 
digitalWrite(LEDpin, HIGH);
delay(2000);
digitalWrite(LEDpin, LOW);      //turn on then off LED to indicate power up
  
}


void loop()
{
  
  //Read from CT1 as default
  Irms1 = ct1.calcIrms(1480);   // Calculate Apparent Power 1 assuming 240Vrms AC - 1480 is  number of samples
  Serial.print(Irms1); Serial.print(" ");
  Power1=Irms1*Vrms;

  
  
  if (CT2) {
  Irms2 = ct2.calcIrms(1480);   // Calculate Apparent Power 2 assuming 240Vrms AC - 1480 is  number of samples
  Serial.print(Irms2); Serial.print(" ");  
  Power2=Irms2*Vrms;
  }
  
  if (CT3) {
  Irms3 = ct3.calcIrms(1480);   // Calculate Apparent Power 3 assuming 240Vrms AC - 1480 is  number of samples
  Serial.print(Irms3); Serial.print(" "); 
  Power2=Irms2*Vrms;
  }
  
  if (CT4) {
  Irms4 = ct4.calcIrms(1480);   // Calculate Apparent Power 4 assuming 240Vrms AC - 1480 is  number of samples
  Serial.print(Irms4);  Serial.print(" ");
  Power4=Irms4*Vrms;
  }
  
  Serial.println(); delay(20);
  
  // because millis() returns to zero after 50 days ! 
  if (!settled && millis() > FILTERSETTLETIME) settled = true;

  if (settled)                                                            // send data only after filters have settled
  { 
    send_rf_data();                                                       // *SEND RF DATA* - see emontx_lib
    digitalWrite(LEDpin, HIGH); delay(5); digitalWrite(LEDpin, LOW);      // flash LED
    emontx_sleep(SLEEP_TIME_BETWEEN_READINGS);                                                      // sleep or delay in seconds - see emontx_lib
  }
  
 
}

void send_rf_data()
{
  digitalWrite(SRF_SLEEP_PIN, LOW);         //  wake up SRF Radio
  delay(10);
  if (CT1) LLAP.sendInt("P1",Power1);        //name, int variable
  if (CT2) LLAP.sendInt("P2",Power2);          
  if (CT3) LLAP.sendInt("P3",Power3);              
  if (CT4) LLAP.sendInt("P4",Power4);             
  delay(10);
  digitalWrite(SRF_SLEEP_PIN, HIGH);        // pull sleep pin high to enter SRF sleep 2
 
}

void emontx_sleep(int seconds) {
  LLAP.sleepForaWhile(5000);
}
