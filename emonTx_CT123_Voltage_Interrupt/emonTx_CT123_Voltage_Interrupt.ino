/*

  Free running interrupt based version of EmonTxCT123_Voltage (no rf sending yet)

  Development version of new EmonLib, that uses the atmega's free running interrupt based ADC sampling mode.

  FURTHER DEVELOPMENT
    
  - turn ct1, ct2, ct3 on and off as needed
  - test how sample rate is affected by rf transmitts and ds18b20 temperature readings
  - record accumulated energy in eprom
  
  - Ongoing Dev Question: The Phase locked loop approach used by Martin Roberts looks like a good alternative approach to always
                          sample over an integer number of mains cycles it would be good to explore this further and understand 
                          advantages/disadvantages.
  
  Note: Channel calculations take between 28 and 56 microseconds - well below the maximum time of 104 microseconds which is the sampling period.
  
  //-----------------------------------------------------------------------------------------------------------------  
  
  Credit goes to the following people for developing and pushing forward this approach over the last year
  on which the code below is based and takes insight from:
  
  MartinR: http://openenergymonitor.org/emon/node/1535
  Robin Emley: http://openenergymonitor.org/emon/sites/default/files/Mk2i_PV_Router_rev5a.ino_.zip
  Pcunha: https://github.com/pcunha-lab/emonTxFirmware/tree/master/emonTx_Interrupts
  Jorg Becker: (builds on Pcunha's code) http://openenergymonitor.org/emon/sites/default/files/EmonTx_Interrupt_JB.zip

  Free-running sampling mode and analog input selection is based on code by Jorg Becker and Pcunha. 

  Thanks to Jorg Becker and Robert Wall for explaining and helping with the integer math based digital high pass filter.  
  
  Technique for adding each cycle to a total accumulator is based on Martin's code. This allows you to allways calculate measurements based
  on an integer number of mains cycles - and do the final calculation outside of the interrupt function in an asyncronous manner so as not
  to slow the interrupt function down.
  
  Uses Martin's conculsion for measuring supply voltage: that its better to specify it manually as internal bandgap is not accurate enough.
  Simplifies code significantly too and less needed as continuous sampling is designed for non-battery use and so voltage should be constant.

  The code is also based on the great AVR465.c example.

  Licence: GNU GPL V3
  Author: Trystan Lea
  
*/

int analog_inputs_pins[] = {2,0,1,3}; // Analog pins to sample and pin sampling order
int numberof_analog_inputs = sizeof(analog_inputs_pins) / 2;

int conversion_in_progress = 0;
int sample_in_register;
int next_conversion = 0;

unsigned long timer;

double realPower, apparentPower, powerFactor, Vrms, Irms,frequency;

//Calibration coeficients
//These need to be set in order to obtain accurate results
double VCAL = 275;
double ICAL = 133.3;
double PHASECAL= 1.7;
int supply_voltage = 3300;

double V_RATIO = VCAL *((supply_voltage/1000.0) / 1023.0);
double I_RATIO = ICAL *((supply_voltage/1000.0) / 1023.0);
        

int inPinV = 2;
signed int lastSampleV,sampleV;
signed long shifted_filterV;
float sumV, total_sumV;

int inPinI1 = 0;
signed int lastSampleI1,sampleI1;
signed long shifted_filterI1;
float sumI1,sumP1,total_sumI1,total_sumP1;

int inPinI2 = 1;
signed int lastSampleI2,sampleI2;
signed long shifted_filterI2;
float sumI2,sumP2,total_sumI2,total_sumP2;

int inPinI3 = 3;
signed int lastSampleI3,sampleI3;
signed long shifted_filterI3;
float sumI3,sumP3,total_sumI3,total_sumP3;

unsigned long numberOfSamples = 0, total_numberOfSamples;
unsigned int numberOfCycles = 0;

boolean last_cyclestate, cyclestate = false;
unsigned long last_cycle_time, cycle_period;
unsigned long total_cycle_period = 0;

static signed long filteredV;
static signed long filteredI1;
static signed long filteredI2;
static signed long filteredI3;

void setup()
{
  Serial.begin(9600);
  Serial.println("emonTx_CT123_Voltage_Interrupt");
  Serial.println("NO RF DATA BEING SENT YET"); 
  
  // REFS0 sets AVcc with external capacitor on AREF pin
  // CT1PIN sets the analog input pin to start reading from
  ADMUX = _BV(REFS0) | next_conversion;
  
  ADCSRA = _BV(ADATE) | _BV(ADIE);
  
  ADCSRA |= _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);

  ADCSRA |= _BV(ADEN) | _BV(ADSC);
  
  last_cycle_time = micros();
}

