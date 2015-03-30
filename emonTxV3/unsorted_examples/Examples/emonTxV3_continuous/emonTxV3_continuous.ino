//      Code by Robin Emley (calypso_rae on Open Energy Monitor Forum) - September 2013
//      Updated November 2013 to include analog and LED pins for the emonTx V3 by Glyn Hudson
//
//      Upgrades by RAE, June 2014.  
//      - The RF transmission interval can now be changed, it was previously fixed at once per second.
//      - The on-board LED now gives a 40ms pulse when an RF message is sent
//      - The RFM12B now remains active rather than routinely being put to sleep.  This is to
//         avoid creating gaps in the continuous monitoring process.
//      - When run on an emonTx V3, and using a standard SCT-013-000 CT , the calculated values 
//         of powerCal appear to be within 5% of the optimal values.  
//
//      The interrupt-based kernel for this sketch was kindly provided by Jorg Becker.

#include <Arduino.h>         // may not be needed, but it's probably a good idea to include this
#include <RFu_JeeLib.h>     // RFu_JeeLib is available at from: http://github.com/openenergymonitor/RFu_jeelib

#define DATALOG_PERIOD_IN_SECONDS 5
#define RF_SPARSITY 1 // normally set to 1.  Can be increased to investigate the effect of 
                      //                         including or omitting RF transmissions
#define CYCLES_PER_SECOND 50
#define LED_ON 1
#define LED_OFF 0


// In this sketch, the ADC is free-running with a cycle time of ~104uS.

//  WORKLOAD_CHECK is available for determining how much spare processing time there 
//  is.  To activate this mode, the #define line below should be included: 
//#define WORKLOAD_CHECK  

// ----------------- RF setup  ---------------------
#define RF_freq RF12_868MHZ // Use the RF_RF_RF_RF_RF_freq to match the module you have.

const int nodeID = 10;  // emonTx RFM12B node ID
const int networkGroup = 210;  // emonTx RFM12B wireless network group - needs to be same as emonBase and emonGLCD 
const int UNO = 1;  // Set to 0 if you're not using the UNO bootloader (i.e using Duemilanove) 
                                               // - All Atmega's shipped from OpenEnergyMonitor come with Arduino Uno bootloader
/*
typedef struct { byte dumpState; int msgNumber; } Tx_struct;    //  data for RF comms
Tx_struct tx_data;
*/
 typedef struct { 
//   int msgNumber; 
   int power_CT1;
   int power_CT2;
   int power_CT3;
   int power_CT4; } Tx_struct;    // revised data for RF comms
Tx_struct tx_data;

// -----------------------------------------------------

enum polarities {NEGATIVE, POSITIVE};


// ----------- Pinout assignments  -----------
//
// digital input pins:
// dig pin 0 is for Serial Rx
// dig pin 1 is for Serial Tx
// dig pin 2 is for the RFM12B module (IRQ) 
// dig pin 10 is for the RFM12B module (SEL) 
// dig pin 11 is for the RFM12B module (SDI) 
// dig pin 12 is for the RFM12B module (SDO) 
// dig pin 13 is for the RFM12B module (CLK) 
const byte LedPin = 6;                          //emonTx V3 LED pin

// analogue input pins (for emonTx V3): 
const byte voltageSensor = 0;       // analogue
const byte currentSensor_CT1 = 1;   // analogue
const byte currentSensor_CT2 = 2;   // analogue
const byte currentSensor_CT3 = 3;   // analogue 
const byte currentSensor_CT4 = 4;   // analogue 


// --------------  general global variables -----------------
//
// Some of these variables are used in multiple blocks so cannot be static.
// For integer maths, many variables need to be 'long'
//
boolean beyondStartUpPhase = false;    // start-up delay, allows things to settle
const byte startUpPeriod = 3;      // in seconds, to allow LP filter to settle
const int DCoffset_I = 512;        // nominal mid-point value of ADC @ x1 scale

