/*
 emonTX Temperature and real power measurement example
 
 An example sketch for the emontx module for
 CT only electricity monitoring.
 
 Part of the openenergymonitor.org project
 Licence: GNU GPL V3
 
 Authors: Glyn Hudson, Trystan Lea
 Builds upon JeeLabs RF12 library and Arduino

 THIS SKETCH REQUIRES:

 Libraries in the standard arduino libraries folder:
	- JeeLib		https://github.com/jcw/jeelib
	- EmonLib		https://github.com/openenergymonitor/EmonLib.git
	- OneWire library	http://www.pjrc.com/teensy/td_libs_OneWire.html
	- DallasTemperature	http://download.milesburton.com/Arduino/MaximTemperature

 Other files in project directory (should appear in the arduino tabs above)
	- emontx_lib.ino
	- print_to_serial.ino
 
*/

#define freq RF12_433MHZ                                                // Frequency of RF12B module can be RF12_433MHZ, RF12_868MHZ or RF12_915MHZ. You should use the one matching the module you have.
const int nodeID = 10;                                                  // emonTx RFM12B node ID
const int networkGroup = 210;                                           // emonTx RFM12B wireless network group - needs to be same as emonBase and emonGLCD

const int UNO = 1;                                                      // Set to 0 if your not using the UNO bootloader (i.e using Duemilanove) - All Atmega's shipped from OpenEnergyMonitor come with Arduino Uno bootloader
#include <avr/wdt.h>                                                     

#include <JeeLib.h>                                                     // Download JeeLib: http://github.com/jcw/jeelib
ISR(WDT_vect) { Sleepy::watchdogEvent(); }                              // Attached JeeLib sleep function to Atmega328 watchdog -enables MCU to be put into sleep mode inbetween readings to reduce power consumption 

#include "EmonLib.h"
EnergyMonitor ct1;                                                      // Create  instances for each CT channel

#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 4                                                  // Data wire is plugged into port 2 on the Arduino
OneWire oneWire(ONE_WIRE_BUS);                                          // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire);                                    // Pass our oneWire reference to Dallas Temperature.
 
// By using direct addressing its possible to make sure that as you add temperature sensors
// the temperature sensor to variable mapping will not change.
// To find the addresses of your temperature sensors use the: **temperature_search sketch**
DeviceAddress address_T1 = { 0x28, 0x22, 0x70, 0xEE, 0x02, 0x00, 0x00, 0xB8 };
DeviceAddress address_T2 = { 0x28, 0x85, 0x7A, 0xEE, 0x02, 0x00, 0x00, 0xDC };
DeviceAddress address_T3 = { 0x28, 0x95, 0x51, 0xEE, 0x02, 0x00, 0x00, 0x0F };
DeviceAddress address_T4 = { 0x28, 0x95, 0x51, 0xEE, 0x02, 0x00, 0x00, 0x0F };

typedef struct {
  	  int realPower;	                                        // RealPower
    	  int apparentPower;	                                        // ApparentPower
  	  int T1;		                                        // temperature sensor 1
	  int T2;		                                        // temperature sensor 2
	  int T3;		                                        // temperature sensor 3
	  int T4;		                                        // temperature sensor 4
} Payload;
Payload emontx;

const int LEDpin = 9;

void setup() {
  Serial.begin(9600);
  Serial.println("emonTX Temperature power example"); 
  Serial.println("OpenEnergyMonitor.org");
  
  ct1.voltageTX(234.26, 1.7);                                           // Voltage: calibration, phase_shift
  ct1.currentTX(1, 111.1);                                              // Setup emonTX CT channel (channel, calibration)
                                                                        // CT Calibration factor = CT ratio / burden resistance
                                                                        // CT Calibration factor = (100A / 0.05A) x 18 Ohms
  sensors.begin();
  
  rf12_initialize(10, freq, 210);                                       // initialize RF
  rf12_sleep(RF12_SLEEP);
  
  pinMode(LEDpin, OUTPUT);   
  digitalWrite(LEDpin, HIGH);
  
  if (UNO) wdt_enable(WDTO_8S); 
}

void loop()
{ 
  sensors.requestTemperatures();                                        // Send the command to get temperatures
  
  emontx.T1 = sensors.getTempC(address_T1) * 100;
  emontx.T2 = sensors.getTempC(address_T2) * 100;
  emontx.T3 = sensors.getTempC(address_T3) * 100;
  emontx.T4 = sensors.getTempC(address_T4) * 100;
  
  ct1.calcVI(20,2000);                                                  // Calculate all. No.of crossings, time-out 
  emontx.realPower = ct1.realPower;
  emontx.apparentPower = ct1.apparentPower;
    
  print_to_serial();
 
  send_rf_data();                                                       // *SEND RF DATA* - see emontx_lib
  emontx_sleep(10);                                                     // sleep or delay in seconds - see emontx_lib
  digitalWrite(LEDpin, HIGH); delay(2); digitalWrite(LEDpin, LOW);      // flash LED
}

