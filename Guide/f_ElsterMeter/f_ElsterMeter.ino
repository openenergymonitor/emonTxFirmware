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
