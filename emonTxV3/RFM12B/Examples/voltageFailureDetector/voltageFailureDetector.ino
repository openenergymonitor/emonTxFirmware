// A test pad for the development of ACV failure logic.
//
// The value of each filtered voltage sample is placed into one of 3 categories
// (enum voltageZones).  Whenever the voltage departs from the central zone, 
// which shows that the AC is still active) the time is noted.  
//    
// The time 'now'is checked every loop and compared with the time of the last 
// recorded departure.  If their difference ever exceeds the #defined value 
// a few lines down, AC failure is declared.  
//
//   With 10mS, the LED flickers, as one might expect;
//   With 11 mS, it seems stable enough;
//   With 12 mS, I have little doubt that it will work reliably
//
//  With a tightly set level, there's always the chance of a false alarm.
//  This risk will need to be balanced against the reduced period that would 
//  be available for emergengy housekeeping measures in the event of a real 
//  power failure if a safer i.e. 'longer' persistence period were to be used.
//
//  All non-essential code has been stripped out.  Two current samples and 
//  one voltage sample are taken each loop, but only the voltage samples 
//  are actually processed.
//
//  RAE 31/1/14

#include <Arduino.h> // may not be needed, but it's probably a good idea to include this

#include <TimerOne.h>
#define ADC_TIMER_PERIOD 125 // uS
#define MAX_INTERVAL_BETWEEN_CONSECUTIVE_PEAKS 12 // mS

// definition of enumerated types
enum polarities {NEGATIVE, POSITIVE};
enum LEDstates {LED_OFF, LED_ON};   
enum voltageZones {NEGATIVE_ZONE, MIDDLE_ZONE, POSITIVE_ZONE};

const byte LEDpin = 9;   
const byte voltageSensor = 2;          // A3 is for the voltage sensor
const byte currentSensor_diverted = 4; // A4 is for CT2 which measures diverted current
const byte currentSensor_grid = 5;     // A5 is for CT1 which measures grid current

const byte startUpPeriod = 3;  // in seconds, to allow LP filter to settle
const int DCoffset_I = 512;    // nominal mid-point value of ADC @ x1 scale

boolean beyondStartUpPhase = false;     // start-up delay, allows things to settle
unsigned long startTime = 0;       // is non-zero when in Tallymode 
long cycleCount = 0;                    // counts mains cycles from start-up 
long DCoffset_V_long;              // <--- for LPF
long DCoffset_V_min;               // <--- for LPF
long DCoffset_V_max;               // <--- for LPF

// for interaction between the main processor and the ISRs 
volatile boolean dataReady = false;
int sampleI_grid;
int sampleI_diverted;
int sampleV;

// for voltage failure detection logic
int voltageThresholdOffset = 250; // in ADC steps from mid-point
long voltageThesholdUpper_long;  // determined once in setup()
long voltageThesholdLower_long;  // determined once in setup()
unsigned long timeOfLastPeakEntry; // would be better as a static in the processing function
enum voltageZones voltageZoneOfLastSampleV; // would be better as a static in the processing function
enum voltageZones lastPeakZoneVisited; // would be better as a static in the processing function


void setup()
{  
  Serial.begin(9600);
  Serial.println();
  
  delay(5000); // allow time to open Serial monitor     
 
  Serial.println();
  Serial.println();
  Serial.println("-------------------------------------");
  Serial.println("Sketch ID:      AC failure detector");
       
  pinMode(LEDpin, OUTPUT); 
  digitalWrite(LEDpin, LED_OFF); 
         
  // Define operating limits for the LP filter which identifies DC offset in the voltage 
  // sample stream.  By limiting the output range, the filter always should start up 
  // correctly.
  DCoffset_V_long = 512L * 256; // nominal mid-point value of ADC @ x256 scale  
  DCoffset_V_min = (long)(512L - 100) * 256; // mid-point of ADC minus a working margin
  DCoffset_V_max = (long)(512L + 100) * 256; // mid-point of ADC plus a working margin

  Serial.print ("ADC mode:       ");
  Serial.print (ADC_TIMER_PERIOD);
  Serial.println ( "uS fixed timer");

  // Set up the ADC to be triggered by a hardware timer of fixed duration  
  ADCSRA  = (1<<ADPS0)+(1<<ADPS1)+(1<<ADPS2);  // Set the ADC's clock to system clock / 128
  ADCSRA |= (1 << ADEN);                 // Enable ADC

  Timer1.initialize(ADC_TIMER_PERIOD);   // set Timer1 interval
  Timer1.attachInterrupt( timerIsr );    // declare timerIsr() as interrupt service routine

  Serial.print(">>free RAM = ");
  Serial.println(freeRam());  // a useful value to keep an eye on

  Serial.println ("----");    

  voltageThesholdUpper_long = (long)voltageThresholdOffset << 8;
  voltageThesholdLower_long = -1 * voltageThesholdUpper_long;
  Serial.println("voltage thresholds long:");
  Serial.print("  upper: ");
  Serial.println(voltageThesholdUpper_long);
  Serial.print("  lower: ");
  Serial.println(voltageThesholdLower_long);
  
}