long energyInBucket_long_CT1 = 0;      // to record the present level in the energy accumulator
long energyInBucket_long_CT2 = 0;      // to record the present level in the energy accumulator
long energyInBucket_long_CT3 = 0;      // to record the present level in the energy accumulator
long energyInBucket_long_CT4 = 0;      // to record the present level in the energy accumulator
int phaseCal_int_CT1;                  // to avoid the need for floating-point maths
int phaseCal_int_CT2;                  // to avoid the need for floating-point maths
int phaseCal_int_CT3;                  // to avoid the need for floating-point maths
int phaseCal_int_CT4;                  // to avoid the need for floating-point maths
long DCoffset_V_long;              // <--- for LPF
long DCoffset_V_min;               // <--- for LPF
long DCoffset_V_max;               // <--- for LPF

// for interaction between the main processor and the ISR 
volatile boolean dataReady = false;
int sampleI_CT1;
int sampleI_CT2;
int sampleI_CT3;
int sampleI_CT4;
int sampleV;

unsigned int cycleCount = 0; 


// Calibration values
//-------------------
// Two calibration values are used in this sketch: powerCal, and phaseCal. 
// With most hardware, the default values are likely to work fine without 
// need for change.  A compact explanation of each of these values now follows:

// When calculating real power, which is what this code does, the individual 
// conversion rates for voltage and current are not of importance.  It is 
// only the conversion rate for POWER which is important.  This is the 
// product of the individual conversion rates for voltage and current.  It 
// therefore has the units of ADC-steps squared per Watt.  Most systems will
// have a power conversion rate of around 20 (ADC-steps squared per Watt).
// 
// powerCal is the RECIPR0CAL of the power conversion rate.  A good value 
// to start with is therefore 1/20 = 0.05 (Watts per ADC-step squared)
//

// Voltage calibration constant:

// AC-AC Voltage adapter is designed to step down the voltage from 230V to 9V
// but the AC Voltage adapter is running open circuit and so output voltage is
// likely to be 20% higher than 9V (9 x 1.2) = 10.8V. 
// Open circuit step down = 230 / 10.8 = 21.3

// The output voltage is then steped down further with the voltage divider which has 
// values Rb = 10k, Rt = 120k (which will reduce the voltage by 13 times.

// The combined step down is therefore 21.3 x 13 = 276.9 which is the 
// theoretical calibration constant entered below.

// Current calibration constant:
// Current calibration constant = 2000 / 22 Ohms burden resistor (The CT sensor has a ratio of 2000:1)

const float powerCal_CT1 = (276.9*(3.3/1023))*(90.9*(3.3/1023)); // <---- powerCal value
const float powerCal_CT2 = (276.9*(3.3/1023))*(90.9*(3.3/1023)); // <---- powerCal value
const float powerCal_CT3 = (276.9*(3.3/1023))*(90.9*(3.3/1023)); // <---- powerCal value
const float powerCal_CT4 = (276.9*(3.3/1023))*(16.6*(3.3/1023)); // <---- powerCal value (2000 / 120R burden resistor)

//const float powerCal_CT1 = 0.044;  // <---- powerCal value  
//const float powerCal_CT2 = 0.044;  // <---- powerCal value  
//const float powerCal_CT3 = 0.044;  // <---- powerCal value  
//const float powerCal_CT4 = 0.044;  // <---- powerCal value  
                        
// phaseCal is used to alter the phase of the voltage waveform relative to the
// current waveform.  The algorithm interpolates between the most recent pair
// of voltage samples according to the value of phaseCal. 
//
//    With phaseCal = 1, the most recent sample is used.  
//    With phaseCal = 0, the previous sample is used
//    With phaseCal = 0.5, the mid-point (average) value in used
//
// NB. Any tool which determines the optimal value of phaseCal must have a similar 
// scheme for taking sample values as does this sketch!
// http://openenergymonitor.org/emon/node/3792#comment-18683
const float  phaseCal_CT1 = 0.22;
const float  phaseCal_CT2 = 0.41;
const float  phaseCal_CT3 = 0.6;
const float  phaseCal_CT4 = 1.25;

int rfsendtimer = 0; // every time this gets to 10 we send an rf packet
int datalogCountInMainsCycles;
const int maxDatalogCountInMainsCycles = DATALOG_PERIOD_IN_SECONDS * CYCLES_PER_SECOND;
float normalisation_CT1;
float normalisation_CT2;
float normalisation_CT3;
float normalisation_CT4;

