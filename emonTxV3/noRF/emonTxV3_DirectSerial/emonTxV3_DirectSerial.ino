// EmonTx V3 Direct Serial example
// AC-AC adapter must be used with this example (highly recomneded for more accurate Real Power calculations)
// Useful for direct connection to Raspberry Pi or computer via USB to UART cable 
// Forum thread: http://openenergymonitor.org/emon/node/3872



// Tell emonLib this is the emonTx V3
// - don't read Vcc assume Vcc = 3.3V as is always the case on emonTx V3
// - eliminates bandgap error and need for calibration http://harizanov.com/2013/09/thoughts-on-avr-adc-accuracy/
#define emonTxV3

// Include Emon Library
#include "EmonLib.h" 

// Create four instances
EnergyMonitor ct1, ct2, ct3, ct4;

unsigned long lastpost = 0;
const byte LEDpin=                          6;                               // emonTx V3 LED
const int nodeID= 10;

void setup()
{    
  pinMode(LEDpin, OUTPUT); 
  digitalWrite(LEDpin,HIGH); 
  
  Serial.begin(9600);
  Serial.println("emonTx V3 Direct serial Example");
  
  // ADC pin, Vcal Calibration, phase_shift
  ct1.voltage(0, 268.97, 1.7);                   
  ct2.voltage(0, 268.97, 1.7);
  ct3.voltage(0, 268.97, 1.7);
  ct4.voltage(0, 268.97, 1.7);
  
// note:  UK/EU Vcal = 268.97; USA Vcal = 130.0; 
  
  // CT Current calibration 
  // (2000 turns / 22 Ohm burden resistor = 90.909)
  ct1.current(1, 90.9); 
  ct2.current(2, 90.9);
  ct3.current(3, 90.9);
  
  // CT 4 is high accuracy @ low power -  4.5kW Max 
  // (2000 turns / 120 Ohm burden resistor = 16.66)
  ct4.current(4, 16.6);
  
  lastpost = 0;
  
  delay(2000);
  digitalWrite(LEDpin,LOW); 
}

void loop()
{ 
  
  // A simple timer to fix the post rate to once every 10 seconds
  // Please dont post faster than once every 5 seconds to emoncms.org
  // Host your own local installation of emoncms for higher resolutions
  if ((millis()-lastpost)>=10000)
  {
    lastpost = millis();
    
    // .calcVI: Calculate all. No.of half wavelengths (crossings), time-out 
    ct1.calcVI(20,2000);
    ct2.calcVI(20,2000);
    ct3.calcVI(20,2000);
    ct4.calcVI(20,2000); 
    
    // Print to serial
    
    Serial.print(nodeID); Serial.print(' ');
    Serial.print(ct1.realPower); Serial.print(' '); 
    Serial.print(ct2.realPower); Serial.print(' '); 
    Serial.print(ct3.realPower); Serial.print(' '); 
    Serial.print(ct4.realPower); Serial.print(' '); 
    Serial.println(ct1.Vrms);
    
    digitalWrite(LEDpin,HIGH); 
    delay(200);
    digitalWrite(LEDpin,LOW); 
    
    // Note: the following measurements are also available:
    // - ct1.apparentPower
    // - ct1.Vrms
    // - ct1.Irms
  }
}
