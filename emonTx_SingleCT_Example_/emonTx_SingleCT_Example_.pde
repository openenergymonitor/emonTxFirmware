/*
                          _____    
                         |_   _|   
  ___ _ __ ___   ___  _ __ | |_  __
 / _ \ '_ ` _ \ / _ \| '_ \| \ \/ /
|  __/ | | | | | (_) | | | | |>  < 
 \___|_| |_| |_|\___/|_| |_\_/_/\_\
 
//--------------------------------------------------------------------------------------
//Single CT wireless node example 

//Based on JeeLabs RF12 library http://jeelabs.org/2009/02/10/rfm12b-library-for-arduino/

// By Glyn Hudson and Trystan Lea: 21/9/11
// openenergymonitor.org
// GNU GPL V3

//using CT port 2 - middle jackplug

//--------------------------------------------------------------------------------------
*/



//JeeLabs libraries 
#include <Ports.h>
#include <RF12.h>
#include <avr/eeprom.h>
#include <util/crc16.h>  //cyclic redundancy check

ISR(WDT_vect) { Sleepy::watchdogEvent(); } 	 // interrupt handler: has to be defined because we're using the watchdog for low-power waiting

//---------------------------------------------------------------------------------------------------
// Serial print settings - disable all serial prints if SERIAL 0 - increases long term stability 
//---------------------------------------------------------------------------------------------------
#define SERIAL 0
//---------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------
// RF12 settings 
//---------------------------------------------------------------------------------------------------
// fixed RF12 settings

#define myNodeID 10         //in the range 1-30
#define network     210      //default network group (can be in the range 1-250). All nodes required to communicate together must be on the same network group
#define freq RF12_433MHZ     //Frequency of RF12B module can be RF12_433MHZ, RF12_868MHZ or RF12_915MHZ. You should use the one matching the module you have.

// set the sync mode to 2 if the fuses are still the Arduino default
// mode 3 (full powerdown) can only be used with 258 CK startup fuses
#define RADIO_SYNC_MODE 2

#define COLLECT 0x20 // collect mode, i.e. pass incoming without sending acks
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// CT energy monitor setup definitions 
//--------------------------------------------------------------------------------------------------
int CT_INPUT_PIN =          0;    //I/O analogue 3 = emonTx CT2 channel. Change to analogue 0 for emonTx CT1 chnnel  
int NUMBER_OF_SAMPLES =     1480; //The period (one wavelength) of mains 50Hz is 20ms. Each samples was measured to take 0.188ms. This meas that 106.4 samples/wavelength are possible. 1480 samples takes 280.14ms which is 14 wavelengths. 
int RMS_VOLTAGE =           240;  //Assumed supply voltage (230V in UK).  Tolerance: +10%-6%
int CT_BURDEN_RESISTOR =    15;   //value in ohms of burden resistor R3 and R6
int CT_TURNS =              1500; //number of turns in CT sensor. 1500 is the vaue of the efergy CT 

double CAL=1.295000139;          //*calibration coefficient* IMPORTANT - each monitor must be calibrated for maximum accuracy. See step 4 http://openenergymonitor.org/emon/node/58. Set to 1.295 for Seedstudio 100A current output CT (included in emonTx V2.0 kit)
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// LED Indicator  
//--------------------------------------------------------------------------------------------------
# define LEDpin 9          //hardwired on emonTx PCB
//--------------------------------------------------------------------------------------------------

//########################################################################################################################
//Data Structure to be sent
//######################################################################################################################## 
typedef struct {
  	  int ct1;		// current transformer 1
	  int supplyV;		// emontx voltage
} Payload;
Payload emontx;
//########################################################################################################################

