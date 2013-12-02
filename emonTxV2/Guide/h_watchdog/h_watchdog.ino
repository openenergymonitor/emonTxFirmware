/*
  Part 7 – Adding a watchdog for extra reliability
  
  The following sketch is a skeleton watchdog only sketch, it does not do much 
  at all, it is only to illustrate the commands needed and their location

  If wdt_reset() is not called in more than 8 seconds such as under a fault condition 
  the atmega will reset it self, otherwise if all is well the watchdog timer will 
  continually be reset and will never reach 8 seconds
  
*/
#include <avr/wdt.h>     // Include watchdog library                                                   

void setup() 
{ 
  wdt_enable(WDTO_8S);   // Enable watchdog: max 8 seconds
}

void loop() 
{ 
  wdt_reset();           // Reset watchdog
}
