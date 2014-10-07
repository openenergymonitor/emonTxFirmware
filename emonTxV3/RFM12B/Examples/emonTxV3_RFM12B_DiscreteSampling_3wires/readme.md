# 3-pĥase, 3-wire (without neutral) modification of the current V3 Discrete Sampling firmware

This is to use in conjunction with the modified emonLib2 files.
* It is used to measure 3-phase power with 2 current measurements (CT1 and CT2) and 1 voltage measurement (let's call it V13). The voltage is shifted by 120 degrees to create a virtual V23 (a virtual V23 is also created for debugging purposes).
* Emonlib2 has a modified CalcVI method that allows to time-shift the voltage waveform, similar to what was done in the 3-phase, 4-wire sketch
* The main loop in the .ino file calculates the power in the "first wire" with CT1 and V13 normally. Then it calls 3 times the power measurement for the "second wire", using CT2, the non time-shifted V, the version shifted by 120 degrees and the one shifted by 240 degrees)
* In emoncms, I then add power 1 and power2s (the 120 degree shifted version, with a minus sign) to get the total power of the system. 