// EmonTx example for monitoring a heatpump with direct serial connection to a raspberrypi

// Connected sensors:
// 4x DS18B20 temperature sensors
// 3x AC Real Power measurement

// Authors: Trystan Lea
// 12th March 2014

#include <avr/wdt.h>

// Include Emon Library
#include "EmonLib.h" 

// Create four instances
EnergyMonitor ct1, ct2, ct3, ct4;

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

void setup()
{
    Serial.begin(9600);
    Serial.println("Heatpump Monitor - direct RPi");
    
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
    
    digitalWrite(DS18B20_PWR, LOW);
    
    // Calibration, phase_shift
    ct1.voltage(0, 276.9, 1.7);                   
    ct2.voltage(0, 276.9, 1.7);
    ct3.voltage(0, 276.9, 1.7);
    ct4.voltage(0, 276.9, 1.7);
    
    // CT Current calibration 
    // (2000 turns / 22 Ohm burden resistor = 90.909)
    ct1.current(1, 90.9); 
    ct2.current(2, 90.9);
    ct3.current(3, 90.9);
    
    // CT 4 is high accuracy @ low power -  4.5kW Max 
    // (2000 turns / 120 Ohm burden resistor = 16.66)
    ct4.current(4, 16.6);

    wdt_enable(WDTO_8S);
}

void loop()
{
    if ((millis()-lastreading)>10000)
    {
        lastreading = millis();
        
        digitalWrite(DS18B20_PWR, HIGH); delay(50);
         
        sensors.requestTemperatures();
        delay(100);
         
        float temp1 = sensors.getTempC(allAddress[0]);
        float temp2 = sensors.getTempC(allAddress[1]);
        float temp3 = sensors.getTempC(allAddress[2]);
        float temp4 = sensors.getTempC(allAddress[3]); 
         
        digitalWrite(DS18B20_PWR, LOW);
        
        wdt_reset();
        
        // CalcVI: Calculate all. No.of half wavelengths (crossings), time-out 
        ct1.calcVI(20,2000);
        ct2.calcVI(20,2000);
        ct3.calcVI(20,2000);
        ct4.calcVI(20,2000);
        
        Serial.print(temp1); Serial.print(" ");
        Serial.print(temp2); Serial.print(" ");
        Serial.print(temp3); Serial.print(" ");
        Serial.print(temp4); Serial.print(" ");
      
        // Print to serial
        Serial.print(ct1.realPower); Serial.print(" "); 
        Serial.print(ct2.realPower); Serial.print(" ");
        Serial.print(ct3.realPower); Serial.print(" ");
        Serial.print(ct4.realPower); Serial.print(" "); 
        Serial.println(ct1.Vrms);
    }

    wdt_reset();
    delay(1000);
}

