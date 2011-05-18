  //Sample variables
int lastSampleI,sampleI;

//Filter variables
double lastFilteredI,filteredI;
//Power calculation variables
double sqI,sumI; 
 
double emon(int inPinI,double ICAL,int Vrms,int numberOfSamples,int CT_BURDEN_RESISTOR,int CT_TURNS,int supplyV) { 
 for (int n=0; n<numberOfSamples; n++)
          {
             lastSampleI=sampleI;
             sampleI = analogRead(inPinI);
             lastFilteredI = filteredI;
             filteredI = 0.996*(lastFilteredI+sampleI-lastSampleI);

             //Root-mean-square method current
             //1) square current values
             sqI = filteredI * filteredI;
             //2) sum 
             sumI += sqI;
          }
          
          //Calculation of the root of the mean of the voltage and current squared (rms)
          //Calibration coeficients applied. 
          
          double I_RATIO = ((1500/56.0) * supplyV / 1024000) * ICAL;

          double Irms = I_RATIO * sqrt(sumI / numberOfSamples); 
          
          //Calculation power values
          double apparentPower = Vrms * Irms;

          sumI = 0;
          
          return apparentPower;
}