boolean LED_pulseInProgress = false;
unsigned long LED_onAt;



void setup()
{  
  rf12_initialize(nodeID, RF_freq, networkGroup);             // initialize RF
//  rf12_sleep(RF12_SLEEP); <- now active continuously 
  
  Serial.begin(9600);                                      // initialize Serial interface
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println("----------------------------------");
  Serial.println("Sketch ID:      emonTxV3_4chan.ino");
       
  pinMode(LedPin, OUTPUT);                                              // Setup indicator LED
       
  // When using integer maths, calibration values that have supplied in floating point 
  // form need to be rescaled.  
  //
  phaseCal_int_CT1 = phaseCal_CT1 * 256; // for integer maths
  phaseCal_int_CT2 = phaseCal_CT2 * 256; // for integer maths
  phaseCal_int_CT3 = phaseCal_CT3 * 256; // for integer maths
  phaseCal_int_CT4 = phaseCal_CT4 * 256; // for integer maths
    
  // Define operating limits for the LP filter which identifies DC offset in the voltage 
  // sample stream.  By limiting the output range, the filter always should start up 
  // correctly.
  DCoffset_V_long = 512L * 256; // nominal mid-point value of ADC @ x256 scale  
  DCoffset_V_min = (long)(512L - 100) * 256; // mid-point of ADC minus a working margin
  DCoffset_V_max = (long)(512L + 100) * 256; // mid-point of ADC plus a working margin

  normalisation_CT1 = powerCal_CT1 / maxDatalogCountInMainsCycles;
  normalisation_CT2 = powerCal_CT2 / maxDatalogCountInMainsCycles;
  normalisation_CT3 = powerCal_CT3 / maxDatalogCountInMainsCycles;
  normalisation_CT4 = powerCal_CT4 / maxDatalogCountInMainsCycles;

  Serial.println ("ADC mode:       free-running");
  
  // Set up the ADC to be free-running 
  ADCSRA  = (1<<ADPS0)+(1<<ADPS1)+(1<<ADPS2);  // Set the ADC's clock to system clock / 128
  ADCSRA |= (1 << ADEN);                 // Enable the ADC 
  
  ADCSRA |= (1<<ADATE);  // set the Auto Trigger Enable bit in the ADCSRA register.  Because 
                         // bits ADTS0-2 have not been set (i.e. they are all zero), the 
                         // ADC's trigger source is set to "free running mode".
                         
  ADCSRA |=(1<<ADIE);    // set the ADC interrupt enable bit. When this bit is written 
                         // to one and the I-bit in SREG is set, the 
                         // ADC Conversion Complete Interrupt is activated. 

  ADCSRA |= (1<<ADSC);   // start ADC manually first time 
  sei();                 // Enable Global Interrupts  

     
  char flag = 0;
  Serial.print ( "Extra Features: ");  
#ifdef WORKLOAD_CHECK  
  Serial.print ( "WORKLOAD_CHECK ");
  flag++;
#endif
  if (flag == 0) {
    Serial.print ("none"); }
  Serial.println ();
        
  Serial.print ( "powerCal_CT1 =      "); Serial.println (powerCal_CT1,4);
  Serial.print ( "phaseCal_CT1 =      "); Serial.println (phaseCal_CT1);
  Serial.print ( "powerCal_CT2 =      "); Serial.println (powerCal_CT2,4);
  Serial.print ( "phaseCal_CT2 =      "); Serial.println (phaseCal_CT2);
  Serial.print ( "powerCal_CT3 =      "); Serial.println (powerCal_CT3,4);
  Serial.print ( "phaseCal_CT3 =      "); Serial.println (phaseCal_CT3);
  Serial.print ( "powerCal_CT4 =      "); Serial.println (powerCal_CT4,4);
  Serial.print ( "phaseCal_CT4 =      "); Serial.println (phaseCal_CT4);
  
  Serial.println ("----");    

#ifdef WORKLOAD_CHECK
   Serial.println ("WELCOME TO WORKLOAD_CHECK ");
  
//   <<- start of commented out section, to save on RAM space!
/*   
   Serial.println ("  This mode of operation allows the spare processing capacity of the system");
   Serial.println ("to be analysed.  Additional delay is gradually increased until all spare time");
   Serial.println ("has been used up.  This value (in uS) is noted and the process is repeated.  ");
   Serial.println ("The delay setting is increased by 1uS at a time, and each value of delay is ");
   Serial.println ("checked several times before the delay is increased. "); 
 */ 
//  <<- end of commented out section, to save on RAM space!

   Serial.println ("  The displayed value is the amount of spare time, per pair of V & I samples, ");
   Serial.println ("that is available for doing additional processing.");
   Serial.println ();
 #endif
}

