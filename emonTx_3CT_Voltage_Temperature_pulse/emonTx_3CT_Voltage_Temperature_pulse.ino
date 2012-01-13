/*
                          _____    
                         |_   _|   
  ___ _ __ ___   ___  _ __ | |_  __
 / _ \ '_ ` _ \ / _ \| '_ \| \ \/ /
|  __/ | | | | | (_) | | | | |>  < 
 \___|_| |_| |_|\___/|_| |_\_/_/\_\
 
//--------------------------------------------------------------------------------------
// 2x CT + voltage measurement

// Based on JeeLabs RF12 library http://jeelabs.org/2009/02/10/rfm12b-library-for-arduino/

// Contributors: Glyn Hudson, Trystan Lea, Michele 
// openenergymonitor.org
// GNU GPL V3

//using CT port 2 - middle jackplug

//--------------------------------------------------------------------------------------
*/

#include "Emon.h"
EnergyMonitor emon1;
EnergyMonitor emon2;  
EnergyMonitor emon3;  

//JeeLabs libraries 
#include <Ports.h>
#include <RF12.h>
#include <avr/eeprom.h>
#include <util/crc16.h>  
#include <avr/wdt.h>

ISR(WDT_vect) { Sleepy::watchdogEvent(); } 	

//---------------------------------------------------------------------------------------------------
// Serial print settings - disable all serial prints if SERIAL 0 - increases long term stability 
//---------------------------------------------------------------------------------------------------
#define SERIAL 1
//---------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------
// RF12 settings 
//---------------------------------------------------------------------------------------------------
// fixed RF12 settings

#define myNodeID 10         //in the range 1-30
#define network     210      //default network group (can be in the range 1-250). All nodes required to communicate together must be on the same network group
#define freq RF12_433MHZ     //Frequency of RF12B module can be RF12_433MHZ, RF12_868MHZ or RF12_915MHZ. You should use the one matching the module you have.

// set the sync mode to 2 if the fuses are still the Arduino default
// mode 3 (full powerdown) can only be used with 258 CK startup fuses
#define RADIO_SYNC_MODE 2

#define COLLECT 0x20 // collect mode, i.e. pass incoming without sending acks
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// LED Indicator  
//--------------------------------------------------------------------------------------------------
# define LEDpin 9          //hardwired on emonTx PCB
//--------------------------------------------------------------------------------------------------

//########################################################################################################################
//Data Structure to be sent
//######################################################################################################################## 
typedef struct {
  	  double real1;		
          double apparent1;
	  double vrms1;
          double irms1;
          
          double real2;		
          double apparent2;
	  double vrms2;
          double irms2;
          
          double real3;		
          double apparent3;
	  double vrms3;
          double irms3;
          
          int supplyV;      
       
          double Livingroom, Hotwater, Outside;   
          
          int pulse, power;
} Payload;
Payload emontx;
//########################################################################################################################



#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 4

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);
int numberOfDevices; // Number of temperature devices found
// arrays to hold device address
DeviceAddress tempDeviceAddress;

uint8_t address[2];

double Livingroom, Hotwater, Outside;

int reset_temp = 1;

//---------------------------------------------------------------------------------------------------
// Pulse counting settings 
//---------------------------------------------------------------------------------------------------

long pulseCount = 0;               //Number of pulses, used to measure energy.
unsigned long pulseTime,lastTime;  //Used to measure power.
double power, elapsedWh;          //power and energy
int ppwh = 1;                    ////1000 pulses/kwh = 1 pulse per wh - Number of pulses per wh - found or set on the meter.
//---------------------------------------------------------------------------------------------------


//********************************************************************
//SETUP
//********************************************************************
void setup()
{  
  
  pinMode(LEDpin, OUTPUT);
  digitalWrite(LEDpin, HIGH);    //turn on LED 
  
  Serial.begin(9600);
  Serial.println("emonTx single CT example");
  Serial.println("openenergymonitor.org");
  
  rf12_initialize(myNodeID,freq,network); 
  rf12_sleep(RF12_SLEEP); 
  delay(10);
  Sleepy::loseSomeTime(3000);
  
  Serial.print("Node: "); 
  Serial.print(myNodeID); 
  Serial.print(" Freq: "); 
   if (freq == RF12_433MHZ) Serial.print("433Mhz");
   if (freq == RF12_868MHZ) Serial.print("868Mhz");
   if (freq == RF12_915MHZ) Serial.print("915Mhz");  
  Serial.print(" Network: "); 
  Serial.println(network);
  
  if (!SERIAL) {
    Serial.println("serial disabled"); 
    Serial.end();
  }
  
  digitalWrite(LEDpin, LOW);              //turn off LED
  
  attachInterrupt(1, onPulse, FALLING);    // KWH interrupt attached to IRQ 1  = pin3 - hardwired to emonTx pulse jackplug. For connections see: http://openenergymonitor.org/emon/node/208
  
  wdt_enable(WDTO_8S);
}

