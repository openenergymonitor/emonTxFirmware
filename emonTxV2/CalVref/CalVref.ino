 /*
 CalVref
 
 A test sketch to allow the internal 1.1 V reference to be accurately calibrated.
 
 The internal reference in the AVR is nominally 1.1 V, within the range 1.0 - 1.2 V. This represents more than 9% uncertainty that is
 reflected in the measurement of Vcc in the standard emonLib library method. This directly affects the battery voltage measurement
 when the emonTx is battery powered, it also is reflected in the calibration of the voltage and current sensors.
 
 If the constant is changed in readVcc( ) , the voltage and current sensors must be recalibrated. 
 
 The theoretical constant, i.e. 1.1 V  in mV times 1024  is 1126400
 The normal range should be 1024000  -  1228800
 
 Part of the openenergymonitor.org project
 Licence: GNU GPL V3
 
 Authors: Glyn Hudson, Trystan Lea
 Builds upon JeeLabs RF12 library and Arduino
 Credit to http://hacking.majenko.co.uk/making-accurate-adc-readings-on-arduino
 and J�r�me who alerted us to http://provideyourown.com/2012/secret-arduino-voltmeter-measure-battery-voltage/
 Adapted from the above by Robert Wall.
 
 (And J�r�me's name generates a "unrecognized characters" compiler warning - ignore it.)
*/



#if defined(ARDUINO) && ARDUINO >= 100

#include "Arduino.h"

#else

#include "WProgram.h"

#endif

bool done = false;

void setup() 
{
	Serial.begin(9600);
	for (int i=0; i<7; i++,delay(1000))
		Serial.print(".");
	Serial.println("\nemonTX Internal Voltage Calibration"); 
	Serial.println("OpenEnergyMonitor.org");

    Serial.println("\nInstructions:\nMeasure the 3.3 V supply with your multimeter at the + and - \"Batt\" pads and enter now the voltage you measure.\n(You can repeat this as often as you wish.)");


}

void loop() 
{ 
	char inbuf[10];
	char nChar;
	float Vs;
	long Vref;
	long RefConstant;
	
	done = false;
	while (!done) {
		nChar = Serial.available();
		if (nChar > 0) 
		{
			done = true; 
			for (int i = 0; i < 10; i++) 
			{
				inbuf[i] = 0;
			}
			Serial.readBytes(inbuf, nChar); 
			Vs = atof(inbuf);
			Serial.print("You measured ");
			Serial.print(Vs);
			Vref = readVcc();
			Serial.print(" V.\nThat means your internal reference is ");
			Serial.print(Vref);
			Serial.print(" counts or ");
			Serial.print(Vs / 1024.0 * Vref,3);
			Serial.print(" V if measured with your meter.");
			RefConstant = (long)(Vs * Vref * 1000);
			Serial.print("\nThe constant for that is ");
			Serial.print(RefConstant);
			Serial.print("L\nUsing that in \"readVcc( )\", it will return ");
			Serial.print(RefConstant / Vref);
			Serial.print(" mV.");
			if (RefConstant / Vref != Vs * 1000)
				Serial.print(" The difference is due to integer rounding.");
			
			if (RefConstant > 1228800L || RefConstant < 1024000)
				Serial.print("\nThe normal range should be 1024000L - 1228800L");
			Serial.println("\n");
		}
		
		else
			delay(100);
	}
		

}


long readVcc(void) {
// Not quite the same as the emonLib version
  long result;

  #if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328__) || defined (__AVR_ATmega328P__)
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);  
  #elif defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  ADCSRB &= ~_BV(MUX5);   // Without this the function always returns -1 on the ATmega2560 http://openenergymonitor.org/emon/node/2253#comment-11432
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
  ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
  ADMUX = _BV(MUX3) | _BV(MUX2);
  #endif

  delay(2);					// Wait for Vref to settle
  ADCSRA |= _BV(ADSC);				// Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  // return the raw count  - that's more useful here
  return result;
}
