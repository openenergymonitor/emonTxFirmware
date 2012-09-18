/*
  Part 6 – Transmitting data via the RFM12
  
  Use the NanodeRF > Guide > a_FixedPackets guide example to test the receiving of the data packet
  sent from this example.
*/

#include <JeeLib.h>

// this is added as we're using the watchdog for low-power waiting
ISR(WDT_vect) { Sleepy::watchdogEvent(); }

#include "EmonLib.h"
EnergyMonitor ct1;

// Define the data structure of the packet to be transmitted 
typedef struct { int power, voltage; } PayloadTX;                              
PayloadTX emontx;                                                       

void setup() 
{
  ct1.currentTX(1, 111.1);

  // initialize RFM12B (node id, frequency, group)
  // Node 10 is an arbitrary choice (though all nodes must be unique on the system)   
  rf12_initialize(10,RF12_433MHZ,210); // NodeID, Frequency, Group

  rf12_sleep(RF12_SLEEP);
}

void loop() 
{ 
  emontx.power = ct1.calcIrms(1480) * 240.0;
  emontx.voltage = 240;

  rf12_sleep(RF12_WAKEUP);
  
  int i = 0; while (!rf12_canSend() && i<10) {rf12_recvDone(); i++;}
 
  rf12_sendStart(0, &emontx, sizeof emontx);
  // set the sync mode to 2 if the fuses are still the Arduino default
  // mode 3 (full powerdown) can only be used with 258 CK startup fuses
  rf12_sendWait(2);
  
  rf12_sleep(RF12_SLEEP);
  Sleepy::loseSomeTime(5000);
}
