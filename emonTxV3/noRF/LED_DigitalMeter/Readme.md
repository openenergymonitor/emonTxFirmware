By Robert Wall - May 2014

My DigitalMeterEmulator sketch, as originally released for use on an Arduino Uno or emonTx V2, has been really useful to me for checking the calibration status of any new hardware.  Having been recently been provided with an emonTx V3 for free, I though it may be helpful to upgrade this sketch for that platform.

Starting with the originally posted sketch, I've changed the voltage and sensor pin allocations to match my new hardware.  When running on one of my own 'Mk2' PCBs, the optimal powerCal value was found to be approx 0.42.

Sketch saved as archive/dev_1.ino

Now to try it on the emonTx V3.  I'll need to change the voltage sensor to 0 and the current sensor to 1 (for CT1) ... done.  I'll also need to change to LED output pin to ... 

const byte LedPin = 6;                          //emonTx V3 LED pin

as per 'my' Continuous Monitoring code after being upgraded for use on this platform, and change the output signal to be active high rather than active low.  With this all in place, it's working nicely at CT1, the optimal powerCal value being around 0.25.

Checking each input in turn to find the optimal powerCal value:

CT1: 0.25
CT2: 0.25
CT3: 0.25
CT4: 0.046

So these are the values that make my emonTx V3 match my digital meter.  Hopefully they will be similar to the calculated values that Glyn & Robert have derived for use in my "continuous Monitoring" sketch, i.e.

//const float powerCal_CT1 = (276.9*(3.3/1023))*(90.9*(3.3/1023)); // <---- powerCal value
//const float powerCal_CT2 = (276.9*(3.3/1023))*(90.9*(3.3/1023)); // <---- powerCal value
//const float powerCal_CT3 = (276.9*(3.3/1023))*(90.9*(3.3/1023)); // <---- powerCal value
//const float powerCal_CT4 = (276.9*(3.3/1023))*(16.6*(3.3/1023)); // <---- powerCal value (2000 / 120R burden resistor)

Using a simple sketch, as below, to evaluate the above numbers:

const float powerCal_CT1 = (276.9*(3.3/1023))*(90.9*(3.3/1023)); // <---- powerCal value
const float powerCal_CT4 = (276.9*(3.3/1023))*(16.6*(3.3/1023)); // <---- powerCal value (2000 / 120R burden resistor)

void setup()
{
delay(3000);
Serial.begin(9600);
Serial.println(powerCal_CT1);
Serial.println(powerCal_CT4);
}

void loop()
{
}

they turn out to be 0.2619 and 0.046.  So that's pretty good agreement.

calfactor 	calculated 	measured 	difference

powerCal_CT1	0.2619	0.25	-4.5%
powerCal_CT4	0.0478	0.046	- 3.8%

The best way forward is probably for me to copy the powerCal calculations from 'my' upgraded CM code into this code.  Then users can either rely on these values, or insert their own.  With this arrangement in place, the LED on the emonTx is flashing slightly faster than on my spare digital supply meter (an Ampy 5196), as expected.  But as a starting point, it's pretty good.  

If may be that my CT is not representative of latest product.  The one that I'm using used to contain a burden resistor, but I have since removed it.  With a brand new one that's recently been supplied by the OEM Shop ... the discrepancy appears to be very similar.  The optimal value for powerCal value CT on CT3 still appears to be close to 0.250.  With powerCal set to 0.0245, the V3's LED can be seen to be pulsing slightly more slowly than on my Ampy meter.

Tidy code, and save as archive/dev_2 ... done.

30/5/14
Tested against our main supply meter in the garage.  Seems to be working fine, albeit with its consumption pulses approx 5% faster than our official meter.  


