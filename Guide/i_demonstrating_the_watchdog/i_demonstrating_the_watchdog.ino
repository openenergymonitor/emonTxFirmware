/*
  Part 8 - Demonstrating the watchdog
  
  The following sketch demonstrates the action of the watchdog

  If wdt_reset() is not called in more than 8 seconds such as under a fault condition 
  the atmega will reset it self, otherwise if all is well the watchdog timer will 
  continually be reset and will never reach 8 seconds.
  
  As downloaded, it prints "Watchdog demo - starts with Setup" once as setup( ) executes, 
  then each time loop( ) executes the numbers count down: 
		3
		2
		1
		About to reset
then the reset occurs and the loop( ) executes again. If the 'for' loop is changed to:

	for (int i=10; i>0; --i)

the the numbers will count down to 2, then the processor resets and runs setup( ) again and prints:

	Watchdog demo - starts with Setup
  
*/
#include <avr/wdt.h>     // Include watchdog library                                                   

void setup() 
{ 
  wdt_enable(WDTO_8S);   // Enable watchdog: max 8 seconds
  Serial.begin(9600);
  Serial.println("Watchdog demo - starts with Setup");
}

void loop() 
{ 
  for (int i=10; i>0; --i)
  {
	Serial.println(i);
	delay(1000);
  }
  Serial.println("About to reset");
  wdt_reset();           // Reset watchdog
}
