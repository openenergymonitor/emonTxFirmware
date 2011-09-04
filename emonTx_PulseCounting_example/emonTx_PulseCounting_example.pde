/*
                          _____    
                         |_   _|   
  ___ _ __ ___   ___  _ __ | |_  __
 / _ \ '_ ` _ \ / _ \| '_ \| \ \/ /
|  __/ | | | | | (_) | | | | |>  < 
 \___|_| |_| |_|\___/|_| |_\_/_/\_\
 
//--------------------------------------------------------------------------------------
//Interrupt pulse counting - for interfacing with pulse output utility meters
//not low power - run emonTx from 5V USB adapter 

//Based on JeeLabs RF12 library http://jeelabs.org/2009/02/10/rfm12b-library-for-arduino/

// By Glyn Hudson and Trystan Lea: 4/9/11
// openenergymonitor.org
// GNU GPL V3


//--------------------------------------------------------------------------------------
*/

//JeeLabs libraries 
#include <Ports.h>
#include <RF12.h>
#include <avr/eeprom.h>
#include <util/crc16.h>  //cyclic redundancy check

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


//---------------------------------------------------------------------------------------------------
// Pulse counting settings 
//---------------------------------------------------------------------------------------------------

long pulseCount = 0;               //Number of pulses, used to measure energy.
unsigned long pulseTime,lastTime;  //Used to measure power.
double power, elapsedWh;          //power and energy
int ppwh = 1;                    ////1000 pulses/kwh = 1 pulse per wh - Number of pulses per wh - found or set on the meter.
//---------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// LED Indicator  
//--------------------------------------------------------------------------------------------------
# define LEDpin 9          //hardwired on emonTx PCB
//--------------------------------------------------------------------------------------------------

//########################################################################################################################
//Data Structure to be received 
//######################################################################################################################## 
typedef struct {
  	  int power;		// power value
          int elapsedWh;               //kwhr elapsed 
} Payload;
Payload emontx;
//########################################################################################################################

void setup()
{
  Serial.begin(115200);                   //fast serial 
   pinMode(LEDpin, OUTPUT);
  
  Serial.println("emonTx interrupt pulse counting example");
  Serial.println("openenergymonitor.org");
  
  
  //-----------------------------------------
  // RFM12B Initialize
  //------------------------------------------
  rf12_initialize(myNodeID,freq,network);   //Initialize RFM12 with settings defined above 
  //rf12_sleep(0);                             //Put the RFM12 to sleep - Note: This RF12 sleep interupt method might not be 100% reliable. Put RF to sleep: RFM12B module can be kept off while not used – saving roughly 15 mA
  //------------------------------------------
  
  
  Serial.print("Node: "); 
  Serial.print(myNodeID); 
  Serial.print(" Freq: "); 
   if (freq == RF12_433MHZ) Serial.print("433Mhz");
   if (freq == RF12_868MHZ) Serial.print("868Mhz");
   if (freq == RF12_915MHZ) Serial.print("915Mhz");  
  Serial.print(" Network: "); 
  Serial.println(network);
  
  attachInterrupt(1, onPulse, FALLING);    // KWH interrupt attached to IRQ 1  = pin3 - hardwired to emonTx pulse jackplug. For connections see: http://openenergymonitor.org/emon/node/208
}


void loop()
{

}


void onPulse()                  // The interrupt routine - ran each time a falling edge of a pulse is detected
{
digitalWrite(LEDpin, HIGH); //flash LED - very quickly  
  lastTime = pulseTime;        //used to measure time between pulses.
  pulseTime = micros();

  pulseCount++;                //pulseCounter               

  power = (3600000000.0 / (pulseTime - lastTime))/ppwh;  //Calculate power
  
  elapsedWh= (1.0*pulseCount/(ppwh));   // Find wh elapsed

  //Print the values.
  Serial.print(power,4);
  Serial.print(" ");
  Serial.println(elapsedWh,3);
  
  emontx.power=int(power);
  emontx.elapsedWh=int(elapsedWh);
    //--------------------------------------------------------------------------------------------------
    // 2. Send data via RF 
    //--------------------------------------------------------------------------------------------------
    while (!rf12_canSend())
    rf12_recvDone();
    rf12_sendStart(rf12_hdr, &emontx, sizeof emontx, RADIO_SYNC_MODE); 
    //--------------------------------------------------------------------------------------------------    
digitalWrite(LEDpin, LOW);
}


