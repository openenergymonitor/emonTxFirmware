/*
  EmonTx CT123 Voltage NanodeRF example
  
  Part of the openenergymonitor.org project
  Licence: GNU GPL V3
  
  Author: Trystan Lea
*/
#include <JeeLib.h>
#include "EmonLib.h"

int node_id = 0;

// Create  instances for each CT channel
EnergyMonitor ct1,ct2,ct3, ct4;

// On-board emonTx LED
const int LEDpin = 9;

//---------------------------------------------------------------------
// The PacketBuffer class is used to generate the json string that is send via ethernet - JeeLabs
//---------------------------------------------------------------------
class PacketBuffer : public Print {
public:
    PacketBuffer () : fill (0) {}
    const char* buffer() { return buf; }
    byte length() { return fill; }
    void reset()
    { 
      memset(buf,NULL,sizeof(buf));
      fill = 0; 
    }
    virtual size_t write (uint8_t ch)
        { if (fill < sizeof buf) buf[fill++] = ch; }
    byte fill;
    char buf[150];
    private:
};
PacketBuffer str;

#include <EtherCard.h>

// ethernet interface mac address, must be unique on the LAN
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };

byte Ethernet::buffer[500];
unsigned long timer = 0;

// 1) Set this to the domain name of your hosted emoncms - leave blank if posting to IP address 
char website[] PROGMEM = "emoncms.org";

// 2) If your emoncms install is in a subdirectory add details here i.e "/emoncms3"
char basedir[] = "";

// 3) Set to your account write apikey 
char apikey[] = "YOUR APIKEY";

void setup() 
{
  Serial.begin(9600);
  // while (!Serial) {}
  // wait for serial port to connect. Needed for Leonardo only
  
  Serial.println("emonTX Shield CT123 Voltage Serial Only example"); 
  Serial.println("OpenEnergyMonitor.org");
 
  delay(1000);
  
  if (ether.begin(sizeof Ethernet::buffer, mymac) == 0) 
    Serial.println( "Failed to access Ethernet controller");

  Serial.println("Setting up DHCP");
  if (!ether.dhcpSetup())
    Serial.println( "DHCP failed");
  
  ether.printIp("My IP: ", ether.myip);
  ether.printIp("Netmask: ", ether.mymask);
  ether.printIp("GW IP: ", ether.gwip);
  ether.printIp("DNS IP: ", ether.dnsip);

  // DNS Setup
  if (!ether.dnsLookup(website))
    Serial.println("DNS failed");
  ether.printIp("SRV: ", ether.hisip);
  
  delay(1000);
  
  // Calibration factor = CT ratio / burden resistance = (100A / 0.05A) / 33 Ohms = 60.606
  ct1.current(1, 60.606);
  ct2.current(2, 60.606);                                     
  ct3.current(3, 60.606);
  ct4.current(4, 60.606); 
  
  // (ADC input, calibration, phase_shift)
  ct1.voltage(0, 300.6, 1.7);                                
  ct2.voltage(0, 300.6, 1.7);                                
  ct3.voltage(0, 300.6, 1.7);
  ct4.voltage(0, 300.6, 1.7);
  
  // Setup indicator LED
  pinMode(LEDpin, OUTPUT);                                              
  digitalWrite(LEDpin, HIGH);  

}

void loop() 
{ 
  ether.packetLoop(ether.packetReceive());

  // 10000 = a post every 10 seconds.
  // its recommended to keep this figure at 10s or longer, 20, 30s
  // as otherwise the amount of data generated is large.
  // Post rate requirements may vary from application to application.
  if ((millis()-timer)>10000) {
    timer = millis();
    
    // Calculate all. No.of crossings, time-out 
    ct1.calcVI(20,2000);                                                  
    ct2.calcVI(20,2000);
    ct3.calcVI(20,2000);
    ct4.calcVI(20,2000);
    
    // Available properties: ct1.realPower, ct1.apparentPower, ct1.powerFactor, ct1.Irms and ct1.Vrms
    
    str.reset();
    str.print(basedir); str.print("/input/post.json?");
    str.print("apikey="); str.print(apikey);
    str.print("&node=");  str.print(node_id);
    str.print("&json={power1:"); str.print(ct1.realPower); str.print(",");
    str.print("power2:"); str.print(ct2.realPower); str.print(",");
    str.print("power3:"); str.print(ct3.realPower); str.print(",");
    str.print("power4:"); str.print(ct4.realPower); 
    str.print("}");
    str.print("\0");  //  End of json string
    
    Serial.print("Data sent: "); Serial.println(str.buf);
    
    // Send some test data to the server:
    ether.browseUrl(PSTR("") ,str.buf, website, 0);
  }
  
}
