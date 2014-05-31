// Tool for emulating a digital supply meter.
//
// The original version was released in October 2012 for use on an
// Arduino Uno or emonTX V3.  The intended burden resistor was 150R.
//
// May 2014:
// Upgraded for use on the emonTx V3 using calibration data supplied by 
// Glyn Hudson and Robert Wall.  When run on an emonTx V3, the on-board LED
// will flash once for every WattHour's worth of energy that is consumed.  
// This allows the hardware to be calibrated against a digital supply meter.
//
// Once the optimal value of powerCal has been ascertained, that same value 
// can be used with confidence on all Mk2 PV Router sketches than I have posted.
//
//                  Robin Emley (calypso_rae on Open Energy Monitor Forum)
//                  May 2014

#define NEGATIVE 0
#define POSITIVE 1
#define ON 1
#define OFF 0

const byte outputPinForLed = 6; // for the emonTx V3
const byte voltageSensorPin = 0; // for the emonTx V3

// only one CT input can be active at a time!
// ******************************************
const byte currentSensorPin = 1; // for the emonTx V3 (CT1)
//const byte currentSensorPin = 2; // for the emonTx V3 (CT2)
//const byte currentSensorPin = 3; // for the emonTx V3 (CT3)
//const byte currentSensorPin = 4; // for the emonTx V3 (CT4)
// ******************************************



long cycleCount = 0; // used to time LED events, rather than calling millis()
int samplesDuringThisMainsCycle = 0;
float cyclesPerSecond = 50; // use float to ensure accurate maths

long noOfSamplePairs = 0;
byte polarityNow; 

boolean beyondStartUpPhase = false;

float energyInBucket = 10; // mimics the operation of a meter at the grid connection point.                                                
int capacityOfEnergyBucket = 3600; // 0.001 kWh = 3600 Joules
int sampleV,sampleI;   // voltage & current samples are integers in the ADC's input range 0 - 1023 
int lastSampleV;     // stored value from the previous loop (HP filter is for voltage samples only)         
float lastFilteredV,filteredV;  //  voltage values after HP-filtering to remove the DC offset

float prevDCoffset;          // <<--- for LPF 
float DCoffset;              // <<--- for LPF 
float cumVdeltasThisCycle;   // <<--- for LPF 
float sampleVminusDC;         // <<--- for LPF
float sampleIminusDC;         // <<--- used with LPF
float lastSampleVminusDC;     // <<--- used with LPF
float sumP;   //  cumulative sum of power calculations within each mains cycle

// items for charging, including LED display
boolean myLED_pulseInProgress = false;
unsigned long myLED_onAt;
int consumedPowerRegister = 0;
float energyLevelForDiags = 0;
float lastEnergyLevelForDiags;

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

// for use with CT1 - CT3
float POWERCAL = (276.9*(3.3/1023))*(90.9*(3.3/1023)); // <---- powerCal value

// for use with CT4
// float POWERCAL = (276.9*(3.3/1023))*(16.6*(3.3/1023)); // <---- powerCal value (2000 / 120R burden resistor)

float PHASECAL = 1;                    



void setup()
{  
  Serial.begin(9600);
  pinMode(outputPinForLed, OUTPUT);  
  digitalWrite(outputPinForLed, ON); 
  delay(5000);
    
  // powerCal can be adjusted here for improved accuracy
  //     
//  POWERCAL = 0.25; // Units are Joules per ADC-level squared.  Used for converting the product of 
                    // voltage and current samples into Joules.
  
  PHASECAL = 1;                         
}


