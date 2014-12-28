/*
emonTx V3 CT1234 + 3-phase Voltage example

An example sketch for the emontx module for
3-phase electricity monitoring, with 4 current transformers 
and 1 only voltage transformer

Part of the openenergymonitor.org project
Licence: GNU GPL V3

Authors: Glyn Hudson, Trystan Lea
Builds upon JeeLabs RF12 library and Arduino
Extended for 3-phase operation: Robert Wall
V.1  7/11/2013    Derived from emonTx_CT123_3Phase_Voltage.ino 

emonTx V3 Shield documentation:http://openenergymonitor.org/emon/modules/emonTxV3
emonTx firmware code explanation: http://openenergymonitor.org/emon/modules/emontx/firmware
emonTx / emonTx Shield calibration instructions: http://openenergymonitor.org/emon/buildingblocks/calibration

REQUIRES in [Arduino]/libraries
Arduino.h
WProgram.h
avr/wdt.h                                       // the UNO bootloader 
JeeLib.h                                        // Download JeeLib:  //https://github.com/jcw/jeelib 

(does NOT require EmonLib)
=============================================================================================

Extended to allow the voltage measurement of a single phase to be used to generate approximate indications of
power (real and apparent) and phase angle for the other two phases of a 3-phase installation.

The measured voltage of one phase is delayed in an array and subsequently used in the calculations for the
remaining phases.
N.B. "Phase shifted" means a small adjustment to the voltage waveform to accommodate small ( < 10 degrees) phase shifts
in the transformers etc. "Delayed" means a delay of the voltage samples by approx 1/3 or 2/3 cycle.
 Without the 4th c.t. in use, this sketch records approx 24 sample sets per cycle at 50 Hz.

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
change "#define PHASE2 8" and/or "#define PHASE3 18"

The fourth channel may be used, for example, for a PV input. This must be on Phase 1 (meaning the same phase as the CT 
connected to Input 1).
Include the line " #define CT4 " if the fourth C.T. is to be used.

*/
// #define DEBUGGING                       // enable this line to include debugging print statements
#define SERIALPRINT                         // include print statement for commissioning - comment this line to exclude
//#define CT4                                     // uncomment this line if you are using the 4th c.t.
                                                            // The timing values "PHASE2", "PHASE3", "Phasecal2" & "Phasecal3" will be different 
                                                            // depending on whether CT 4 is used or not.
                                                           

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#define RF69_COMPAT 0                            // Set to 1 if using RFM69CW or 0 is using RFM12B

#define RF_freq RF12_433MHZ                      // Frequency of RF12B module can be 
                                                 //    RF12_433MHZ, RF12_868MHZ or RF12_915MHZ. 
                                                 //  You should use the one matching the module you have.
#define FILTERSETTLETIME 5000                    //  Time (ms) to allow the filters to settle before sending data

#define PHASE2 8                                 //  Number of samples delay for L2
#define PHASE3 18                                //  Number  of samples delay for L3, also size of array
                                                 //  These can be adjusted if the phase correction is not adequate
                                                 //  Suggested starting values for 4 ct's:
                                                 //    PHASE2  6
                                                 //    PHASE3 15
                                                 //    Phasecal2 = 0.57
                                                 //    Phasecal3 = 0.97

const int nodeID = 10;                           //  emonTx RFM12B node ID
const int networkGroup = 210;                    //  emonTx RFM12B wireless network group
                                                 //  - needs to be same as emonBase and emonGLCD needs to be same
                                                 //    as emonBase and emonGLCD

const int UNO = 1;                               // Set to 0 if you are not using the UNO bootloader 
                                                 // (i.e using Duemilanove) - All Atmega's shipped from
                                                 // OpenEnergyMonitor come with Arduino Uno bootloader

const byte TIME_BETWEEN_READINGS= 10;			 //Time between readings   

												 
#include <avr/wdt.h>                             // the UNO bootloader 

#include <JeeLib.h>                              //https://github.com/jcw/jeelib - Tested with JeeLib 3/11/14
ISR(WDT_vect) { Sleepy::watchdogEvent(); }


