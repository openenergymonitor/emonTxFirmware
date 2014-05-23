/*

  EmonTx v3 example for monitoring a heatpump with RFM12 wireless tx to basestation

   -----------------------------------------
  Part of the openenergymonitor.org project
  
  Authors: Trystan Lea 
  Builds upon JCW JeeLabs RF12 library and Arduino 
  
  Licence: GNU GPL V3
  
  Connected sensors:
  
  4x DS18B20 temperature sensors
  3x AC Real Power measurement

  Authors: Trystan Lea
  23th May 2014
  
*/

// Frequency of RF12B module can be RF12_433MHZ, RF12_868MHZ or RF12_915MHZ. 
// You should use the one matching the module you have.
#define RF_freq RF12_433MHZ
// Node id of this emontx v3 heatpump monitoring node
const int nodeID = 10;
// RFM12 Network group to use:
const int networkGroup = 210;  

#include <avr/wdt.h>

// Tell emonLib this is the emonTx V3 - don't read Vcc assume Vcc = 3.3V as is always the case on emonTx V3 
// eliminates bandgap error and need for calibration http://harizanov.com/2013/09/thoughts-on-avr-adc-accuracy/
#define emonTxV3 

// Special modified version of the JeeJib library to work with the RFu328 https://github.com/openenergymonitor/RFu_jeelib        
#include <RFu_JeeLib.h>

// Attached JeeLib sleep function to Atmega328 watchdog -enables MCU to be put into sleep mode inbetween readings to reduce power consumption 
ISR(WDT_vect) { Sleepy::watchdogEvent(); }                            

// Include Emon Library
#include "EmonLib.h" 

// Although the EmonTx can measure power on up to 4 CT's
// This heatpump monitor example only needs one power measurement for electrical power in to the heatpump
// If your application needs another CT its easy to add one: see other emontx v3 examples for reference

EnergyMonitor ct;

#include <OneWire.h>
#include <DallasTemperature.h>

// 9 (93.8ms),10 (187.5ms) ,11 (375ms) or 12 (750ms) bits equal to resplution of 0.5C, 0.25C, 0.125C and 0.0625C
const int TEMPERATURE_PRECISION = 12;

const byte DS18B20_PWR = 19;
const byte ONE_WIRE_BUS = 5;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

int numSensors;
byte allAddress [4][8];

unsigned long lastreading;

long wh = 0;
int joules = 0;

long wtdrtime = 0;

// Create struct for rf data packet
typedef struct { int power, wh, T1, T2, T3, T4; } PayloadTX;     
PayloadTX emontx;

boolean debug = 0;

void setup()
{
    Serial.begin(9600);
    Serial.println("Heatpump Monitor RF");
    if (Serial) debug = 1; else debug=0; 
    
    rf12_initialize(nodeID, RF_freq, networkGroup);
    rf12_sleep(RF12_SLEEP);
    delay(100);
    
    pinMode(DS18B20_PWR, OUTPUT);
    digitalWrite(DS18B20_PWR, HIGH); delay(50);
    
    sensors.begin();
    
    // disable automatic temperature conversion to reduce time spent awake, conversion will be implemented manually in sleeping http://harizanov.com/2013/07/optimizing-ds18b20-code-for-low-power-applications/ 
    sensors.setWaitForConversion(false);             
    numSensors=(sensors.getDeviceCount());
    for(int j=0;j<numSensors;j++) {
        oneWire.search(allAddress[j]);
        sensors.setResolution(allAddress[j], TEMPERATURE_PRECISION);
    }
    
    // Calibration, phase_shift
    ct.voltage(0, 276.9, 1.7);
    
    // CT Current calibration 
    // (2000 turns / 22 Ohm burden resistor = 90.909)
    // ct.current(1, 90.9);
    // CT 4 is high accuracy @ low power -  4.5kW Max 
    // (2000 turns / 120 Ohm burden resistor = 16.66)
    ct.current(4, 16.6);

    joules = 0;

    wdt_enable(WDTO_8S);
    
    // reduces settle time
    ct.calcVI(20,2000);
}

void loop()
{
    if ((millis()-lastreading)>=5000)
    {
        lastreading = millis();
         
        sensors.requestTemperatures();
        delay(100);
         
        float T1 = sensors.getTempC(allAddress[0]);
        float T2 = sensors.getTempC(allAddress[1]);
        float T3 = sensors.getTempC(allAddress[2]);
        float T4 = sensors.getTempC(allAddress[3]);
        
        wdt_reset();
        
        // CalcVI: Calculate all. No.of half wavelengths (crossings), time-out 
        ct.calcVI(20,2000);
            
        joules += ct.realPower * 10;
        wh += joules / 3600;
        joules = joules % 3600;
        
        // Print to serial
        if (debug) 
        {
          Serial.print(ct.realPower); Serial.print(" ");
          Serial.print(wh); Serial.print(" ");
          Serial.print(ct.Vrms); Serial.print(" ");
          
          Serial.print(T1); Serial.print(" ");
          Serial.print(T2); Serial.print(" ");
          Serial.print(T3); Serial.print(" ");
          Serial.print(T4); Serial.println();
          
          delay(100);
        }
        
        emontx.power = ct.realPower;
        emontx.wh = wh;
        
        emontx.T1 = T1 * 100;
        emontx.T2 = T2 * 100;
        emontx.T3 = T3 * 100;
        emontx.T4 = T4 * 100;
        
        
        rf12_sleep(RF12_WAKEUP);                                   
        rf12_sendNow(0, &emontx, sizeof emontx);                           //send temperature data via RFM12B using new rf12_sendNow wrapper
        rf12_sendWait(2);
        rf12_sleep(RF12_SLEEP);
    }

    if ((millis()-wtdrtime)>1000)
    {
       wtdrtime = millis();
       wdt_reset();
    }
}

