/*
 emonTX LowPower Temperature Example 
 
Example for using emonTx with two AA batteries connected to 3.3V rail. 
Voltage regulator must not be fitted
Jumper between PWR and Dig7 on JeePort 4

This setup allows the DS18B20 to be switched on/off by turning Dig7 HIGH/LOW
 
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
const int nodeID = 20;                                                  // emonTx temperature RFM12B node ID - should be unique on network
const int networkGroup = 210;                                           // emonTx RFM12B wireless network group - needs to be same as emonBase and emonGLCD
                                            
const int sensorResolution = 11;                                        //DS18B20 resolution 9,10,11 or 12bit corresponding to (0.5, 0.25, 0.125, 0.0625 degrees C LSB), lower resolution means lower power
const int time_between_readings=5000;                                  //in ms

#include <JeeLib.h>                                                     // Download JeeLib: http://github.com/jcw/jeelib
#include <avr/sleep.h>
ISR(WDT_vect) { Sleepy::watchdogEvent(); }                              // Attached JeeLib sleep function to Atmega328 watchdog -enables MCU to be put into sleep mode inbetween readings to reduce power consumption 

#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 4                                                  // Data wire is plugged into port 2 on the Arduino
OneWire oneWire(ONE_WIRE_BUS);                                          // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire);                                    // Pass our oneWire reference to Dallas Temperature.
DeviceAddress sensor;                                                   // arrays to hold device address

typedef struct {
  	  int temp;		                                      
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
  
  
  sensors.begin();
  if (!sensors.getAddress(sensor, 0)) Serial.println("Unable to find temperarture senosr");
  sensors.setResolution(sensor, sensorResolution);
  Serial.print("Temperature Sensor Resolution:");
  Serial.println(sensors.getResolution(sensor), DEC); 
  
  rf12_initialize(nodeID, freq, networkGroup);                          // initialize RFM12B
  rf12_control(0xC040);                                                 // set low-battery level to 2.2V i.s.o. 3.1V
  delay(10);
  rf12_sleep(RF12_SLEEP);
  
  
  pinMode(7,OUTPUT);                                                  //DS18B20 power control pin - see jumper setup instructions above

}

void loop()
{ 
  digitalWrite(7,HIGH);                                                 // turn on DS18B20
  delay(100);                                                           // wait for sensor to stable
  sensors.requestTemperatures();                                        // Send the command to get temperatures
  float temp=(sensors.getTempCByIndex(0));
  digitalWrite(7,LOW);                                                  //turn off DS18B20
  emontx.temp=(temp*100);
  emontx.battery=readVcc();
  Serial.print(emontx.temp); Serial.print(" "); Serial.println(emontx.battery);
  delay(100); 
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
  #if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328__)
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);  
  #elif defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif
  delay(2);					// Wait for Vref to settle
  ADCSRA |= _BV(ADSC);				// Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1125300L / result;			//1100mV*1023 ADC steps 
  return result;
}
