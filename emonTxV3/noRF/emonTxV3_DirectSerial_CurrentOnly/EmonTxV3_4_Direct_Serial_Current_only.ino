// EmonTx V3 Current Only Direct Serial example
// Useful for direct connection to Raspberry Pi or computer via USB to UART cable 

// Entry in emonHub for this sketch:
/*

[[10]]
    nodename = emontx
    [[[rx]]]
       names = current1, current2, current3, current4
       datacode = 0
       scales = 1,1,1,1
       units = A,A,A,A

alternatively

[[10]]
    nodename = emontx
    [[[rx]]]
       names = apparentPower1, apparentPower2, apparentPower3, apparentPower4
       datacode = 0
       scales = 1,1,1,1
       units = VA,VA,VA,VA

*/

// Tell emonLib this is the emonTx V3
// - don't read Vcc assume Vcc = 3.3V as is always the case on emonTx V3
// - eliminates bandgap error and need for calibration http://harizanov.com/2013/09/thoughts-on-avr-adc-accuracy/
#define emonTxV3

// Include Emon Library
#include "EmonLib.h" 

// Create four instances
EnergyMonitor ct1, ct2, ct3, ct4;

unsigned long lastpost = 0;
const byte LEDpin= 6;             // emonTx V3 LED  - Change this to LEDpin = 9 for the emonTx Shield
const int nodeID= 10;
const int num_samples = 1118;     // calcIrms samples at approx 5588 per second. num_samples should be close to a whole number of cycles
                                  //  1118 will give a sample averaging period of approx 200 ms. = 10 cycles @ 50 Hz or 12 cycles @ 60 Hz
const double voltage = 230.0;     // Set to your system voltage to calculate apparent power.

void setup()
{    
  pinMode(LEDpin, OUTPUT); 
  digitalWrite(LEDpin,HIGH); 
  
  Serial.begin(9600);
  Serial.println("emonTx V3 Current Only Direct serial Example");
  
 
  // CT Current calibration 
  // (2000 turns / 22 Ohm burden resistor = 90.909)
  ct1.current(1, 90.9); 
  ct2.current(2, 90.9);
  ct3.current(3, 90.9);
  
  // CT 4 is high accuracy @ low power -  4.5kW Max (emonTx only) 
  // (2000 turns / 120 Ohm burden resistor = 16.66)
  // Use 90.9 for the emonTx Shield
  ct4.current(4, 16.6);                 
  
  lastpost = 0;
  
  delay(2000);
  digitalWrite(LEDpin,LOW); 
}

void loop()
{ 
  
  // A simple timer to fix the post rate to once every 10 seconds
  // Please don't post faster than once every 5 seconds to emoncms.org
  // Host your own local installation of emoncms for higher resolutions
  if ((millis()-lastpost)>=10000)
  {
    lastpost = millis();
        
    // Print to serial
    // To have the sketch calculate apparent power (VA), change 
    //   ct1.calcIrms(num_samples) 
    // to 
    //   ct1.calcIrms(num_samples) * voltage
    
    Serial.print(nodeID); Serial.print(' ');
    Serial.print(ct1.calcIrms(num_samples)); Serial.print(' '); 
    Serial.print(ct2.calcIrms(num_samples)); Serial.print(' '); 
    Serial.print(ct3.calcIrms(num_samples)); Serial.print(' '); 
    Serial.println(ct4.calcIrms(num_samples));
    
    digitalWrite(LEDpin,HIGH); 
    delay(200);
    digitalWrite(LEDpin,LOW); 
    
  }
}
