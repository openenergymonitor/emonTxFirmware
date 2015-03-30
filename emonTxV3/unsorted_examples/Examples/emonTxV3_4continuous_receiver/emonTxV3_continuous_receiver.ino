//
//  A simple sketch for displaying the content of RF messages.  Written for use with
//  my emonTxV3_4chan.ino sketch which provides continuous monitoring of real power for
//  four channels.  The on-board LED is controlled by the LSB of the received msgNumber 
//  field.
//
//                  Robin Emley (calypso_rae on Open Energy Monitor Forum)
//                  Sept 2013
//

#define RF69_COMPAT 0                                                              // Set to 1 if using RFM69CW or 0 is using RFM12B
#include <JeeLib.h>      // Download JeeLib: http://github.com/jcw/jeelib
#define RF_freq RF12_868MHZ // Use the RF_freq to match the module you have.

const int TXnodeID = 10;
const int myNode = 15;
const int networkGroup = 210; 
const int UNO = 1; // Set to 0 if you're not using the UNO bootloader 

typedef struct { 
int msgNumber;
int power_CT1;
int power_CT2;
int power_CT3;
int power_CT4;
} Rx_struct;    //  revised data for RF comms
Rx_struct receivedData;

int lastMsgNumber = 0;

const byte onBoard_LEDpin = 9; // active high (duplicates the LED at pin 6)



void setup() 
{
  Serial.begin(9600);
  delay(5000);
  Serial.println("Receiver for emonTx V3 trials");
  Serial.print("Node: "); 
  Serial.print(myNode); 
  Serial.print(" Freq: "); 
  if (RF_freq == RF12_433MHZ) Serial.print("433Mhz");
  if (RF_freq == RF12_868MHZ) Serial.print("868Mhz");
  if (RF_freq == RF12_915MHZ) Serial.print("915Mhz"); 
  Serial.print(" Network: "); 
  Serial.println(networkGroup);
  delay(1000);
  rf12_set_cs(10); //emonTx, emonGLCD, NanodeRF, JeeNode

  rf12_initialize(myNode, RF_freq, networkGroup);  

  pinMode(onBoard_LEDpin, OUTPUT);  
}

void loop() 
{ 
  unsigned long timeNow = millis();
  
  if (rf12_recvDone())
  {      
    if (rf12_crc == 0 && (rf12_hdr & RF12_HDR_CTL) == 0)
    {
      int node_id = (rf12_hdr & 0x1F);
      byte n = rf12_len;
    
      if (node_id == TXnodeID)
      {
        receivedData = *(Rx_struct*) rf12_data;

        int msgNumber = receivedData.msgNumber;
        int power_CT1 = receivedData.power_CT1;
        int power_CT2 = receivedData.power_CT2;
        int power_CT3 = receivedData.power_CT3;
        int power_CT4 = receivedData.power_CT4;
        
	if (receivedData.msgNumber & 0x01 == 1) {
	  digitalWrite(onBoard_LEDpin, HIGH); }
	else {
	  digitalWrite(onBoard_LEDpin, LOW);  }
       

	Serial.print(msgNumber);
        Serial.print(", ");
        Serial.print(power_CT1);
        Serial.print(", ");
        Serial.print(power_CT2);
        Serial.print(", ");
        Serial.print(power_CT3);
        Serial.print(", ");
        Serial.println(power_CT4);

        if (msgNumber != lastMsgNumber + 1)
        {
          Serial.println("Message numbering error!");
        }
        
        lastMsgNumber = msgNumber;
	}	
      }	
      else
      {
        Serial.println("Corrupt message!");
      }
   }
}
