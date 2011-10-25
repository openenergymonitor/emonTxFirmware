int lastSampleI,sampleI;         // Sample variables
double lastFilteredI =0 ,filteredI = 0;  // Filter variables
double sqI = 0,sumI = 0;                 // Power calculation variables
 
double emon(int CT_INPUT_PIN,double ICAL,int RMS_VOLTAGE,int NUMBER_OF_SAMPLES,int CT_BURDEN_RESISTOR,int CT_TURNS,int SUPPLY_VOLTAGE) 
{ 
  sampleI = analogRead( CT_INPUT_PIN );
  for (int n = 0; n < NUMBER_OF_SAMPLES; n++)
  {
    lastSampleI = sampleI;
    sampleI = analogRead( CT_INPUT_PIN );
    lastFilteredI = filteredI;
    filteredI = 0.996*(lastFilteredI+sampleI-lastSampleI);

    // Root-mean-square method current
    // 1) square current values
    sqI = filteredI * filteredI;
    // 2) sum 
    sumI += sqI;
  }
  double I_RATIO = (( CT_TURNS / CT_BURDEN_RESISTOR ) * (SUPPLY_VOLTAGE/1000.0)) / 1024.0;    // Rough calibration by component values
  I_RATIO = I_RATIO * ICAL;                                                                   // Fine adjustment calibration for accuracy
  
  double Irms = I_RATIO * sqrt(sumI / NUMBER_OF_SAMPLES);                                     // Final step in calculation of RMS CURRENT
  sumI = 0;                                                                                   // Reset sum ready for next run.
  
  double apparentPower = RMS_VOLTAGE * Irms;                                                  // Apparent power calculation
  return apparentPower;
}
