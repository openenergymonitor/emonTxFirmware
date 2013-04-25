/*
  emonTX LowPower Temperature Example 
 
  Example for using emonTx with two AA batteries connected to 3.3V rail. 
  Voltage regulator must not be fitted
  Jumper between PWR and Dig7 on JeePort 4

  WHEN IN OPERATION CURRENT CONSUMPTION @ 3.137V = 6.89mA (peak 7mA)
  WHEN SLEEPING                                    0.01mA or maybe less (limit of meter) 

  Temperature sensing takes: 782308 us
  RFM12 Sending takes 2920 us
  TOTAL = 0.785s @ 7mA = 0.017242822 J per pulse
  one every 10s = 0.001724282 W
  baseload = 0.00003137W (worst case)
  = 0.001755652 W
  3000 mah = 2x 10517J ~ 138 Days

  This setup allows the DS18B20 to be switched on/off by turning Dig7 HIGH/LOW
 
  Part of the openenergymonitor.org project
  Licence: GNU GPL V3
 
  Authors: Glyn Hudson, Trystan Lea
  Builds upon JeeLabs RF12 library and Arduino

  THIS SKETCH REQUIRES:

  Libraries in the standard arduino libraries folder:
	- JeeLib		https://github.com/jcw/jeelib
	- OneWire library	http://www.pjrc.com/teensy/td_libs_OneWire.html
	- DallasTemperature	http://download.milesburton.com/Arduino/MaximTemperature
        - DHT22 Humidity        https://github.com/adafruit/DHT-sensor-library - be sure to rename the sketch folder to remove the '-'
 
 
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

#define freq RF12_433MHZ                                                // Frequency of RF12B module can be RF12_433MHZ, RF12_868MHZ or RF12_915MHZ. You should use the one matching the module you have.
const int nodeID = 17;                                                  // emonTx temperature RFM12B node ID - should be unique on network
const int networkGroup = 210;                                           // emonTx RFM12B wireless network group - needs to be same as emonBase and emonGLCD
                                            
const int sensorResolution = 11;                                        //DS18B20 resolution 9,10,11 or 12bit corresponding to (0.5, 0.25, 0.125, 0.0625 degrees C LSB), lower resolution means lower power
const int time_between_readings= 5000;                                  //in ms

#include <JeeLib.h>                                                     // Download JeeLib: http://github.com/jcw/jeelib
#include <avr/sleep.h>
ISR(WDT_vect) { Sleepy::watchdogEvent(); }                              // Attached JeeLib sleep function to Atmega328 watchdog -enables MCU to be put into sleep mode inbetween readings to reduce power consumption 

#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 4                                                  // Data wire is plugged into port 2 on the Arduino
OneWire oneWire(ONE_WIRE_BUS);                                          // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire);                                    // Pass our oneWire reference to Dallas Temperature.
DeviceAddress sensor;  // arrays to hold device address

// Humidity code adapted from ladyada' example
#include "DHT.h"
#define DHTPIN 5     // what pin we're connected to

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11   // DHT 11 
#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Connect pin 1 (on the left) of the sensor to +5V
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

DHT dht(DHTPIN, DHTTYPE);

typedef struct {
  	  int temp1;
          int temp2;
          int humidity;                                  
	  int battery;		                                      
} Payload;
Payload emontx;

void setup() {
  Serial.begin(9600);
  Serial.println("emonTX Low-power temperature example"); 
  Serial.println("OpenEnergyMonitor.org");
  Serial.print("Node: "); 
  Serial.print(nodeID); 
  Serial.print(" Freq: "); 
  if (freq == RF12_433MHZ) Serial.print("433Mhz");
  if (freq == RF12_868MHZ) Serial.print("868Mhz");
  if (freq == RF12_915MHZ) Serial.print("915Mhz"); 
  Serial.print(" Network: "); 
  Serial.println(networkGroup);
 
  pinMode(7,OUTPUT);                                                    // DS18B20 power control pin - see jumper setup instructions above
  digitalWrite(7,HIGH);                                                 // turn on DS18B20
  delay(10);
  
  sensors.begin();
  if (!sensors.getAddress(sensor, 0)) Serial.println("Unable to find temperarture senosr");
  sensors.setResolution(sensor, sensorResolution);
  Serial.print("Temperature Sensor Resolution:");
  Serial.println(sensors.getResolution(sensor), DEC); 
  
  dht.begin();
  
  rf12_initialize(nodeID, freq, networkGroup);                          // initialize RFM12B
  rf12_control(0xC040);                                                 // set low-battery level to 2.2V i.s.o. 3.1V
  delay(10);
  rf12_sleep(RF12_SLEEP);


}

void loop()
{ 
  digitalWrite(7,HIGH); delay(2);                                       // turn on DS18B20
  sensors.requestTemperatures();                                        // Send the command to get temperatures
  emontx.temp1 = (sensors.getTempCByIndex(0))*100;
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  emontx.humidity = dht.readHumidity();
  emontx.temp2 = dht.readTemperature();
  digitalWrite(7,LOW); 
  emontx.battery=readVcc();
  rf12_sleep(RF12_WAKEUP);
  // if ready to send + exit loop if it gets stuck as it seems too
  int i = 0; while (!rf12_canSend() && i<10) {rf12_recvDone(); i++;}
  rf12_sendStart(0, &emontx, sizeof emontx);
  // set the sync mode to 2 if the fuses are still the Arduino default
  // mode 3 (full powerdown) can only be used with 258 CK startup fuses
  rf12_sendWait(2);
  rf12_sleep(RF12_SLEEP);  
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