//********************************************************************
//SETUP
//********************************************************************
void setup() {
  Serial.begin(9600);
  pinMode(LEDpin, OUTPUT);
  digitalWrite(LEDpin, HIGH);    //turn on LED 
  
  Serial.println("emonTx single CT example");
  Serial.println("openenergymonitor.org");
  
  delay(10);                             
  
  //-----------------------------------------
  // RFM12B Initialize
  //------------------------------------------
  rf12_initialize(myNodeID,freq,network);   //Initialize RFM12 with settings defined above 
  rf12_sleep(RF12_SLEEP);                             //Put the RFM12 to sleep - Note: This RF12 sleep interupt method might not be 100% reliable. Put RF to sleep: RFM12B module can be kept off while not used – saving roughly 15 mA
  //------------------------------------------
  delay(10);
  Sleepy::loseSomeTime(3000);                //wait 3s for power to settle 
  
  Serial.print("Node: "); 
  Serial.print(myNodeID); 
  Serial.print(" Freq: "); 
   if (freq == RF12_433MHZ) Serial.print("433Mhz");
   if (freq == RF12_868MHZ) Serial.print("868Mhz");
   if (freq == RF12_915MHZ) Serial.print("915Mhz");  
  Serial.print(" Network: "); 
  Serial.println(network);
  
  if (SERIAL==0) {
    Serial.println("serial disabled"); 
    Serial.end();
  }
  
  digitalWrite(LEDpin, LOW);              //turn off LED
}

//********************************************************************
//LOOP
//********************************************************************
void loop() {
    
    
    //--------------------------------------------------------------------------------------------------
    // 1. Read current supply voltage and get current CT energy monitoring reading 
    //--------------------------------------------------------------------------------------------------
          emontx.supplyV = readVcc();  //read emontx supply voltage
          emontx.ct1=int(emon( CT_INPUT_PIN, CAL, RMS_VOLTAGE, NUMBER_OF_SAMPLES, CT_BURDEN_RESISTOR, CT_TURNS, emontx.supplyV));
    //--------------------------------------------------------------------------------------------------
      
       
    //--------------------------------------------------------------------------------------------------
    // 2. Send data via RF 
    //--------------------------------------------------------------------------------------------------
           rfwrite() ;
    //--------------------------------------------------------------------------------------------------    
	
	
  
  //for debugging 
  if (SERIAL==1)
    Serial.println(emontx.ct1); 
   
  
  digitalWrite(LEDpin, HIGH);    //flash LED - very quickly 
  delay(2);                     // Needed to make sure print is finished before going to sleep
  digitalWrite(LEDpin, LOW); 


//sleep controll code  
//Uses JeeLabs power save function: enter low power mode and update Arduino millis 
//only be used with time ranges of 16..65000 milliseconds, and is not as accurate as when running normally.http://jeelabs.org/2010/10/18/tracking-time-in-your-sleep/

if ( (emontx.supplyV) > 3300 ) //if emonTx is powered by 5V usb power supply (going through 3.3V voltage reg) then don't go to sleep
  delay(10000); //10s
else
  if ( (emontx.supplyV) < 2700)  //if battery voltage drops below 2.7V then enter battery conservation mode (sleep for 60s in between readings) (need to fine tune this value) 
    Sleepy::loseSomeTime(60000);
    else
      Sleepy::loseSomeTime(10000); //10s


    




}
//********************************************************************


//--------------------------------------------------------------------------------------------------
// Send payload data via RF - see http://jeelabs.net/projects/cafe/wiki/RF12 for RF12 library documentation 
//--------------------------------------------------------------------------------------------------
static void rfwrite(){
    rf12_sleep(RF12_WAKEUP);     //wake up RF module
    int ec = 0;
    while (!rf12_canSend())
    {
      rf12_recvDone();
      ec++;
      if (ec>1000) break;
    }
    rf12_sendStart(0, &emontx, sizeof emontx); 
    //rf12_sendStart(rf12_hdr, &emontx, sizeof emontx, RADIO_SYNC_MODE); -- includes header data 
    rf12_sendWait(2);    //wait for RF to finish sending while in standby mode
    rf12_sleep(RF12_SLEEP);    //put RF module to sleep
}
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Read current emonTx battery voltage - not main supplyV!
//--------------------------------------------------------------------------------------------------
long readVcc() {
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH<<8;
  result = 1126400L / result; // Back-calculate AVcc in mV
  return result;
}
//--------------------------------------------------------------------------------------------------

