/*
 EmonTx CT123 + 3-phase Voltage example
 
 An example sketch for the emontx module for
 3-phase electricity monitoring.with 3 current transformers 
 and 1 only voltage transformer
 
 Part of the openenergymonitor.org project
 Licence: GNU GPL V3
 
 Authors: Glyn Hudson, Trystan Lea
 Builds upon JeeLabs RF12 library and Arduino
 Extended for 3-phase operation: Robert Wall
 
 emonTx documentation: http://openenergymonitor.org/emon/modules/emontx/
 emonTx firmware code explanation: http://openenergymonitor.org/emon/modules/emontx/firmware
 emonTx calibration instructions: http://openenergymonitor.org/emon/modules/emontx/firmware/calibration
 
 REQUIRES in [Arduino]/libraries
 Arduino.h
 WProgram.h
 avr/wdt.h							// the UNO bootloader 
 JeeLib.h							// Download JeeLib: http://github.com/jcw/jeelib
 REQUIRES in project directory 
 emontx_lib.ino

 (does NOT require EmonLib)
 =============================================================================================
 
 Extended to allow the voltage measurement of a single phase to be used to generate approximate indications of
 power (real and apparent) and phase angle for the other two phases of a 3-phase installation.
 
 The measured voltage of one phase is delayed in an array and subsequently used in the calculations for the
 remaining phases.
 N.B. "Phase shifted" means a small adjustment to the voltage waveform to accommodate small ( < 10 degrees) phase shifts
 in the transformers etc. "Delayed" means a delay of the voltage samples by approx 1/3 or 2/3 cycle.
 
 POSSIBLE SOURCES OF ERROR
 This method is an approximation. It assumes that the voltages of the three phases remain identical and the angles
 between the voltage vectors remain accurately 120 degrees apart. The lower the fault level of the supply (i.e. the higher
 the impedance), the greater the change in the true voltage will be as a result of load changes, and therefore the 
 inaccuracies that result from these approximations will be greater also.
 If the mains frequency changes, this will appear as a change in real power and power factor for L2 and more so for L3. 
 
 CALIBRATION
 Adjust Vcal = 234.26 so that the correct voltage for L1 is displayed.
 Adjust Ical1 = 119.0 so that the correct current for L1 is displayed.
 Do the same for Ical2 & Ical3.
 Connect a pure resistive load (e.g. a heater) to L1 and adjust Phasecal1 to display a power factor of 1.00.
 Do the same for L2 and L3. If it not possible to keep Phasecal within the range 0 - 2, it is permissible to
 change "#define PHASE2 8" and/or "#define PHASE3 17"
 
*/
#define FILTERSETTLETIME 5000                                           //  Time (ms) to allow the filters to settle before sending data

// #define DEBUGGING									// enable this line to include debugging print statements
#define SERIALPRINT								// include print statement for commissioning - comment this line to exclude

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#define freq RF12_868MHZ						// Frequency of RF12B module can be 
												//	RF12_433MHZ, RF12_868MHZ or RF12_915MHZ. 
												//	You should use the one matching the module you
												//	have.

#define PHASE2 8								//  Number of samples delay for L2
#define PHASE3 17								//  Number  of samples delay for L3, also size of array
												//  These can be adjusted if the phase correction is not adequate
						
const int nodeID = 10;							// emonTx RFM12B node ID
const int networkGroup = 210;					// emonTx RFM12B wireless network group
												// - needs to be same as emonBase and emonGLCD needs to be same
												//   as emonBase and emonGLCD

const int UNO = 1;								// Set to 0 if you are not using the UNO bootloader 
												// (i.e using Duemilanove) - All Atmega's shipped from
												// OpenEnergyMonitor come with Arduino Uno bootloader
														
#include <avr/wdt.h>							// the UNO bootloader 

#include <JeeLib.h>								// Download JeeLib: http://github.com/jcw/jeelib
ISR(WDT_vect) { Sleepy::watchdogEvent(); }


//Set Voltage and current input pins
int inPinV = 2;
int inPinI1 = 3;
int inPinI2 = 0;
int inPinI3 = 1;
//Calibration coefficients
//These need to be set in order to obtain accurate results
double Vcal = 234.26;							// Calibration constant for voltage input
double Ical1 = 134.00;							// Calibration constant for current transformer 1
double Ical2 = 134.00;							// Calibration constant for current transformer 2
double Ical3 = 134.00;							// Calibration constant for current transformer 3
double Phasecal1 = 1.00;						// Calibration constant for phase shift L1
double Phasecal2 = 1.68;						// Calibration constant for phase shift L2
double Phasecal3 = 1.00;						// Calibration constant for phase shift L3

