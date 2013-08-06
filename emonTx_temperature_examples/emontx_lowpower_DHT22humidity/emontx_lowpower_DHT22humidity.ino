/*
  emonTX LowPower Temperature Example 
 
  Example for using emonTx with two AA batteries connected to 3.3V rail. 
  Voltage regulator must not be fitted
  Jumper between PWR and Dig7 on JeePort 4

  This setup allows the DHT22 to be switched on/off by turning Dig7 HIGH/LOW
 
  Part of the openenergymonitor.org project
  Licence: GNU GPL V3
 
  Authors: Glyn Hudson, Trystan Lea
  Builds upon JeeLabs RF12 library and Arduino

  THIS SKETCH REQUIRES:

  Libraries in the standard arduino libraries folder:
	- JeeLib		https://github.com/jcw/jeelib
        - DHT22 Humidity        https://github.com/adafruit/DHT-sensor-library - be sure to rename the sketch folder to remove the '-'


 
*/

#define freq RF12_433MHZ                                                // Frequency of RF12B module can be RF12_433MHZ, RF12_868MHZ or RF12_915MHZ. You should use the one matching the module you have.
const int nodeID = 21;                                                  // emonTx RFM12B node ID - should be unique on network, see recomended node ID range below
const int networkGroup = 210;                                           // emonTx RFM12B wireless network group - needs to be same as emonBase and emonGLCD

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
                                           
const int time_between_readings= 60000;                                  //60s in ms - FREQUENCY OF READINGS 

#include <JeeLib.h>                                                     // Download JeeLib: http://github.com/jcw/jeelib
#include <avr/sleep.h>
ISR(WDT_vect) { Sleepy::watchdogEvent(); }                              // Attached JeeLib sleep function to Atmega328 watchdog -enables MCU to be put into sleep mode inbetween readings to reduce power consumption 

// Humidity code adapted from ladyada' example
#include "DHT.h"
#define DHTPIN 4     // what pin we're connected to

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11 
#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Connect pin 1 (on the left) of the sensor to +5V
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 4.7-10K resistor from pin 2 (data) to pin 1 (power) of the sensor

DHT dht(DHTPIN, DHTTYPE);

typedef struct {
  	  int temp;
          int humidity;                                  
	  int battery;		                                      
} Payload;
Payload emontx;

void setup() {
  Serial.begin(9600);
  Serial.println("emonTX Low-power humidity example"); 
  Serial.println("OpenEnergyMonitor.org");
  Serial.print("Node: "); 
  Serial.print(nodeID); 
  Serial.print(" Freq: "); 
  if (freq == RF12_433MHZ) Serial.print("433Mhz");
  if (freq == RF12_868MHZ) Serial.print("868Mhz");
  if (freq == RF12_915MHZ) Serial.print("915Mhz"); 
  Serial.print(" Network: "); 
  Serial.println(networkGroup);
 
 
  pinMode(7,OUTPUT);                                                    // DHT22 power control pin - see jumper setup instructions above
  digitalWrite(7,HIGH);                                                 // turn on DHT22
  delay(2000);                                                          // wait 2s for DH22 to warm up
  dht.begin();

  rf12_initialize(nodeID, freq, networkGroup);                          // initialize RFM12B
  rf12_control(0xC040);                                                 // set low-battery level to 2.2V i.s.o. 3.1V
  delay(10);
  rf12_sleep(RF12_SLEEP);


}

void loop()
{ 
  
  digitalWrite(7,HIGH); delay(2000);                                     //wait 2s for sensor                                                                // Send the command to get temperatures
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  emontx.humidity = ((dht.readHumidity())*10);
  emontx.temp = ((dht.readTemperature())*10);
  Serial.println(emontx.humidity); 
  delay(5);
  //digitalWrite(7,LOW); 
  emontx.battery=readVcc();
  
   // check if returns are valid, if they are NaN (not a number) then something went wrong!
  if (isnan(emontx.temp) || isnan(emontx.humidity)) {
    Serial.println("Failed to read from DHT");}
    else
    {
      
      rf12_sleep(RF12_WAKEUP);
      // if ready to send + exit loop if it gets stuck as it seems too
      int i = 0; while (!rf12_canSend() && i<10) {rf12_recvDone(); i++;}
      rf12_sendStart(0, &emontx, sizeof emontx);
      // set the sync mode to 2 if the fuses are still the Arduino default
      // mode 3 (full powerdown) can only be used with 258 CK startup fuses
      rf12_sendWait(2);
      rf12_sleep(RF12_SLEEP); 
      delay(50);
    }
    
  Sleepy::loseSomeTime(time_between_readings);
}

long readVcc() {
  long result;
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2);
  ADCSRA |= _BV(ADSC);
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1126400L / result;
  return result;
}