// ISR(ADC_vect) is the function called after each single channel ADC conversion is complete
// this function handle's cycling through all enabled analog inputs storing the result in the analog_input_values array
// The calc() function is called once all enabled analog inputs have been sampled.
ISR(ADC_vect)
{   
  signed long shiftedFCL;
  
  sample_in_register = conversion_in_progress;
  conversion_in_progress = analog_inputs_pins[next_conversion];
  
  if (sample_in_register == inPinV)
  {
    // VOLTAGE
    lastSampleV=sampleV;                                 // Used for digital high pass filter
    sampleV = ADC;                                       // Read in raw voltage signal
  
    // See documentation here for tutorial on digital filters:
    // http://openenergymonitor.org/emon/buildingblocks/digital-filters-for-offset-removal
    shiftedFCL = shifted_filterV + (long)((sampleV-lastSampleV)<<8);
    shifted_filterV = shiftedFCL - (shiftedFCL>>8);
    filteredV = (shifted_filterV+128)>>8;
      
    sumV += filteredV * filteredV;
  }
  
  if (sample_in_register == inPinI1)
  {
    // CT1
    lastSampleI1=sampleI1;
    sampleI1 = ADC;
    
    shiftedFCL = shifted_filterI1 + (long)((sampleI1-lastSampleI1)<<8);
    shifted_filterI1 = shiftedFCL - (shiftedFCL>>8);
    filteredI1 = (shifted_filterI1+128)>>8;
    
    sumI1 += filteredI1 * filteredI1;
    sumP1 += filteredV * filteredI1;
  }
  
  if (sample_in_register == inPinI2)
  {
    // CT2
    lastSampleI2=sampleI2;
    sampleI2 = ADC; 
    
    shiftedFCL = shifted_filterI2 + (long)((sampleI2-lastSampleI2)<<8);
    shifted_filterI2 = shiftedFCL - (shiftedFCL>>8);
    filteredI2 = (shifted_filterI2+128)>>8;
    
    sumI2 += filteredI2 * filteredI2;
    sumP2 += filteredV * filteredI2;
  }
  
  if (sample_in_register == inPinI3)
  {
    // CT3
    lastSampleI3=sampleI3;
    sampleI3 = ADC;
    
    shiftedFCL = shifted_filterI3 + (long)((sampleI3-lastSampleI3)<<8);
    shifted_filterI3 = shiftedFCL - (shiftedFCL>>8);
    filteredI3 = (shifted_filterI3+128)>>8;
    
    sumI3 += filteredI3 * filteredI3;
    sumP3 += filteredV * filteredI3;
  }
  
  // Set the adc channel to read from the sample after the current one already underway
  // the value of which will not be in the next ISR call but the one after.
  
  // cycle through analog inputs to the next input which is enabled
  
  boolean next_conversion_set=false;
  while (next_conversion_set==false)
  {
    next_conversion++;
    // If we've looped through all analog inputs then go back to the start
    if (next_conversion>(numberof_analog_inputs-1)) next_conversion = 0;
    ADMUX = _BV(REFS0) | analog_inputs_pins[next_conversion]; next_conversion_set = true;
    
    // If we're starting at input zero again then a whole set of inputs have been sampled, time to do calcs
    if (next_conversion==0) 
    {
      numberOfSamples++;                                   // Count number of times looped.

      last_cyclestate = cyclestate;
      if (filteredV>0) cyclestate = true; else cyclestate = false;
  
      if (last_cyclestate == 0 && cyclestate == 1) 
      {
        numberOfCycles ++;

        unsigned long cycle_time = micros();
        cycle_period = cycle_time - last_cycle_time;
        last_cycle_time = cycle_time;
    
        total_cycle_period += cycle_period; 

        total_numberOfSamples += numberOfSamples; numberOfSamples = 0;
        total_sumV += sumV; sumV = 0;

        total_sumI1 += sumI1; sumI1 = 0;
        total_sumP1 += sumP1; sumP1 = 0;

        total_sumI2 += sumI2; sumI2 = 0;
        total_sumP2 += sumP2; sumP2 = 0;
    
        total_sumI3 += sumI3; sumI3 = 0;
        total_sumP3 += sumP3; sumP3 = 0;
      }  
    }
  }
}

void loop()
{
  if ((millis()-timer)>5000)
  {
    timer = millis();
    
    //-------------------------------------------------------------------------------------------------------------------------
    // Complete measurement calculations
    //------------------------------------------------------------------------------------------------------------------------- 
    // Calculation of the root of the mean of the voltage and current squared (rms)
    // Calibration coeficients applied. 
   
    Vrms = V_RATIO * sqrt(total_sumV / total_numberOfSamples); 
    Irms = I_RATIO * sqrt(total_sumI1 / total_numberOfSamples); 

    //Calculation power values
    realPower = V_RATIO * I_RATIO * total_sumP1 / total_numberOfSamples;
    apparentPower = Vrms * Irms;
    powerFactor=realPower / apparentPower;
   
    frequency = 1.0/((total_cycle_period / numberOfCycles) /1000000.0);

    // Reset integer mains cycle accumulators
    numberOfCycles = 0;
    total_cycle_period = 0; 
        
    total_numberOfSamples = 0;
    total_sumV = 0;

    total_sumI1 = 0;
    total_sumP1 = 0;

    total_sumI2 = 0;
    total_sumP2 = 0;
    
    total_sumI3 = 0;
    total_sumP3 = 0;
    
    // Print output
    
    Serial.print(realPower,1);
    Serial.print("W ");
    Serial.print(apparentPower,1);
    Serial.print("VA ");
    Serial.print(Vrms);
    Serial.print("V ");
    Serial.print(Irms);
    Serial.print("A ");
    Serial.print(powerFactor);
    Serial.print(' ');
    Serial.print(frequency,3);
    Serial.println("Hz");
  }
}