void loop() // each loop is for one pair of V & I measurements
{
  noOfSamplePairs++;              // for stats only
  samplesDuringThisMainsCycle++;  // for power calculation at the start of each mains cycle

  // store values from previous loop
  lastSampleV=sampleV;            // for digital high-pass filter
  lastFilteredV = filteredV;      // for HPF, used to identify the start of each mains cycle
  lastSampleVminusDC = sampleVminusDC;  // for phasecal calculation 
 
// Get the next pair of raw samples.  Because the CT generally adds more phase-advance 
// than the voltage sensor, it makes sense to sample current before voltage
  sampleI = analogRead(currentSensorPin);   
  sampleV = analogRead(voltageSensorPin);   

  // remove the DC offset from these samples as determined by a low-pass filter
  sampleVminusDC = sampleV - DCoffset; 
  sampleIminusDC = sampleI - DCoffset;

  // a high-pass filter is used just for determining the start of each mains cycle  
  filteredV = 0.996*(lastFilteredV+sampleV-lastSampleV);   

  // Establish the polarities of the latest and previous filtered voltage samples
  byte polarityOfLastReading = polarityNow;
  if(filteredV >= 0) 
    polarityNow = POSITIVE; 
  else 
    polarityNow = NEGATIVE;


  if (polarityNow == POSITIVE)
  {
    if (polarityOfLastReading != POSITIVE)
    {
      // This is the start of a new mains cycle (just after the +ve going z-c point)
      cycleCount++; // for stats only
//      checkLedStatus(); // a really useful function, but can be commented out if not required

      // update the Low Pass Filter for DC-offset removal
      prevDCoffset = DCoffset;
      DCoffset = prevDCoffset + (0.01 * cumVdeltasThisCycle); 

      //  Calculate the real power of all instantaneous measurements taken during the 
      //  previous mains cycle, and determine the gain (or loss) in energy.
      float realPower = POWERCAL * sumP / (float)samplesDuringThisMainsCycle;
      float realEnergy = realPower / cyclesPerSecond;

      if (beyondStartUpPhase == true)
      {  
        // Providing that the DC-blocking filters have had sufficient time to settle,    
        // add this power contribution to the energy bucket
        energyInBucket += realEnergy;   
        energyLevelForDiags += realEnergy;   
         
        if ((cycleCount % 50) == 0) 
        {
          Serial.println (energyInBucket);
        }
         
        if (energyInBucket > capacityOfEnergyBucket)
        {
          energyInBucket -= capacityOfEnergyBucket;
          registerConsumedPower();   
        }
        
        if (energyInBucket < 0)
        {
          digitalWrite(outputPinForLed, 1);
          energyInBucket = 0; 
        }  
      }
      else
      {  
        // wait until the DC-blocking filters have had time to settle
        if(cycleCount > 100) // 100 mains cycles is 2 seconds
          beyondStartUpPhase = true;
      }
      checkMyLED_status();
      
      // clear the per-cycle accumulators for use in this new mains cycle.  
      sumP = 0;
      samplesDuringThisMainsCycle = 0;
      cumVdeltasThisCycle = 0;
    } // end of processing that is specific to the first +ve Vsample in each new mains cycle
  }  // end of processing that is specific to positive Vsamples
  
  
  // Processing for ALL Vsamples, both positive and negative
  //------------------------------------------------------------
   
  // Apply phase-shift to the voltage waveform to ensure that the system measures a
  // resistive load with a power factor of unity.
  float  phaseShiftedVminusDC = 
                lastSampleVminusDC + PHASECAL * (sampleVminusDC - lastSampleVminusDC);  
  float instP = phaseShiftedVminusDC * sampleIminusDC; //  power contribution for this pair of V&I samples 
  sumP +=instP;     // cumulative power contributions for this mains cycle 

  cumVdeltasThisCycle += (sampleV - DCoffset); // for use with LP filter
} // end of loop()


void registerConsumedPower()
{
  consumedPowerRegister++;
  myLED_onAt = cycleCount;
  digitalWrite(outputPinForLed, ON);  
  myLED_pulseInProgress = true;
  Serial.println(energyLevelForDiags - lastEnergyLevelForDiags);
  lastEnergyLevelForDiags = energyLevelForDiags;
}

void checkMyLED_status()
{
  if (myLED_pulseInProgress == true)
  {
    if (cycleCount > (myLED_onAt + 2)) // pulse duration = 40 ms
    {
      digitalWrite(outputPinForLed, OFF); 
      myLED_pulseInProgress = false; 
    }
  }
}  

