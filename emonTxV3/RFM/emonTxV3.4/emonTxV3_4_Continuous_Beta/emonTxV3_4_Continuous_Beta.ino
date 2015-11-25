//
// EmonTx v3.4 Continuous sampling Beta
//

#define firmware_version "v0.1"
#define emonTxV3
#define RF69_COMPAT 1                                                 // Set to 1 if using RFM69CW or 0 is using RFM12B

#include <JeeLib.h>
#include <EmonLibCM.h>
#include <OneWire.h>
#include <DallasTemperature.h>

ISR(WDT_vect) { Sleepy::watchdogEvent(); }


//----------------------------emonTx V3 Settings---------------------------------------------------------------------------------------------------------------
const byte Vrms                  = 230;                              // Vrms for apparent power readings (when no AC-AC voltage sample is present)
const double TIME_BETWEEN_READINGS = 9.8;                               // Time between readings   

//http://openenergymonitor.org/emon/buildingblocks/calibration

const float Ical1=                90.9;                              // (2000 turns / 22 Ohm burden) = 90.9
const float Ical2=                90.9;                              // (2000 turns / 22 Ohm burden) = 90.9
const float Ical3=                90.9;                              // (2000 turns / 22 Ohm burden) = 90.9
const float Ical4=                16.67;                             // (2000 turns / 120 Ohm burden) = 16.67
float Vcal=                       260.4;                            // (230V x 13) / (9V x 1.2) = 276.9 Calibration for UK AC-AC adapter 77DB-06-09 

//float Vcal=276.9;
//const float Vcal=               260;                               // Calibration for EU AC-AC adapter 77DE-06-09 
const float Vcal_USA=             130.0;                             // Calibration for US AC-AC adapter 77DA-10-09
boolean USA=FALSE; 

const byte min_pulsewidth= 110;                                      // minimum width of interrupt pulse (default pulse output meters = 100ms)
const int TEMPERATURE_PRECISION=  11;                                // 9 (93.8ms),10 (187.5ms) ,11 (375ms) or 12 (750ms) bits equal to resplution of 0.5C, 0.25C, 0.125C and 0.0625C
const byte MaxOnewire=             6;                            
#define ASYNC_DELAY 375                                              // DS18B20 conversion delay - 9bit requres 95ms, 10bit 187ms, 11bit 375ms and 12bit resolution takes 750ms
//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------


//----------------------------emonTx V3 hard-wired connections--------------------------------------------------------------------------------------------------------------- 
const byte LEDpin=                 6;                              // emonTx V3 LED
const byte DS18B20_PWR=            19;                             // DS18B20 Power
const byte DIP_switch1=            8;                              // Voltage selection 230 / 110 V AC (default switch off 230V)  - switch off D8 is HIGH from internal pullup
const byte DIP_switch2=            9;                              // RF node ID (default no chance in node ID, switch on for nodeID -1) switch off D9 is HIGH from internal pullup
const byte battery_voltage_pin=    7;                              // Battery Voltage sample from 3 x AA
const byte pulse_countINT=         1;                              // INT 1 / Dig 3 Terminal Block / RJ45 Pulse counting pin(emonTx V3.4) - (INT0 / Dig2 emonTx V3.2)
const byte pulse_count_pin=        3;                              // INT 1 / Dig 3 Terminal Block / RJ45 Pulse counting pin(emonTx V3.4) - (INT0 / Dig2 emonTx V3.2)
#define ONE_WIRE_BUS               5                               // DS18B20 Data                     
//-------------------------------------------------------------------------------------------------------------------------------------------

//Setup DS128B20
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
byte allAddress [MaxOnewire][8];  // 8 bytes per address
byte numSensors;
//-------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------RFM12B / RFM69CW SETTINGS----------------------------------------------------------------------------------------------------
#define RF_freq RF12_433MHZ                                              // Frequency of RF69CW module can be RF12_433MHZ, RF12_868MHZ or RF12_915MHZ. You should use the one matching the module you have.
byte nodeID = 8;                                                        // emonTx RFM12B node ID
const int networkGroup = 210;

