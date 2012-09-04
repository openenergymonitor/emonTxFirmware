/*
  Part 5 – Pulse counting
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
