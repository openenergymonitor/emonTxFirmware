

#include <EEPROM.h>

byte value;

static void load_config(){
  
  byte flag=0;
  // Read nodeID
  if (EEPROM.read(0) != 255){                                // 255 = EEPROM default (blank) value
    nodeID = EEPROM.read(0);
    flag++;
  }
  if (EEPROM.read(1) != 255){
    RF_freq = EEPROM.read(1);
    flag++;
  }
  if (EEPROM.read(2) != 255){
    networkGroup = EEPROM.read(2);
    flag++;
  }
  
  if (flag > 0){
    Serial.println("Loaded EEPROM RF config >");
  }
  else {
    Serial.println("No EEPROM config");
  }
  
  }

static void config (char c) {
  
  if ('0' <= c && c <= '9') {
    value = 10 * value + c - '0';
    return;
  }

  if (c > ' ') {

    switch (c) {

      case 'i': //set node ID
        if (value){
          nodeID = value;
        break;
      }

      case 'b': // set band: 4 = 433, 8 = 868, 9 = 915
        value = bandToFreq(value);
        if (value){
          RF_freq = value;
        }
        break;
    
      case 'g': // set network group
        if (value>=0){
          networkGroup = value;
        }
        break;

      case 's': // Save to EEPROM. Atemga328p has 1kb  EEPROM
        save_config();
        break;

      case 'v': // print firmware version
        Serial.print(F("[emonTx FW: V")); Serial.print(version*0.1); Serial.print(F("]"));
        break;
      
      default:
        showString(helpText1);
      } //end case

    //Print Current RF config
    if (RF_STATUS==1) {
      Serial.print(F(" "));
      // Serial.print((char) ('@' + (nodeID & RF12_HDR_MASK)));
      Serial.print(F(" i"));
      Serial.print(nodeID & RF12_HDR_MASK);
      Serial.print(F(" g"));
      Serial.print(networkGroup);
      Serial.print(F(" @ "));
      Serial.print(RF_freq == RF12_433MHZ ? 433 :
                   RF_freq == RF12_868MHZ ? 868 :
                   RF_freq == RF12_915MHZ ? 915 : 0);
      Serial.print(F(" MHz"));
    }
    
    Serial.print(F(" USA ")); Serial.print(USA);
    Serial.println(F(" "));
    
  } // end c > ' '
  value = 0;

}

static void save_config(){
  Serial.println("Saving...");
  // Clear any old EEPROM settings
  for (int i = 0 ; i < EEPROM.length() ; i++) {
    EEPROM.write(i, 0);
  }
  //Save new settings
  EEPROM.write(0, nodeID);
  EEPROM.write(1, RF_freq);
  EEPROM.write(2, networkGroup);
  Serial.println("Done. New config saved to EEPROM");
}

static byte bandToFreq (byte band) {
  return band == 4 ? RF12_433MHZ : band == 8 ? RF12_868MHZ : band == 9 ? RF12_915MHZ : 0;
}

static void showString (PGM_P s) {
  for (;;) {
    char c = pgm_read_byte(s++);
    if (c == 0)
      break;
    if (c == '\n')
      Serial.print('\r');
    Serial.print(c);
  }
}