#define RETRY_PERIOD    50  // how soon to retry if ACK didn't come in
#define RETRY_LIMIT     5   // maximum number of times to retry
#define ACK_TIME        10  // number of milliseconds to wait for an ack
#define RADIO_SYNC_MODE 2

typedef struct { 
  int power1, power2, power3, power4, Vrms, temp[MaxOnewire]; 
  unsigned long pulseCount;
} PayloadTX;

PayloadTX emontx;

//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------

//Random Variables 
//boolean settled = false;
boolean CT1, CT2, CT3, CT4, debug, DS18B20_STATUS; 
volatile byte pulseCount = 0;
unsigned long pulsetime=0;
// Record time of interrupt pulse
unsigned long lastsent = 0;
void setup()
{ 
  pinMode(LEDpin, OUTPUT);
  digitalWrite(LEDpin,HIGH);
  
  pinMode(pulse_count_pin, INPUT_PULLUP);                     // Set emonTx V3.4 interrupt pulse counting pin as input (Dig 3 / INT1)
  emontx.pulseCount=0;                                        // Make sure pulse count starts at zero

  Serial.begin(9600);
 
  Serial.print("emonTx V3.4 Continuous Beta "); Serial.print(firmware_version);
  #if (RF69_COMPAT)
    Serial.println(" RFM69CW");
  #else
    Serial.println(" RFM12B");
  #endif
  Serial.println("OpenEnergyMonitor.org");
  
  // READ DIP SWITCH POSITIONS 
  pinMode(DIP_switch1, INPUT_PULLUP);
  pinMode(DIP_switch2, INPUT_PULLUP);
  if (digitalRead(DIP_switch1)==LOW) nodeID--;                            // IF DIP switch 1 is switched on then subtract 1 from nodeID
  if (digitalRead(DIP_switch2)==LOW) USA=TRUE;                            // IF DIP switch 2 is switched on then activate USA mode
  
  if (USA==TRUE){                                                         // if USA mode is true
    Vcal=Vcal_USA;                                                        // Assume USA AC/AC adatper is being used, set calibration accordingly 
  } 
  
  delay(10);
  rf12_initialize(nodeID, RF_freq, networkGroup);                         // initialize RFM12B/rfm69CW
  for (int i=10; i>=0; i--)                                               // Send RF test sequence (for factory testing)
  {
    emontx.power1=i; 
    rf12_sendNow(0, &emontx, sizeof emontx);
    delay(100);
  }
  rf12_sendWait(2);
  emontx.power1=0;

  digitalWrite(LEDpin,LOW); 
  
  //################################################################################################################################
  //Setup and for presence of DS18B20
  //################################################################################################################################
  pinMode(DS18B20_PWR, OUTPUT);
  digitalWrite(DS18B20_PWR, HIGH); delay(100); 
  sensors.begin();
  sensors.setWaitForConversion(false);             // disable automatic temperature conversion to reduce time spent awake, conversion will be implemented manually in sleeping 
                                                   // http://harizanov.com/2013/07/optimizing-ds18b20-code-for-low-power-applications/ 
  numSensors=(sensors.getDeviceCount());
  if (numSensors > MaxOnewire) numSensors=MaxOnewire;              // Limit number of sensors to max number of sensors 
  for (byte j=0; j<numSensors; j++) {
    oneWire.search(allAddress[j]);                                 // Fetch addresses
    sensors.setResolution(allAddress[j], TEMPERATURE_PRECISION);   // and set the a to d conversion resolution of each.
  }
  
  delay(500);
  digitalWrite(DS18B20_PWR, LOW);
  if (numSensors==0) DS18B20_STATUS=0; else DS18B20_STATUS=1;

  //################################################################################################################################

  if (Serial) debug = 1; else debug=0;          // if serial UART to USB is connected show debug O/P. If not then disable serial
  if (debug==1)
  {
    Serial.print("CT 1 Cal "); Serial.println(Ical1);
    Serial.print("CT 2 Cal "); Serial.println(Ical2);
    Serial.print("CT 3 Cal "); Serial.println(Ical3);
    Serial.print("CT 4 Cal "); Serial.println(Ical4);
    delay(1000);
        
    if (DS18B20_STATUS==1) {
      Serial.print("Detected Temp Sensors:  "); 
      Serial.println(numSensors);
    } else { 
      Serial.println("No temperature sensor");
    }
    
    Serial.print("Node: "); Serial.print(nodeID); 
    Serial.print(" Freq: "); 
    if (RF_freq == RF12_433MHZ) Serial.print("433Mhz");
    if (RF_freq == RF12_868MHZ) Serial.print("868Mhz");
    if (RF_freq == RF12_915MHZ) Serial.print("915Mhz"); 
    Serial.print(" Network: "); Serial.println(networkGroup);

    Serial.print("CT1 CT2 CT3 CT4 VRMS/BATT PULSE");
    if (DS18B20_STATUS==1){Serial.print(" Temperature 1-"); Serial.print(numSensors);}
    Serial.println(" "); 
    delay(500);  

  }
  else 
  { 
    Serial.end();
  }
  
  EmonLibCM_number_of_channels(4);       // number of current channels
  EmonLibCM_cycles_per_second(50);       // frequency 50Hz, 60Hz
  EmonLibCM_datalog_period(TIME_BETWEEN_READINGS);          // period of readings in seconds
  
  EmonLibCM_min_startup_cycles(10);      // number of cycles to let ADC run before starting first actual measurement
                                         // larger value improves stability if operated in stop->sleep->start mode

  EmonLibCM_voltageCal(Vcal*(3.3/1023));            // 260.4 * (3.3/1023)
  
  EmonLibCM_currentCal(0,Ical1*(3.3/1023));  // 2000 turns / 22 Ohms burden resistor
  EmonLibCM_currentCal(1,Ical2*(3.3/1023));  // 2000 turns / 22 Ohms burden resistor
  EmonLibCM_currentCal(2,Ical3*(3.3/1023));  // 2000 turns / 22 Ohms burden resistor
  EmonLibCM_currentCal(3,Ical4*(3.3/1023));  // 2000 turns / 120 Ohms burden resistor

  EmonLibCM_phaseCal(0,0.22);
  EmonLibCM_phaseCal(1,0.41);
  EmonLibCM_phaseCal(2,0.60);
  EmonLibCM_phaseCal(3,1.25);
  
  EmonLibCM_Init();
  
  attachInterrupt(pulse_countINT, onPulse, FALLING);     // Attach pulse counting interrupt pulse counting 
 
  for(byte j=0;j<MaxOnewire;j++) 
      emontx.temp[j] = 3000;                             // If no temp sensors connected default to status code 3000 
                                                         // will appear as 300 once multipled by 0.1 in emonhub
} //end SETUP