//Set Voltage and current input pins
int inPinV = 0;
int inPinI1 = 1;
int inPinI2 = 2;
int inPinI3 = 3;
int inPinI4 = 4;
//Calibration coefficients
//These need to be set in order to obtain accurate results
double Vcal = 276.9;                            // Calibration constant for voltage input
double Ical1 = 90.9;                            // Calibration constant for current transformer 1
double Ical2 = 90.9;                            // Calibration constant for current transformer 2
double Ical3 = 90.9;                            // Calibration constant for current transformer 3
double Ical4 = 16.6;                            // Calibration constant for current transformer 4

double Phasecal1 = 1.00;                         // Calibration constant for phase shift L1
double Phasecal2 = 1.35;                         // Calibration constant for phase shift L2
double Phasecal3 = 1.37;                         // Calibration constant for phase shift L3
double Phasecal4 = 1.00;                         // Calibration constant for phase shift CT 4


//--------------------------------------------------------------------------------------
// Variable declaration for filters, phase shift, voltages, currents & powers
//--------------------------------------------------------------------------------------
double realPower1,                               // The final data
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
realPower4,
apparentPower4,
powerFactor4,
Irms4,
Vrms;       

    
typedef struct { int power1, power2, power3, power4, Vrms; } PayloadTX;        // neat way of packaging data for RF comms
                                                 // (Include all the variables that are desired,
                                                 // ensure the same struct is used to receive)

PayloadTX emontx;                                // create an instance

const int LEDpin = 6;                            // On-board emonTx LED 

boolean settled = false;

void setup() 
{
Serial.begin(9600);
Serial.println("emonTx V3 CT1234 Voltage 3 Phase example");
Serial.println("OpenEnergyMonitor.org");
Serial.print("Node: "); 
Serial.print(nodeID); 
Serial.print(" Freq: "); 
if (RF_freq == RF12_433MHZ) Serial.print("433Mhz");
else if (RF_freq == RF12_868MHZ) Serial.print("868Mhz");
else if (RF_freq == RF12_915MHZ) Serial.print("915Mhz"); 
else Serial.print("Not set");
Serial.print(" Network: "); 
Serial.println(networkGroup);


rf12_initialize(nodeID, RF_freq, networkGroup);  // initialize RF
rf12_sleep(RF12_SLEEP);

pinMode(LEDpin, OUTPUT);                         // Setup indicator LED
digitalWrite(LEDpin, HIGH);

}

//*********************************************************************************************************************
void loop() 
{ 
// Outer loop - Reads Voltages & Currents - Sends results
calcVI3Ph(11,2000);                              // Calculate all. No.of complete cycles, time-out  

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
#ifdef CT4
Serial.print(" Current 4: "); Serial.print(Irms4);
Serial.print(" Power 4: "); Serial.print(realPower4);
Serial.print(" VA 4: "); Serial.print(apparentPower4);
Serial.print(" PF 4: "); Serial.println(powerFactor4);
#endif

Serial.println(); delay(100);

#endif 

emontx.power1 = realPower1;                      // Copy the desired variables ready for transmision
emontx.power2 = realPower2;
emontx.power3 = realPower3;
emontx.power4 = realPower4;
emontx.Vrms   = Vrms;

if (!settled && millis() > FILTERSETTLETIME)     // because millis() returns to zero after 50 days ! 
    settled = true;
    
if (settled)                                     // send data only after filters have settled
{  
    send_rf_data();                              // *SEND RF DATA* - see emontx_lib
    digitalWrite(LEDpin, HIGH); delay(2); digitalWrite(LEDpin, LOW);      // flash LED
    delay(TIME_BETWEEN_READINGS*1000);  
}

}

//*********************************************************************************************************************

