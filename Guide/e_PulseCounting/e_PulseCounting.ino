/*
  Part 5 – Pulse counting
  
  Many meters have pulse outputs, including electricity meters: single phase, 3-phase, 
  import, export.. Gas meters, Water flow meters etc

  The pulse output may be a flashing LED or a switching relay (usually solid state) or both.

  In the case of an electricity meter a pulse output corresponds to a certain amount of 
  energy passing through the meter (Kwhr/Wh). For single-phase domestic electricity meters
  (eg. Elster A100c) each pulse usually corresponds to 1 Wh (1000 pulses per kwh).  

  The code below detects the falling edge of each pulse and increment pulseCount
  
  It calculated the power by the calculating the time elapsed between pulses.
  
  Read more about pulse counting here:
  http://openenergymonitor.org/emon/buildingblocks/introduction-to-pulse-counting

  -----------------------------------------
  Part of the openenergymonitor.org project
  Licence: GNU GPL V3
*/

long pulseCount = 0;
unsigned long pulseTime,lastTime; // Used to measure time between pulses
double power;
int ppwh = 1;                     // pulses per watt hour - found or set on the meter.

void setup() 
{
  Serial.begin(9600);
  
  // pulse detection interrupt (emontx pulse channel - IRQ0 D3)
  attachInterrupt(1, onPulse, FALLING);
}

void loop() 
{ 
  Serial.print(power);
  Serial.print(' ');
  Serial.println(pulseCount * ppwh);  // watt hour elapsed
  delay(1000);
}

// The interrupt routine - runs each time a falling edge of a pulse is detected
void onPulse()                  
{
  lastTime = pulseTime;
  pulseTime = micros();
  pulseCount++;                                               // count pulse               
  power = int((3600000000.0 / (pulseTime - lastTime))/ppwh);  // calculate power
}
