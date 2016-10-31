

byte value;

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
          if (RF_STATUS==1) rf12_initialize(nodeID, RF_freq, networkGroup);
        break;
      }

      case 'b': // set band: 4 = 433, 8 = 868, 9 = 915
        value = bandToFreq(value);
        if (value){
          RF_freq = value;
          if (RF_STATUS==1) rf12_initialize(nodeID, RF_freq, networkGroup);
        }
        break;
    
      case 'g': // set network group
        if (value>=0){
          networkGroup = value;
          if (RF_STATUS==1) rf12_initialize(nodeID, RF_freq, networkGroup);
        }
        break;

      case 's': // set Vcc Cal 1=UK/EU 2=USA
        Serial.print(F("Saved to EEPROM, reset to apply"));
        break;

      case 'v': // print firmware version
        Serial.print(F("[emonTx.")); Serial.print(version*0.1); Serial.print(F("]"));
        break;
      
      default:
        showString(helpText1);
      } //end case
    //Print Current RF config

    if (RF_STATUS==1) {
      Serial.print(F(" "));
      Serial.print((char) ('@' + (nodeID & RF12_HDR_MASK)));
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