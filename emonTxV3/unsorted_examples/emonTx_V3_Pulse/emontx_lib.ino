void send_rf_data()
{
  rf12_sleep(RF12_WAKEUP);
  rf12_sendNow(0, &emontx, sizeof emontx);                           //send temperature data via RFM12B using new rf12_sendNow wrapper
  rf12_sendWait(2);
  rf12_sleep(RF12_SLEEP);
}

void emontx_sleep(int seconds) {
    for (int i=0; i<seconds; i++) { 
      delay(1000); 
      if (UNO) wdt_reset();
    } 
}