//--------------------------------------------------------------------------------------
// Variable declaration for filters, phase shift, voltages, currents & powers
//--------------------------------------------------------------------------------------
int lastSampleV,sampleV;						//sample_ holds the raw analog read value, lastSample_ holds the last sample
int lastSampleI1,sampleI1;
int lastSampleI2,sampleI2;
int lastSampleI3,sampleI3;

double lastFilteredV,filteredV;					//Filtered_ is the raw analog value minus the DC offset
double lastFilteredI1, filteredI1;
double lastFilteredI2, filteredI2;
double lastFilteredI3, filteredI3;

double phaseShiftedV1;							//Holds the calibrated delayed & phase shifted voltage.
double phaseShiftedV2;
double phaseShiftedV3;

double sqV,sumV,sqI1,sumI1,sqI2,sumI2,sqI3,sumI3;
double instP1,sumP1,instP2,sumP2,instP3,sumP3;	//sq = squared, sum = Sum, inst = instantaneous

int startV;										//Instantaneous voltage at start of sample window.

double realPower1,								// The final data
       apparentPower1,
       powerFactor1,
       Irms1,
	   realPower2,
       apparentPower2,
       powerFactor2,
       Irms2,
	   realPower3,
       apparentPower3,
       powerFactor3,
       Irms3,
       Vrms;	   

	
typedef struct { int power1, power2, power3, Vrms; } PayloadTX;		// neat way of packaging data for RF comms
																	// (Include all the variables that are desired,
																	// ensure the same struct is used to receive)

PayloadTX emontx;													// create an instance

const int LEDpin = 9;												// On-board emonTx LED 

boolean settled = false;

void setup() 
{
  Serial.begin(9600);
  Serial.println("emonTX CT123 Voltage 3 Phase example");
  Serial.println("OpenEnergyMonitor.org");
  Serial.print("Node: "); 
  Serial.print(nodeID); 
  Serial.print(" Freq: "); 
  if (freq == RF12_433MHZ) Serial.print("433Mhz");
  else if (freq == RF12_868MHZ) Serial.print("868Mhz");
  else if (freq == RF12_915MHZ) Serial.print("915Mhz"); 
  else Serial.print("Not set");
  Serial.print(" Network: "); 
  Serial.println(networkGroup);
  
  
  rf12_initialize(nodeID, freq, networkGroup);				// initialize RF
  rf12_sleep(RF12_SLEEP);

  pinMode(LEDpin, OUTPUT);									// Setup indicator LED
  digitalWrite(LEDpin, HIGH);
  
  if (UNO) wdt_enable(WDTO_8S);								// Enable anti crash (restart) watchdog if UNO bootloader is selected. 
															//  (Watchdog does not work with duemilanove bootloader)
															// Restarts emonTx if sketch hangs for more than 8s
}

//*********************************************************************************************************************
void loop() 
{ 
// Outer loop - Reads Voltages & Currents - Sends results
  calcVI3Ph(11,2000);											// Calculate all. No.of complete cycles, time-out  

// Removing these print statements is recommended for normal use (if not required).
#ifdef SERIALPRINT
  
  Serial.print("Voltage: "); Serial.println(Vrms);
  Serial.print(" Current 1: "); Serial.print(Irms1);
  Serial.print(" Power 1: "); Serial.print(realPower1);
  Serial.print(" VA 1: "); Serial.print(apparentPower1);
  Serial.print(" PF 1: "); Serial.println(powerFactor1);
  Serial.print(" Current 2: "); Serial.print(Irms2);
  Serial.print(" Power 2: "); Serial.print(realPower2);
  Serial.print(" VA 2: "); Serial.print(apparentPower2);
  Serial.print(" PF 2: "); Serial.println(powerFactor2);
  Serial.print(" Current 3: "); Serial.print(Irms3);
  Serial.print(" Power 3: "); Serial.print(realPower3);
  Serial.print(" VA 3: "); Serial.print(apparentPower3);
  Serial.print(" PF 3: "); Serial.println(powerFactor3);
  
  Serial.println(); delay(100);

#endif 
 
  emontx.power1 = realPower1;								// Copy the desired variables ready for transmision
  emontx.power2 = realPower2;
  emontx.power3 = realPower3;
  emontx.Vrms   = Vrms;

  // because millis() returns to zero after 50 days ! 
  if (!settled && millis() > FILTERSETTLETIME) settled = true;

  if (settled)                                                            // send data only after filters have settled
  { 
    send_rf_data();											// *SEND RF DATA* - see emontx_lib
    digitalWrite(LEDpin, HIGH); delay(2); digitalWrite(LEDpin, LOW);      // flash LED
    emontx_sleep(3);											// sleep or delay in seconds - see emontx_lib
  }

}

