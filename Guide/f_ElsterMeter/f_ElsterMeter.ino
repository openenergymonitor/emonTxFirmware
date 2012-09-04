/*
  Part 6 - Elster Meter Reader
 
  If you have an Elster meter this is a fantastic way to read the exact accumulated watt hours 
  that you have generated or used and can compliment and cross check a CT based measurement.
 
  You will need to download the elster.h library developed by Dave Berkeley here:
  https://github.com/openenergymonitor/ElsterMeterReader
 
  Read more here:
  http://openenergymonitor.blogspot.co.uk/2012/08/reading-watt-hour-data-from-elster.html

  -----------------------------------------
  Part of the openenergymonitor.org project
  Licence: GNU GPL V3
*/

#include "elster.h"

void meter_reading(unsigned long r)
{
  Serial.print(r);
  Serial.print("\r\n");
}

ElsterA100C meter(meter_reading);

void setup()
{
  Serial.begin(9600);
  Serial.println("IRDA Meter reader");
  meter.init(1);
}

void loop()
{
  // Decode the meter stream
  const int byte_data = meter.decode_bit_stream();
  if (byte_data != -1) {
    meter.on_data(byte_data);
  }
}