// An Interrupt Service Routine is now defined in which the ADC is instructed to 
// measure V and I alternately.  A "data ready"flag is set after each voltage conversion 
// has been completed.  
//   For each pair of samples, this means that current is measured before voltage.  The 
// current sample is taken first because the phase of the waveform for current is generally 
// slightly advanced relative to the waveform for voltage.  The data ready flag is cleared 
// within loop().
//   This Interrupt Service Routine is for use when the ADC is fixed timer mode.  It is 
// executed whenever the ADC timer expires.  In this mode, the next ADC conversion is 
// initiated from within this ISR.  
//
void timerIsr(void)
{                                         
  static unsigned char sample_index = 0;

  switch(sample_index)
  {
    case 0:
      sampleV = ADC;                    // store the ADC value (this one is for Voltage)
      ADMUX = 0x40 + currentSensor_diverted;  // set up the next conversion, which is for Diverted Current
      ADCSRA |= (1<<ADSC);              // start the ADC
      sample_index++;                   // increment the control flag
      dataReady = true;                 // all three ADC values can now be processed
      break;
    case 1:
      sampleI_diverted = ADC;               // store the ADC value (this one is for Diverted Current)
      ADMUX = 0x40 + currentSensor_grid;  // set up the next conversion, which is for Grid Current
      ADCSRA |= (1<<ADSC);              // start the ADC
      sample_index++;                   // increment the control flag
      break;
    case 2:
      sampleI_grid = ADC;               // store the ADC value (this one is for Grid Current)
      ADMUX = 0x40 + voltageSensor;  // set up the next conversion, which is for Voltage
      ADCSRA |= (1<<ADSC);              // start the ADC
      sample_index = 0;                 // reset the control flag
      break;
    default:
      sample_index = 0;                 // to prevent lockup (should never get here)      
  }
}


// When using interrupt-based logic, the main processor waits in loop() until the 
// dataReady flag has been set by the ADC.  Once this flag has been set, the main
// processor clears the flag and proceeds with all the processing for one set of 
// V & I samples.  It then returns to loop() to wait for the next set to become 
// available.
//   If the next set of samples become available before the processing of the 
// previous set has been completed, data could be lost.  This situation can be 
// avoided by prior use of the WORKLOAD_CHECK mode.  Using this facility, the amount
// of spare processing capacity per loop can be determined.  
//   If there is insufficient processing capacity to do all that is required, the 
// base workload can be reduced by increasing the duration of ADC_TIMER_PERIOD.
//
void loop()             
{ 
  if (dataReady)   // flag is set after every pair of ADC conversions
  {
    dataReady = false; // reset the flag
    allGeneralProcessing(); // executed once for each pair of V&I samples   
  }  // <-- this closing brace needs to be outside the WORKLOAD_CHECK blocks!  
} // end of loop()