//********************************************************************
//LOOP
//********************************************************************
void loop()
{
  wdt_reset();
  
  if (reset_temp == 1)
  {
    sensors.begin();
    numberOfDevices = sensors.getDeviceCount();
    for(int i=0;i<numberOfDevices; i++)
    {
      if (sensors.getAddress(tempDeviceAddress, i)) sensors.setResolution(tempDeviceAddress, 12);
    }
    reset_temp =0;
  }
  
  sensors.requestTemperatures(); // Send the command to get temperatures
  
  for(int i=0;i<numberOfDevices; i++)
  {
    if(sensors.getAddress(tempDeviceAddress, i))
    {
      for (uint8_t p = 1; p < 3; p++)  // Shorten the 64-bit unique ID to 16-bits
      {
         address[p-1] = tempDeviceAddress[p];
         //Serial.print(address[0][p-1],HEX);    
      } 
      
      // 1) If Cylinder top sensor then get its temperature
      int a = 0;
      if (address[0] == 0x95) { a = 1; } else { a = 0; }
      if (address[1] == 0x51) { a = 1; } else { a = 0; }
      
      if (a == 1) 
      {
        Livingroom = sensors.getTempC(tempDeviceAddress);
        if (Livingroom == DEVICE_DISCONNECTED) {reset_temp = 1; delay(10000);}
      }
      
      // 2) If Cylinder bottom sensor then get its temperature
      a = 0;
      if (address[0] == 0x85) { a = 1; } else { a = 0; }
      if (address[1] == 0x7A) { a = 1; } else { a = 0; }
      if (a == 1) 
      {
        Hotwater = sensors.getTempC(tempDeviceAddress);
        if (Hotwater == DEVICE_DISCONNECTED) {reset_temp = 1; delay(10000);}
      }
      
      // 3) If Collector sensor then get its temperature
      a = 0;
      if (address[0] == 0x22) { a = 1; } else { a = 0; }
      if (address[1] == 0x70) { a = 1; } else { a = 0; }
      if (a == 1) 
      {
        Outside = sensors.getTempC(tempDeviceAddress);
        if (Outside == DEVICE_DISCONNECTED) {reset_temp = 1; delay(10000);}
      }
      
    }
  }
  delay(100);
  
  emontx.Livingroom = Livingroom*100;
  emontx.Hotwater = Hotwater*100;
  emontx.Outside = Outside*100;
  
  int vcc = readVcc();  //read emontx supply voltage
  
  //------------------------------------------------
  // MEASURE FROM CT 3
  //------------------------------------------------
  emon3.setPins(2,1); // CT 3
  emon3.calibration(281.6,126.5,0); 
  emon3.calc(20,2000,vcc );

  emontx.real3 = emon3.realPower;
  emontx.apparent3 = emon3.apparentPower;
  emontx.vrms3 = emon3.Vrms;
  emontx.irms3 = emon3.Irms;

  //------------------------------------------------
  // MEASURE FROM CT 2
  //------------------------------------------------
  emon2.setPins(2,0); // CT 2
  emon2.calibration(281.6,126.5,0); 
  emon2.calc(20,2000,vcc );
    
  emontx.real2 = emon2.realPower;
  emontx.apparent2 = emon2.apparentPower;
  emontx.vrms2 = emon2.Vrms;
  emontx.irms2 = emon2.Irms;

  //------------------------------------------------
  // MEASURE FROM CT 1
  //------------------------------------------------
  emon1.setPins(2,3); // CT 1
  emon1.calibration(281.6,126.5,0); 
  emon1.calc(20,2000,vcc );
    
  emontx.real1 = emon1.realPower;
  emontx.apparent1 = emon1.apparentPower;
  emontx.vrms1 = emon1.Vrms;
  emontx.irms1 = emon1.Irms;

  //------------------------------------------------
  emontx.supplyV = vcc;
  //------------------------------------------------
   
  emontx.pulse=pulseCount;        //as well as sending power we also send pulse increments inorder to accuratly calculate kwhr
  
    //--------------------------------------------------------------------------------------------------
    // 2. Send data via RF 
    //--------------------------------------------------------------------------------------------------
    rfwrite() ;
    //--------------------------------------------------------------------------------------------------    
    
  if (SERIAL) { serial_output();}

  digitalWrite(LEDpin, HIGH);    //flash LED - very quickly 
  delay(2); 
  digitalWrite(LEDpin, LOW); 
  
//sleep controll code  
//Uses JeeLabs power save function: enter low power mode and update Arduino millis 
//only be used with time ranges of 16..65000 milliseconds, and is not as accurate as when running normally.http://jeelabs.org/2010/10/18/tracking-time-in-your-sleep/

if ( (emontx.supplyV) > 3300 ) //if emonTx is powered by 5V usb power supply (going through 3.3V voltage reg) then don't go to sleep
  for (int i=0; i<10; i++){ delay(1000); wdt_reset();} //delay 10s
else
  if ( (emontx.supplyV) < 2700)  //if battery voltage drops below 2.7V then enter battery conservation mode (sleep for 60s in between readings) (need to fine tune this value) 
    Sleepy::loseSomeTime(60000);
    else
      Sleepy::loseSomeTime(1000); //10s


   pulseCount=0;       //reset pulse increments 
}
//********************************************************************