// An Interrupt Service Routine is now defined in which the ADC is instructed to perform 
// a conversion of the voltage signal and each of the signals for current.  A "data ready" 
// flag is set after each voltage conversion has been completed, it being the last one in
// the sequence.  
//   Samples for current are taken first because the phase of the waveform for current is 
// generally slightly advanced relative to the waveform for voltage.  The data ready flag 
// is cleared within loop().

// This Interrupt Service Routine is for use when the ADC is in the free-running mode.
// It is executed whenever an ADC conversion has finished, approx every 104 us.  In 
// free-running mode, the ADC has already started its next conversion by the time that
// the ISR is executed.  The ISR therefore needs to "look ahead". 
//   At the end of conversion Type N, conversion Type N+1 will start automatically.  The ISR 
// which runs at this point therefore needs to capture the results of conversion Type N , 
// and set up the conditions for conversion Type N+2, and so on.  
// 
ISR(ADC_vect)  
{                                         
  static unsigned char sample_index = 0;
  
  switch(sample_index)
  {
    case 0:
      sampleV = ADC; 
      ADMUX = 0x40 + currentSensor_CT2; // set up the next-but-one conversion
      sample_index++; // advance the control flag             
      dataReady = true; 
      break;
    case 1:
      sampleI_CT1 = ADC; 
      ADMUX = 0x40 + currentSensor_CT3; // for the next-but-one conversion
      sample_index++; // advance the control flag                
      break;
    case 2:
      sampleI_CT2 = ADC; 
      ADMUX = 0x40 + currentSensor_CT4; // for the next-but-one conversion
      sample_index++; // advance the control flag                
      break;
    case 3:
      sampleI_CT3 = ADC; 
      ADMUX = 0x40 + voltageSensor; // for the next-but-one conversion
      sample_index++; // advance the control flag                 
      break;
    case 4:
      sampleI_CT4 = ADC; 
      ADMUX = 0x40 + currentSensor_CT1; // for the next-but-one conversion
      sample_index = 0; // reset the control flag                
      break;
    default:
      sample_index = 0;                 // to prevent lockup (should never get here)      
  }  
}


