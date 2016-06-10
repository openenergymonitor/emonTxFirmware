# Change Log:

- v2.3   16/11/15 Change to unsigned long for pulse count and make default node ID 8 to avoid emonHub node decoder conflict & fix counting pulses faster than 110ms, strobed meter LED http://openenergymonitor.org/emon/node/11490 
- v2.2   12/11/15 Remove debug timming serial print code
- v2.1   24/10/15 Improved timing so that packets are sent just under 10s, reducing resulting data gaps in feeds + default status code for no temp sensors of 3000 which reduces corrupt packets improving data reliability
- V2.0   30/09/15 Update number of samples 1480 > 1662 to improve sampling accurancy: 1662 samples take 300 mS, which equates to 15 cycles @ 50 Hz or 18 cycles @ 60 Hz.
- V1.9   25/08/15 Fix spurious pulse readings from RJ45 port when DS18B20 but no pulse counter is connected (enable internal pull-up)
- V1.8 - 18/06/15 Increase max pulse width to 110ms
- V1.7 - 12/06/15 Fix pulse count debounce issue & enable pulse count pulse temperature
- V1.6 - Add support for multiple DS18B20 temperature sensors 
- V1.5 - Add interrupt pulse counting - simplify serial print debug 
- V1.4.1 - Remove filter settle routine as latest emonLib 19/01/15 does not require 
- V1.4 - Support for RFM69CW, DIP switches and battery voltage reading on emonTx V3.4
- V1.3 - fix filter settle time to eliminate large inital reading
- V1.2 - fix bug which caused Vrms to be returned as zero if CT1 was not connected 
- V1.1 - fix bug in startup Vrms calculation, startup Vrms startup calculation is now more accuratre

`emonhub.conf` node decoder (nodeid is 8 when switch is off, 7 when switch is on)
See: https://github.com/openenergymonitor/emonhub/blob/emon-pi/configuration.md