//--------------------------------------------------------------------------------------------------
// The interrupt routine - runs each time a falling edge of a pulse is detected
//--------------------------------------------------------------------------------------------------
void onPulse()                  
{

  lastTime = pulseTime;        //used to measure time between pulses.
  pulseTime = micros();

  pulseCount++;                                                      //pulseCounter               

  emontx.power = int((3600000000.0 / (pulseTime - lastTime))/ppwh);  //Calculate power
  
  //elapsedWh= (1.0*pulseCount/(ppwh));   // Find wh elapsed
  
}
//--------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------
// Send payload data via RF - see http://jeelabs.net/projects/cafe/wiki/RF12 for RF12 library documentation 
//--------------------------------------------------------------------------------------------------
static void rfwrite(){
    rf12_sleep(RF12_WAKEUP);  
    int ec = 0;
    while (!rf12_canSend()) {
      rf12_recvDone();
      ec++;
      if (ec>1000) break;
    }
    //rf12_sendStart(rf12_hdr, &emontx, sizeof emontx, RADIO_SYNC_MODE); - with hdr info 
    rf12_sendStart(0, &emontx, sizeof emontx); 
    rf12_sendWait(2);  
    rf12_sleep(RF12_SLEEP);   
}

//--------------------------------------------------------------------------------------------------
// Read current emonTx battery voltage - not main supplyV!
//--------------------------------------------------------------------------------------------------
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

//--------------------------------------------------------------------------------------------------
// Serial output
//--------------------------------------------------------------------------------------------------
void serial_output()
{
    Serial.print(emontx.real1);
    Serial.print(' ');
    Serial.print(emontx.apparent1);
    Serial.print(' ');
    Serial.print(emontx.vrms1);
    Serial.print(' ');
    Serial.print(emontx.irms1);
    Serial.print(" | ");
    
    Serial.print(emontx.real2);
    Serial.print(' ');
    Serial.print(emontx.apparent2);
    Serial.print(' ');
    Serial.print(emontx.vrms2);
    Serial.print(' ');
    Serial.print(emontx.irms2);
    Serial.print(" | ");
    
    Serial.print(emontx.real3);
    Serial.print(' ');
    Serial.print(emontx.apparent3);
    Serial.print(' ');
    Serial.print(emontx.vrms3);
    Serial.print(' ');
    Serial.print(emontx.irms3);
    Serial.print(' ');
    Serial.print(emontx.supplyV); 
    Serial.print(' ');
    
    Serial.print(emontx.Livingroom);
    Serial.print(' ');
    Serial.print(emontx.Hotwater);
    Serial.print(' ');
    Serial.print(emontx.Outside);
    Serial.print(' ');
    Serial.print(emontx.pulse);
    Serial.print(' ');
    Serial.println(emontx.power); 
}