// When using interrupt-based logic, the main processor waits in loop() until the 
// dataReady flag has been set by the ADC.  Once this flag has been set, the main
// processor clears the flag and proceeds with all the processing for one pair of 
// V & I samples.  It then returns to loop() to wait for the next pair to become 
// available.
//   If the next pair of samples become available before the processing of the 
// previous pair has been completed, data could be lost.  This situation can be 
// avoided by prior use of the WORKLOAD_CHECK mode.  Using this facility, the amount
// of spare processing capacity per loop can be determined.  
//
void loop()             
{ 
#ifdef WORKLOAD_CHECK
  static int del = 0; // delay, as passed to delayMicroseconds()
  static int res = 0; // result, to be displayed at the next opportunity
  static byte count = 0; // to allow multiple runs per setting
  static byte displayFlag = 0; // to determine when printing may occur
#endif
  
  if (dataReady)   // flag is set after every pair of ADC conversions
  {
    dataReady = false; // reset the flag
    allGeneralProcessing(); // executed once for each pair of V&I samples
    
#ifdef WORKLOAD_CHECK 
    delayMicroseconds(del); // <--- to assess how much spare time there is
    if (dataReady)       // if data is ready again, delay was too long
    { 
      res = del;             // note the exact value
      del = 1;               // and start again with 1us delay   
      count = 0;
      displayFlag = 0;   
    }
    else
    {
      count++;          // to give several runs with the same value
      if (count > 50)
      {
        count = 0;
        del++;          //  increase delay by 1uS
      } 
    }
#endif  

  }  // <-- this closing brace needs to be outside the WORKLOAD_CHECK blocks! 
  
#ifdef WORKLOAD_CHECK 
  switch (displayFlag) 
  {
    case 0: // the result is available now, but don't display until the next loop
      displayFlag++;
      break;
    case 1: // with minimum delay, it's OK to print now
      Serial.print(res);
      displayFlag++;
      break;
    case 2: // with minimum delay, it's OK to print now
      Serial.println("uS");
      displayFlag++;
      break;
    default:; // for most of the time, displayFlag is 3           
  }
#endif
  
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
  static long sumP_CT1;                              // for per-cycle summation of 'real power' 
  static long sumP_CT2;                              // for per-cycle summation of 'real power' 
  static long sumP_CT3;                              // for per-cycle summation of 'real power' 
  static long sumP_CT4;                              // for per-cycle summation of 'real power' 
  static enum polarities polarityOfLastSampleV;  // for zero-crossing detection
  static long cumVdeltasThisCycle_long;    // for the LPF which determines DC offset (voltage)
  static long lastSampleVminusDC_long;     //    for the phaseCal algorithm
  static byte whenToSendRFmessageCount = 0;
  
  // remove DC offset from the raw voltage sample by subtracting the accurate value 
  // as determined by a LP filter.
  long sampleVminusDC_long = ((long)sampleV<<8) - DCoffset_V_long; 

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
        
        // Calculate the real power and energy during the last whole mains cycle.
        //
        // sumP contains the sum of many individual calculations of instantaneous power.  In  
        // order to obtain the average power during the relevant period, sumP must first be 
        // divided by the number of samples that have contributed to its value.
        //
        // The next stage would normally be to apply a calibration factor so that real power 
        // can be expressed in Watts.  That's fine for floating point maths, but it's not such
        // a good idea when integer maths is being used.  To keep the numbers large, and also 
        // to save time, calibration of power is omitted at this stage.  realPower_long is 
        // therefore (1/powerCal) times larger than the actual power in Watts.
        //
        long realPower_long_CT1 = sumP_CT1 / samplesDuringThisCycle; // proportional to Watts
        long realPower_long_CT2 = sumP_CT2 / samplesDuringThisCycle; // proportional to Watts
        long realPower_long_CT3 = sumP_CT3 / samplesDuringThisCycle; // proportional to Watts
        long realPower_long_CT4 = sumP_CT4 / samplesDuringThisCycle; // proportional to Watts
   
        // Next, the energy content of this power rating needs to be determined.  Energy is 
        // power multiplied by time, so the next step is normally to multiply by the time over 
        // which the power was measured.
        //   Instanstaneous power is calculated once every mains cycle, so that's every fiftieth 
        // of a second.  When integer maths is being used, this routine power-to-energy conversion 
        // seems an unnecessary workload.  As all sampling periods are of similar duration (20mS), 
        // it is more efficient simply to add all the power samples together, and note that their 
        // sum is actually 50 times greater than it would otherwise be.
        //   Although the numerical value itself does not change, a new name is helpful so as  
        // to avoid any confusion.  The 'energy' variable below is 50 * (1/powerCal) times larger 
        // than the actual energy in Joules.
        //
        long realEnergy_long_CT1 = realPower_long_CT1; 
        long realEnergy_long_CT2 = realPower_long_CT2; 
        long realEnergy_long_CT3 = realPower_long_CT3; 
        long realEnergy_long_CT4 = realPower_long_CT4; 
        
        // add this latest contribution to the energy bucket
        energyInBucket_long_CT1 += realEnergy_long_CT1;    
        energyInBucket_long_CT2 += realEnergy_long_CT2;    
        energyInBucket_long_CT3 += realEnergy_long_CT3;    
        energyInBucket_long_CT4 += realEnergy_long_CT4;    
        
        datalogCountInMainsCycles++;
        
        if (datalogCountInMainsCycles >= maxDatalogCountInMainsCycles)
        {         
          tx_data.power_CT1 = energyInBucket_long_CT1 * normalisation_CT1;
          tx_data.power_CT2 = energyInBucket_long_CT2 * normalisation_CT2;
          tx_data.power_CT3 = energyInBucket_long_CT3 * normalisation_CT3;
          tx_data.power_CT4 = energyInBucket_long_CT4 * normalisation_CT4;          
          boolean OKtoSend;
          
          // for investigating the effect of RF transmissions on the sampling process
          if (RF_SPARSITY == 1)
          {
            OKtoSend = true; 
          }
          else
          {
            whenToSendRFmessageCount++;
            if (whenToSendRFmessageCount >= RF_SPARSITY)
            {
              whenToSendRFmessageCount = 0;
              OKtoSend = true; 
            }
            else
            {
              OKtoSend = false;
            } 
          }
          
         
          if (OKtoSend)
          {
            send_rf_data(); 
            digitalWrite(LedPin, LED_ON); 
            LED_pulseInProgress = true;        
          }
          
          // Serial activity has to follow RF
          Serial.print(tx_data.power_CT1);
          Serial.print(", ");
          Serial.print(tx_data.power_CT2);
          Serial.print(", ");
          Serial.print(tx_data.power_CT3);
          Serial.print(", ");
          Serial.print(tx_data.power_CT4);
          
          if (OKtoSend)
          {
            Serial.println();
          }
          else
          {
            Serial.println(" << RF not sent!");
          }  

          
          datalogCountInMainsCycles = 0;
          energyInBucket_long_CT1 = 0;         
          energyInBucket_long_CT2 = 0;         
          energyInBucket_long_CT3 = 0;         
          energyInBucket_long_CT4 = 0;            
        }

        // clear the per-cycle accumulators for use in this new mains cycle.  
        samplesDuringThisCycle = 0;
        sumP_CT1 = 0;
        sumP_CT2 = 0;
        sumP_CT3 = 0;
        sumP_CT4 = 0;

      } // end of processing that is specific to the first Vsample in each +ve half cycle   
    }
    else
    {  
      // wait until the DC-blocking filters have had time to settle
      if(millis() > startUpPeriod * 1000) 
      {
        beyondStartUpPhase = true;
        sumP_CT1 = 0;
        sumP_CT2 = 0;
        sumP_CT3 = 0;
        sumP_CT4 = 0;
        samplesDuringThisCycle = 0;
        Serial.println ("Go!");
      }
    }
  } // end of processing that is specific to samples where the voltage is positive
  
  else // the polarity of this sample is negative
  {     
    if (polarityOfLastSampleV != NEGATIVE)
    {
      // This is the start of a new -ve half cycle (just after the zero-crossing point)
      //
      // This is a convenient point to update the Low Pass Filter for DC-offset removal,
      // which needs to be done right from the start.
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
        
      check_LED_status();
        
    } // end of processing that is specific to the first Vsample in each -ve half cycle
  } // end of processing that is specific to samples where the voltage is positive
  
  // Processing for EVERY pair of samples. Most of this code is not used during the 
  // start-up period, but it does no harm to leave it in place.  Accumulated values 
  // are cleared when beyondStartUpPhase is set to true.
  //
  // remove most of the DC offset from the current sample (the precise value does not matter)
  long sampleIminusDC_long_CT1 = ((long)(sampleI_CT1-DCoffset_I))<<8;
  long sampleIminusDC_long_CT2 = ((long)(sampleI_CT2-DCoffset_I))<<8;
  long sampleIminusDC_long_CT3 = ((long)(sampleI_CT3-DCoffset_I))<<8;
  long sampleIminusDC_long_CT4 = ((long)(sampleI_CT4-DCoffset_I))<<8;
  
  // phase-shift the voltage waveform so that it aligns with the current when a 
  // resistive load is used
  long  phaseShiftedSampleVminusDC_long_CT1 = lastSampleVminusDC_long
         + (((sampleVminusDC_long - lastSampleVminusDC_long)*phaseCal_int_CT1)>>8);  
  long  phaseShiftedSampleVminusDC_long_CT2 = lastSampleVminusDC_long
         + (((sampleVminusDC_long - lastSampleVminusDC_long)*phaseCal_int_CT2)>>8);  
  long  phaseShiftedSampleVminusDC_long_CT3 = lastSampleVminusDC_long
         + (((sampleVminusDC_long - lastSampleVminusDC_long)*phaseCal_int_CT3)>>8);  
  long  phaseShiftedSampleVminusDC_long_CT4 = lastSampleVminusDC_long
         + (((sampleVminusDC_long - lastSampleVminusDC_long)*phaseCal_int_CT4)>>8);  
  
  // calculate the "real power" in this sample pair and add to the accumulated sum
  long filtV_div4_CT1 = phaseShiftedSampleVminusDC_long_CT1>>2;  // reduce to 16-bits (now x64, or 2^6)
  long filtI_div4_CT1 = sampleIminusDC_long_CT1>>2; // reduce to 16-bits (now x64, or 2^6)
  long instP_CT1 = filtV_div4_CT1 * filtI_div4_CT1;  // 32-bits (now x4096, or 2^12)
  instP_CT1 = instP_CT1>>12;     // scaling is now x1, as for Mk2 (V_ADC x I_ADC)       
  sumP_CT1 +=instP_CT1; // cumulative power, scaling as for Mk2 (V_ADC x I_ADC)
  
  long filtV_div4_CT2 = phaseShiftedSampleVminusDC_long_CT2>>2;  // reduce to 16-bits (now x64, or 2^6)
  long filtI_div4_CT2 = sampleIminusDC_long_CT2>>2; // reduce to 16-bits (now x64, or 2^6)
  long instP_CT2 = filtV_div4_CT2 * filtI_div4_CT2;  // 32-bits (now x4096, or 2^12)
  instP_CT2 = instP_CT2>>12;     // scaling is now x1, as for Mk2 (V_ADC x I_ADC)       
  sumP_CT2 +=instP_CT2; // cumulative power, scaling as for Mk2 (V_ADC x I_ADC)
  
  long filtV_div4_CT3 = phaseShiftedSampleVminusDC_long_CT3>>2;  // reduce to 16-bits (now x64, or 2^6)
  long filtI_div4_CT3 = sampleIminusDC_long_CT3>>2; // reduce to 16-bits (now x64, or 2^6)
  long instP_CT3 = filtV_div4_CT3 * filtI_div4_CT3;  // 32-bits (now x4096, or 2^12)
  instP_CT3 = instP_CT3>>12;     // scaling is now x1, as for Mk2 (V_ADC x I_ADC)       
  sumP_CT3 +=instP_CT3; // cumulative power, scaling as for Mk2 (V_ADC x I_ADC)
  
  long filtV_div4_CT4 = phaseShiftedSampleVminusDC_long_CT4>>2;  // reduce to 16-bits (now x64, or 2^6)
  long filtI_div4_CT4 = sampleIminusDC_long_CT4>>2; // reduce to 16-bits (now x64, or 2^6)
  long instP_CT4 = filtV_div4_CT4 * filtI_div4_CT4;  // 32-bits (now x4096, or 2^12)
  instP_CT4 = instP_CT4>>12;     // scaling is now x1, as for Mk2 (V_ADC x I_ADC)       
  sumP_CT4 +=instP_CT4; // cumulative power, scaling as for Mk2 (V_ADC x I_ADC)
  
  samplesDuringThisCycle++;
  
  // store items for use during next loop
  cumVdeltasThisCycle_long += sampleVminusDC_long; // for use with LP filter
  lastSampleVminusDC_long = sampleVminusDC_long;  // required for phaseCal algorithm
  
  polarityOfLastSampleV = polarityNow;  // for identification of half cycle boundaries
}
// end of loop()


void check_LED_status()
{
  if (LED_pulseInProgress == true)
  {
    if (cycleCount > (LED_onAt + 2)) // pulse duration = 40 ms
    {
      digitalWrite(LedPin, LED_OFF); 
      LED_pulseInProgress = false; 
    }
  }
}  


void send_rf_data()
{
//  rf12_sleep(RF12_WAKEUP);
  // if ready to send + exit route if it gets stuck   
  int i = 0; 
  while (!rf12_canSend() && i<10)
  { 
    rf12_recvDone(); 
    i++;
  }  
//  rf12_sendStart(0, &tx_data, sizeof tx_data);
  rf12_sendNow(0, &tx_data, sizeof tx_data);

//  rf12_sendWait(2);  <- removed because this is a 'blocking' function
//  rf12_sleep(RF12_SLEEP); <- the RF chip is now always in its active state
}


int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}