//*********************************************************************************************************************

void calcVI3Ph(int cycles, int timeout)
{
	int SupplyVoltage = readVcc();
	int crossCount = 0;										// Used to measure number of times threshold is crossed.
	int numberOfSamples = 0;								// This is now incremented  
	int numberOfPowerSamples = 0;							// Needed because 1 cycle of voltages needs to be stored before use
	boolean lastVCross, checkVCross;						// Used to measure number of times threshold is crossed.
	double storedV[PHASE3];									// Array to store >120 degrees of voltage samples
	
	//-------------------------------------------------------------------------------------------------------------------------
	// 1) Waits for the waveform to be close to 'zero' (500 adc) part in sin curve.
	//-------------------------------------------------------------------------------------------------------------------------
	boolean st=false;										// an indicator to exit the while loop

	unsigned long start = millis();							// millis()-start makes sure it doesnt get stuck in the loop if there is an error.

	while(st==false)										// Wait for first zero crossing...
	{
		startV = analogRead(inPinV);						// using the voltage waveform
		if ((startV < 550) && (startV > 440)) st=true;		// check it's within range
		if ((millis()-start)>timeout) st = true;
	}
  
	//-------------------------------------------------------------------------------------------------------------------------
	// 2) Main measurment loop
	//------------------------------------------------------------------------------------------------------------------------- 
	start = millis(); 

	while ((crossCount < cycles * 2) && ((millis()-start)<timeout)) 
	{

		lastSampleV=sampleV;								// Used for digital high pass filter - offset removal
		lastSampleI1=sampleI1;
		lastSampleI2=sampleI2;
		lastSampleI3=sampleI3;
    
		lastFilteredV = filteredV;
		lastFilteredI1 = filteredI1;  
		lastFilteredI2 = filteredI2; 
		lastFilteredI3 = filteredI3;
    
		//-----------------------------------------------------------------------------
		// A) Read in raw voltage and current samples
		//-----------------------------------------------------------------------------
		sampleV = analogRead(inPinV);						// Read in raw voltage signal
		sampleI1 = analogRead(inPinI1);						// Read in raw current signal
		sampleI2 = analogRead(inPinI2);						// Read in raw current signal
		sampleI3 = analogRead(inPinI3);						// Read in raw current signal

		//-----------------------------------------------------------------------------
		// B) Apply digital high pass filters to remove 2.5V DC offset (to centre wave on 0).
		//-----------------------------------------------------------------------------
		filteredV = 0.996*(lastFilteredV+sampleV-lastSampleV);
		filteredI1 = 0.996*(lastFilteredI1+sampleI1-lastSampleI1);
		filteredI2 = 0.996*(lastFilteredI2+sampleI2-lastSampleI2);
		filteredI3 = 0.996*(lastFilteredI3+sampleI3-lastSampleI3);
   
		storedV[numberOfSamples%PHASE3] = filteredV;		// store this voltage sample in circular buffer

		if (crossCount > 2)									// one complete cycle has been stored, so can use delayed 
		{													//    voltage samples to calculate instantaneous powers
			//-----------------------------------------------------------------------------
			// C) Root-mean-square method voltage
			//-----------------------------------------------------------------------------  
			sqV= filteredV * filteredV;					        //1) square voltage values
			sumV += sqV;								        //2) sum

			//-----------------------------------------------------------------------------
			// D) Root-mean-square method current
			//-----------------------------------------------------------------------------   
			sqI1 = filteredI1 * filteredI1;				        //1) square current values
			sumI1 += sqI1;								        //2) sum 
			sqI2 = filteredI2 * filteredI2;
			sumI2 += sqI2;
			sqI3 = filteredI3 * filteredI3;
			sumI3 += sqI3;

			//-----------------------------------------------------------------------------
			// E) Phase calibration - for Phase 1: shifts V1 to correct transformer errors
			//	for phases 2 & 3 delays V1 by 120 degrees & 240 degrees respectively 
			//	and shifts for fine adjustment and to correct transformer errors.
			//-----------------------------------------------------------------------------
			phaseShiftedV1 = lastFilteredV + Phasecal1 * (filteredV - lastFilteredV);
			phaseShiftedV2 = storedV[(numberOfSamples-PHASE2-1)%PHASE3] 
				+ Phasecal2 * (storedV[(numberOfSamples-PHASE2)%PHASE3] 
							 - storedV[(numberOfSamples-PHASE2-1)%PHASE3]);
			phaseShiftedV3 = storedV[(numberOfSamples+1)%PHASE3] 
				+ Phasecal3 * (storedV[(numberOfSamples+2)%PHASE3]
							 - storedV[(numberOfSamples+1)%PHASE3]);
			
    
			//-----------------------------------------------------------------------------
			// F) Instantaneous power calc
			//-----------------------------------------------------------------------------   
			instP1 = phaseShiftedV1 * filteredI1;			//Instantaneous Power
			sumP1 +=instP1;									//Sum  
			instP2 = phaseShiftedV2 * filteredI2;
			sumP2 +=instP2;
			instP3 = phaseShiftedV3 * filteredI3;
			sumP3 +=instP3;
			
			numberOfPowerSamples++;							//Count number of times looped for Power averages.

		}
		
		//-----------------------------------------------------------------------------
		// G)  Find the number of times the voltage has crossed the initial voltage
		//    	- every 2 crosses we will have sampled 1 wavelength 
		//   	 - so this method allows us to sample an integer number of half wavelengths which increases accuracy
		//-----------------------------------------------------------------------------       
		lastVCross = checkVCross;                     
		if (sampleV > startV) 
			checkVCross = true; 
		else 
			checkVCross = false;
		if (numberOfSamples==1)
			lastVCross = checkVCross;                  
                    
		if (lastVCross != checkVCross)
			crossCount++;
		
		numberOfSamples++;									//Count number of times looped.
			
	}
 
	//-------------------------------------------------------------------------------------------------------------------------
	// 3) Post loop calculations
	//------------------------------------------------------------------------------------------------------------------------- 
	//Calculation of the root of the mean of the voltage and current squared (rms)
	//Calibration coefficients applied. 
  
	double V_Ratio = Vcal *((SupplyVoltage/1000.0) / 1023.0);
	Vrms = V_Ratio * sqrt(sumV / numberOfPowerSamples); 
  
	double I_Ratio1 = Ical1 *((SupplyVoltage/1000.0) / 1023.0);
	Irms1 = I_Ratio1 * sqrt(sumI1 / numberOfPowerSamples); 
	double I_Ratio2 = Ical2 *((SupplyVoltage/1000.0) / 1023.0);
	Irms2 = I_Ratio2 * sqrt(sumI2 / numberOfPowerSamples); 
	double I_Ratio3 = Ical3 *((SupplyVoltage/1000.0) / 1023.0);
	Irms3 = I_Ratio3 * sqrt(sumI3 / numberOfPowerSamples); 

	//Calculation power values
	realPower1 = V_Ratio * I_Ratio1 * sumP1 / numberOfPowerSamples;
	apparentPower1 = Vrms * Irms1;
	powerFactor1 = realPower1 / apparentPower1;

	realPower2 = V_Ratio * I_Ratio2 * sumP2 / numberOfPowerSamples;
	apparentPower2 = Vrms * Irms2;
	powerFactor2 = realPower2 / apparentPower2;

	realPower3 = V_Ratio * I_Ratio3 * sumP3 / numberOfPowerSamples;
	apparentPower3 = Vrms * Irms3;
	powerFactor3 = realPower3 / apparentPower3;

	//Reset accumulators
	sumV = 0;
	sumI1 = 0;
	sumI2 = 0;
	sumI3 = 0;
	sumP1 = 0;
	sumP2 = 0;
	sumP3 = 0;
	//--------------------------------------------------------------------------------------       

#ifdef DEBUGGING
	// Include these statements for development/debugging only
	
	Serial.print("Total Samples: "); Serial.print(numberOfSamples);
	Serial.print(" Power Samples: "); Serial.print(numberOfPowerSamples);
	Serial.print(" Time: "); Serial.print(millis() - start);
	Serial.print(" Crossings: "); Serial.println(crossCount);

	for (int j=0; j<PHASE3; j++)
	{
		Serial.print(storedV[j]); Serial.print(" ");
		Serial.println();
	}
#endif
}

//*********************************************************************************************************************

long readVcc() 
{
  long result;
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2);
  ADCSRA |= _BV(ADSC);
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1126400L / result;
  return result;
}