// This routine is called to process each pair of V & I samples.  Note that when using 
// interrupt-based code, it is not necessary to delay the processing of each pair of 
// samples as was done in Mk2a builds.  This is because there is no longer a strict 
// alignment between the obtaining of each sample by the ADC and the processing that can 
// be done by the main processor while the ADC conversion is in progress.  
//   When interrupts are used, the main processor and the ADC work autonomously, their
// operation being only linked via the dataReady flag.  As soon as data is made available
// by the ADC, the main processor can start to work on it immediately.  
//
void allGeneralProcessing()
{
  static int samplesDuringThisCycle;             // for normalising the power in each mains cycle
  static enum polarities polarityOfLastSampleV;  // for zero-crossing detection
  static long cumVdeltasThisCycle_long;    // for the LPF which determines DC offset (voltage)

  // for AC failure detection 
  enum voltageZones voltageZoneNow; 
  boolean nextPeakDetected = false;
  unsigned long timeNow = millis();
  
  // remove DC offset from the raw voltage sample by subtracting the accurate value 
  // as determined by a LP filter.
  long sampleVminusDC_long = ((long)sampleV<<8) - DCoffset_V_long; 
  
  /* ---------------------------------------
   * New section for AC failure detection
   */
   
  // First, determine which zone the filtered voltage level is in now.
  if (sampleVminusDC_long > voltageThesholdUpper_long) {
    voltageZoneNow = POSITIVE_ZONE; }
  else
  if (sampleVminusDC_long < voltageThesholdLower_long) {
    voltageZoneNow = NEGATIVE_ZONE; }
  else {
    voltageZoneNow = MIDDLE_ZONE; }

  // Check whether the voltage has just appeared in the next 
  // peak zone as expected.  If it has, reset the timer logic.
  //
  if (voltageZoneOfLastSampleV == MIDDLE_ZONE) 
  {
    if ( (voltageZoneNow == POSITIVE_ZONE) &&
         (lastPeakZoneVisited == NEGATIVE_ZONE) )
    {
      nextPeakDetected = true;
    } 
    else
    if ( (voltageZoneNow == NEGATIVE_ZONE) &&
         (lastPeakZoneVisited == POSITIVE_ZONE) )
    {
      nextPeakDetected = true;
    } 
    
    if (nextPeakDetected)
    {
      timeOfLastPeakEntry = timeNow;
      lastPeakZoneVisited = voltageZoneNow;
    }
  }
  
  // Now check whether we're out of time
  if (timeNow - timeOfLastPeakEntry > MAX_INTERVAL_BETWEEN_CONSECUTIVE_PEAKS)
  {
    // AC has failed, so raise the alarm
    digitalWrite(LEDpin, LED_ON);
  }
  else
  {
    // no problem, so clear the alarm
    digitalWrite(LEDpin, LED_OFF);
  }
/*  end of section for AC failure detection 
 * ---------------------------------------
 */
 
  // determine polarity, to aid the logical flow
  enum polarities polarityNow;   
  if(sampleVminusDC_long > 0) { 
    polarityNow = POSITIVE; }
  else { 
    polarityNow = NEGATIVE; }

  if (polarityNow == POSITIVE) 
  { 

    if (beyondStartUpPhase)
    {      
      if (polarityOfLastSampleV != POSITIVE)
      {
        // This is the start of a new +ve half cycle (just after the zero-crossing point)
        cycleCount++;          
      } // end of processing that is specific to the first Vsample in each +ve half cycle   
    }
    else
    {  
      // wait until the DC-blocking filters have had time to settle
      if(millis() > startTime + (startUpPeriod * 1000)) 
      {
        beyondStartUpPhase = true;
        Serial.println ("Go!");
      }
    }
    
  } // end of processing that is specific to samples where the voltage is positive
  
  else // the polatity of this sample is negative
  {     
    if (polarityOfLastSampleV != NEGATIVE)
    {
      // This is a convenient point to update the Low Pass Filter for DC-offset removal
      long previousOffset = DCoffset_V_long;
      DCoffset_V_long = previousOffset + (cumVdeltasThisCycle_long>>6); // faster than * 0.01
      cumVdeltasThisCycle_long = 0;
      
      // To ensure that the LPF will always start up correctly when 240V AC is available, its
      // output value needs to be prevented from drifting beyond the likely range of the 
      // voltage signal.  This avoids the need to use a HPF as was done for initial Mk2 builds.
      //
      if (DCoffset_V_long < DCoffset_V_min) {
        DCoffset_V_long = DCoffset_V_min; }
      else  
      if (DCoffset_V_long > DCoffset_V_max) {
        DCoffset_V_long = DCoffset_V_max; }
                   
    } // end of processing that is specific to the first Vsample in each -ve half cycle
  } // end of processing that is specific to samples where the voltage is positive
  
  // processing for EVERY pair of samples
  //


  samplesDuringThisCycle++;
  
  // store items for use during next loop
  cumVdeltasThisCycle_long += sampleVminusDC_long; // for use with LP filter
//  lastSampleVminusDC_long = sampleVminusDC_long;  // required for phaseCal algorithm
  polarityOfLastSampleV = polarityNow;  // for identification of half cycle boundaries
  voltageZoneOfLastSampleV = voltageZoneNow; // for voltage failure detection
//  updateDisplay();
}
//  ----- end of main Mk2i code -----



int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}