static byte waitForAck() {
  MilliTimer ackTimer;
  while (!ackTimer.poll(ACK_TIME)) {
    if (rf12_recvDone() && rf12_crc == 0 && rf12_hdr == (RF12_HDR_DST | RF12_HDR_CTL | nodeID))
      return 1;
  }
  return 0;
}

void loop()
{
  if (EmonLibCM_Ready())   
  {
    if (EmonLibCM_ACAC) {digitalWrite(LEDpin, HIGH); delay(200); digitalWrite(LEDpin, LOW);}    // flash LED if powered by AC
    
    if (EmonLibCM_ACAC) {
      EmonLibCM_datalog_period(TIME_BETWEEN_READINGS);
      emontx.Vrms = EmonLibCM_Vrms;
      emontx.power1 = EmonLibCM_getRealPower(0);
      emontx.power2 = EmonLibCM_getRealPower(1);
      emontx.power3 = EmonLibCM_getRealPower(2);
      emontx.power4 = EmonLibCM_getRealPower(3);
    } else {
      emontx.Vrms = Vrms;
      emontx.power1 = Vrms * EmonLibCM_getIrms(0);
      emontx.power2 = Vrms * EmonLibCM_getIrms(1);
      emontx.power3 = Vrms * EmonLibCM_getIrms(2);
      emontx.power4 = Vrms * EmonLibCM_getIrms(3);
    }
                                                                                         // read battery voltage if powered by DC
    // int battery_voltage=analogRead(battery_voltage_pin) * 0.681322727;                // 6.6V battery = 3.3V input = 1024 ADC

    if (DS18B20_STATUS==1) 
    {
      digitalWrite(DS18B20_PWR, HIGH); 
      delay(50);
      sensors.requestTemperatures();
      // Must wait for conversion, since we use ASYNC mode 
      if (EmonLibCM_ACAC) delay(ASYNC_DELAY); else Sleepy::loseSomeTime(ASYNC_DELAY);
      
      for(byte j=0;j<numSensors;j++) {
        float temp = sensors.getTempC(allAddress[j]);
        if ((temp>-55.0) && (temp<125.0)) {
          emontx.temp[j] = temp * 10;
        }
      }
      digitalWrite(DS18B20_PWR, LOW);
    }
    
    if (pulseCount)                                                                     // if the ISR has counted some pulses, update the total count
    {
      cli();                                                                            // Disable interrupt just in case pulse comes in while we are updating the count
      emontx.pulseCount += pulseCount;
      pulseCount = 0;
      sei();                                                                            // Re-enable interrupts
    }
 
    if (debug==1) {
      Serial.print("data: ");
      Serial.print(emontx.power1); Serial.print(" ");
      Serial.print(emontx.power2); Serial.print(" ");
      Serial.print(emontx.power3); Serial.print(" ");
      Serial.print(emontx.power4); Serial.print(" ");
      Serial.print(emontx.Vrms); Serial.print(" ");
      Serial.print(emontx.pulseCount); Serial.print(" ");
      if (DS18B20_STATUS==1){
        for(byte j=0;j<numSensors;j++){
          Serial.print(emontx.temp[j]);
          Serial.print(" ");
        } 
      }
      Serial.print(", ");
      Serial.print(millis()-lastsent);
      Serial.println("ms");
      lastsent = millis();
      
      delay(50);
    }
    
    for (byte i = 0; i < RETRY_LIMIT; ++i) {
      rf12_sendNow(RF12_HDR_ACK, &emontx, sizeof emontx);
      rf12_sendWait(RADIO_SYNC_MODE);
      byte acked = waitForAck();
      rf12_sleep(RF12_SLEEP);

      if (acked) break;
    
      if (debug==1) { 
        Serial.print("retry: ");
        Serial.println(i);
      }
      delay(RETRY_PERIOD);
    }
    
    if (!EmonLibCM_ACAC) {
      int sleeptime = (TIME_BETWEEN_READINGS - 1)*1000;
      if (DS18B20_STATUS) sleeptime -= (ASYNC_DELAY+50);
      sleeptime -= 100;
      EmonLibCM_Stop();
      delay(50);
      msdelay(sleeptime);
      EmonLibCM_datalog_period(1);
      EmonLibCM_Start();
    }
  }
}

// The interrupt routine - runs each time a falling edge of a pulse is detected
void onPulse()                  
{  
  if ( (millis() - pulsetime) > min_pulsewidth) {
    pulseCount++; // calculate wh elapsed from time between pulses
  }
  pulsetime=millis();
}

void msdelay(int ms)
{
  //delay(ms);
  Sleepy::loseSomeTime(ms);
}