void calcVI3Ph(int cycles, int timeout)
{
    //--------------------------------------------------------------------------------------
    // Variable declaration for filters, phase shift, voltages, currents & powers
    //--------------------------------------------------------------------------------------

    static int lastSampleV,sampleV;                         // 'sample' holds the raw analog read value, 'lastSample' holds the last sample
    static int lastSampleI1,sampleI1;
    static int lastSampleI2,sampleI2;
    static int lastSampleI3,sampleI3;
    static int lastSampleI4,sampleI4;


    static double lastFilteredV,filteredV;      // 'Filtered' is the raw analog value minus the DC offset
    static double lastFilteredI1, filteredI1;
    static double lastFilteredI2, filteredI2;
    static double lastFilteredI3, filteredI3;
    static double lastFilteredI4, filteredI4;

    double phaseShiftedV1;                      // Holds the calibrated delayed & phase shifted voltage.
    double phaseShiftedV2;
    double phaseShiftedV3;
    double phaseShiftedV4;

    double sumV,sumI1,sumI2,sumI3,sumI4;
    double sumP1,sumP2,sumP3,sumP4;             // sq = squared, sum = Sum, inst = instantaneous



    int startV;                                                     // Instantaneous voltage at start of sample window.

    int SupplyVoltage = 3300;                        //Hardcode supply voltage for emonTx V3, it should be always 3.3V
    int crossCount = -2;                                   // Used to measure number of times threshold is crossed.
    int numberOfSamples = 0;                         // This is now incremented  
    int numberOfPowerSamples = 0;                // Needed because 1 cycle of voltages needs to be stored before use
    boolean lastVCross, checkVCross;             // Used to measure number of times threshold is crossed.
    double storedV[PHASE3];                          // Array to store >240 degrees of voltage samples
    
    //-------------------------------------------------------------------------------------------------------------------------
    // 1) Waits for the waveform to be close to 'zero' (500 adc) part in sin curve.
    //-------------------------------------------------------------------------------------------------------------------------
    boolean st=false;                           // an indicator to exit the while loop

    unsigned long start = millis();             // millis()-start makes sure it doesnt get stuck in the loop if there is an error.

    while(st==false)                            // Wait for first zero crossing...
    {
        startV = analogRead(inPinV);            // using the voltage waveform
        if ((startV < 550) && (startV > 440)) st=true;        // check it's within range
        if ((millis()-start)>timeout) st = true;
    }

    //-------------------------------------------------------------------------------------------------------------------------
    // 2) Main measurment loop
    //------------------------------------------------------------------------------------------------------------------------- 
    start = millis(); 

    while ((crossCount < cycles * 2) && ((millis()-start)<timeout)) 
    {
        lastSampleV=sampleV;                    // Used for digital high pass filter - offset removal
        lastSampleI1=sampleI1;
        lastSampleI2=sampleI2;
        lastSampleI3=sampleI3;
        lastSampleI4=sampleI4;

        lastFilteredV = filteredV;
        lastFilteredI1 = filteredI1;  
        lastFilteredI2 = filteredI2; 
        lastFilteredI3 = filteredI3;
        lastFilteredI4 = filteredI4;

        //-----------------------------------------------------------------------------
        // A) Read in raw voltage and current samples
        //-----------------------------------------------------------------------------
        sampleV = analogRead(inPinV);           // Read in raw voltage signal
        sampleI1 = analogRead(inPinI1);         // Read in raw current signal
        sampleI2 = analogRead(inPinI2);         // Read in raw current signal
        sampleI3 = analogRead(inPinI3);         // Read in raw current signal
 #ifdef CT4
       sampleI4 = analogRead(inPinI4);          // Read in raw current signal
#endif
        //-----------------------------------------------------------------------------
        // B) Apply digital high pass filters to remove 2.5V DC offset (to centre wave on 0).
        //-----------------------------------------------------------------------------
        filteredV = 0.996*(lastFilteredV+(sampleV-lastSampleV));
        filteredI1 = 0.996*(lastFilteredI1+(sampleI1-lastSampleI1));
        filteredI2 = 0.996*(lastFilteredI2+(sampleI2-lastSampleI2));
        filteredI3 = 0.996*(lastFilteredI3+(sampleI3-lastSampleI3));
#ifdef CT4
        filteredI4 = 0.996*(lastFilteredI4+(sampleI4-lastSampleI4));
#endif

        storedV[numberOfSamples%PHASE3] = filteredV;        // store this voltage sample in circular buffer

        //-----------------------------------------------------------------------------

        // C)  Find the number of times the voltage has crossed the initial voltage
        //        - every 2 crosses we will have sampled 1 wavelength 
        //        - so this method allows us to sample an integer number of half wavelengths which increases accuracy
        //-----------------------------------------------------------------------------       


        lastVCross = checkVCross;                     

        checkVCross = (sampleV > startV)? true : false;
        if (numberOfSamples==1)
            lastVCross = checkVCross;                  
                    
        if (lastVCross != checkVCross)
        {
            crossCount++;
            if (crossCount == 0)              // Started recording at -2 crossings so that one complete cycle has been stored before accumulating.
            {
                sumV  = 0;
                sumI1 = 0;
                sumI2 = 0;
                sumI3 = 0;
                sumI4 = 0;
                sumP1 = 0;                                    
                sumP2 = 0;
                sumP3 = 0;
                sumP4 = 0;
                numberOfPowerSamples = 0;
            }
        }

        //-----------------------------------------------------------------------------
        // D) Root-mean-square method voltage
        //-----------------------------------------------------------------------------  
        sumV += filteredV * filteredV;          // sum += square voltage values

        //-----------------------------------------------------------------------------
        // E) Root-mean-square method current
        //-----------------------------------------------------------------------------   
        sumI1 += filteredI1 * filteredI1;       // sum += square current values
        sumI2 += filteredI2 * filteredI2;
        sumI3 += filteredI3 * filteredI3;
#ifdef CT4
        sumI4 += filteredI4 * filteredI4;
#endif

        //-----------------------------------------------------------------------------
        // F) Phase calibration - for Phase 1: shifts V1 to correct transformer errors
        //    for phases 2 & 3 delays V1 by 120 degrees & 240 degrees respectively 
        //    and shifts for fine adjustment and to correct transformer errors.
        //-----------------------------------------------------------------------------
        phaseShiftedV1 = lastFilteredV + Phasecal1 * (filteredV - lastFilteredV);
        phaseShiftedV2 = storedV[(numberOfSamples-PHASE2-1)%PHASE3] 
            + Phasecal2 * (storedV[(numberOfSamples-PHASE2)%PHASE3] 
                        - storedV[(numberOfSamples-PHASE2-1)%PHASE3]);
        phaseShiftedV3 = storedV[(numberOfSamples+1)%PHASE3] 
            + Phasecal3 * (storedV[(numberOfSamples+2)%PHASE3]
                        - storedV[(numberOfSamples+1)%PHASE3]);
        

        //-----------------------------------------------------------------------------
        // G) Instantaneous power calc
        //-----------------------------------------------------------------------------   
        sumP1 += phaseShiftedV1 * filteredI1;   // Sum  += Instantaneous Power
        sumP2 += phaseShiftedV2 * filteredI2;
        sumP3 += phaseShiftedV3 * filteredI3;
#ifdef CT4
        sumP4 += phaseShiftedV1 * filteredI4;
#endif
        
        numberOfPowerSamples++;                  // Count number of times looped for Power averages.
        numberOfSamples++;                      //Count number of times looped.    

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
#ifdef CT4
    double I_Ratio4 = Ical4 *((SupplyVoltage/1000.0) / 1023.0);
    Irms4 = I_Ratio4 * sqrt(sumI4 / numberOfPowerSamples); 
#endif

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

#ifdef CT4
    realPower4 = V_Ratio * I_Ratio4 * sumP4 / numberOfPowerSamples;
    apparentPower4 = Vrms * Irms4;
    powerFactor4 = realPower4 / apparentPower4;
#else
    realPower4 = 0.0;
    apparentPower4 = 0.0;
    powerFactor4 = 0.0;
#endif

    //Reset accumulators
    sumV = 0;
    sumI1 = 0;
    sumI2 = 0;
    sumI3 = 0;
    sumI4 = 0;
    sumP1 = 0;
    sumP2 = 0;
    sumP3 = 0;
    sumP4 = 0;
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

//*********************************************************************************************************************

void send_rf_data()
{
  rf12_sleep(RF12_WAKEUP);                                   
  rf12_sendNow(0, &emontx, sizeof emontx);                           //send temperature data via RFM12B using new rf12_sendNow wrapper
  rf12_sendWait(2);
}

//*********************************************************************************************************************

void emontx_sleep(int seconds) {
  Sleepy::loseSomeTime(seconds*1000);
}
