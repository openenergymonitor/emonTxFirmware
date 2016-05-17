/*
Interface for the RFM69CW Radio Module
*/

#ifdef RFM69CW

#include <avr/sleep.h>	
#define REG_FIFO            0x00	
#define REG_OPMODE          0x01
#define MODE_TRANSMITTER    0x0C
#define REG_DIOMAPPING1     0x25	
#define REG_IRQFLAGS2       0x28
#define IRQ2_FIFOFULL       0x80
#define IRQ2_FIFONOTEMPTY   0x40
#define IRQ2_PACKETSENT     0x08
#define IRQ2_FIFOOVERRUN    0x10



void rfm_init(void)
{	
	// Set up to drive the Radio Module
	digitalWrite(RFMSELPIN, HIGH);
	pinMode(RFMSELPIN, OUTPUT);
	SPI.begin();
	SPI.setBitOrder(MSBFIRST);
	SPI.setDataMode(0);
	SPI.setClockDivider(SPI_CLOCK_DIV8);
	
	// Initialise RFM69CW
	do 
		writeReg(0x2F, 0xAA); // RegSyncValue1
	while (readReg(0x2F) != 0xAA) ;
	do
	  writeReg(0x2F, 0x55); 
	while (readReg(0x2F) != 0x55);
	
	writeReg(0x01, 0x04); // RegOpMode: RF_OPMODE_SEQUENCER_ON | RF_OPMODE_LISTEN_OFF | RF_OPMODE_STANDBY
	writeReg(0x02, 0x00); // RegDataModul: RF_DATAMODUL_DATAMODE_PACKET | RF_DATAMODUL_MODULATIONTYPE_FSK | RF_DATAMODUL_MODULATIONSHAPING_00 = no shaping
	writeReg(0x03, 0x02); // RegBitrateMsb  ~49.23k BPS
	writeReg(0x04, 0x8A); // RegBitrateLsb
	writeReg(0x05, 0x05); // RegFdevMsb: ~90 kHz 
	writeReg(0x06, 0xC3); // RegFdevLsb
	#ifdef RF12_868MHZ
		writeReg(0x07, 0xD9); // RegFrfMsb: Frf = Rf Freq / 61.03515625 Hz = 0xD90000 = 868.00 MHz as used JeeLib  
		writeReg(0x08, 0x00); // RegFrfMid
		writeReg(0x09, 0x00); // RegFrfLsb
	#elif defined RF12_915MHZ // JeeLib uses 912.00 MHz	
		writeReg(0x07, 0xE4); // RegFrfMsb: Frf = Rf Freq / 61.03515625 Hz = 0xE40000 = 912.00 MHz as used JeeLib 
		writeReg(0x08, 0x00); // RegFrfMid
		writeReg(0x09, 0x00); // RegFrfLsb
	#else // default to 433 MHz band
		writeReg(0x07, 0x6C); // RegFrfMsb: Frf = Rf Freq / 61.03515625 Hz = 0x6C8000 = 434.00 MHz as used JeeLib 
		writeReg(0x08, 0x80); // RegFrfMid
		writeReg(0x09, 0x00); // RegFrfLsb
	#endif

//	writeReg(0x0B, 0x20); // RegAfcCtrl:
	writeReg(0x11, RFPWR); // RegPaLevel = 0x9F = PA0 on, +13 dBm  -- RFM12B equivalent: 0x99 | 0x88 (-10dBm) appears to be the max before the AC power supply fails @ 230 V mains. Min value is 0x80 (-18 dBm)
	writeReg(0x1E, 0x2C); //
	writeReg(0x25, 0x80); // RegDioMapping1: DIO0 is used as IRQ 
	writeReg(0x26, 0x03); // RegDioMapping2: ClkOut off
	writeReg(0x28, 0x00); // RegIrqFlags2: FifoOverrun

	// RegPreamble (0x2c, 0x2d): default 0x0003
	writeReg(0x2E, 0x88); // RegSyncConfig: SyncOn | FifoFillCondition | SyncSize = 2 bytes | SyncTol = 0
	writeReg(0x2F, 0x2D); // RegSyncValue1: Same as JeeLib
	writeReg(0x30, networkGroup); // RegSyncValue2
	writeReg(0x37, 0x00); // RegPacketConfig1: PacketFormat=fixed | !DcFree | !CrcOn | !CrcAutoClearOff | !AddressFiltering >> 0x00
}


// transmit data via the RFM69CW
void rfm_send(const byte *data, const byte size, const byte group, const byte node)      // *SEND RF DATA*
{
	while (readReg(REG_IRQFLAGS2) & (IRQ2_FIFONOTEMPTY | IRQ2_FIFOOVERRUN))		// Flush FIFO
        readReg(REG_FIFO);

	writeReg(REG_OPMODE, (readReg(REG_OPMODE) & 0xE3) | MODE_TRANSMITTER);		// Transmit mode
    writeReg(REG_DIOMAPPING1, 0x00); 											// PacketSent
		
	volatile uint8_t txstate = 0;
	byte i = 0;
	uint16_t crc = _crc16_update(~0, group);	

	while(txstate < 7)
	{
		if ((readReg(REG_IRQFLAGS2) & IRQ2_FIFOFULL) == 0)			// FIFO !full
		{
			uint8_t next = 0xAA;
			switch(txstate)
			{
			  case 0: next=node & 0x1F; txstate++; break;    		// Bits: CTL, DST, ACK, Node ID(5)
			  case 1: next=size; txstate++; break;				   	// No. of payload bytes
			  case 2: next=data[i++]; if(i==size) txstate++; break;
			  case 3: next=(byte)crc; txstate++; break;
			  case 4: next=(byte)(crc>>8); txstate++; break;
			  case 5:
			  case 6: next=0xAA; txstate++; break; 					// dummy bytes (if < 2, locks up)
			}
			if(txstate<4) crc = _crc16_update(crc, next);
			writeReg(REG_FIFO, next);								// RegFifo(next);
		}
	}

	//while (readReg(REG_IRQFLAGS2) & IRQ2_FIFONOTEMPTY)			// 58 Bytes max useful payload
	while (!(readReg(REG_IRQFLAGS2) & IRQ2_PACKETSENT))				// wait for transmission to complete (not present in JeeLib) 
																	//   60 Bytes max useful payload
		;
	writeReg(REG_OPMODE, (readReg(REG_OPMODE) & 0xE3) | 0x01); 		// Standby Mode
	set_sleep_mode(SLEEP_MODE_IDLE);  								// SLEEP_MODE_STANDBY
	sleep_mode();	
	
} 



void writeReg(uint8_t addr, uint8_t value)
{
	select();
	SPI.transfer(addr | 0x80);
	SPI.transfer(value);
	unselect();
}

uint8_t readReg(uint8_t addr)
{
	select();
	SPI.transfer(addr & 0x7F);
	uint8_t regval = SPI.transfer(0);
	unselect();
	return regval;
}

// select the transceiver
void select() {
	noInterrupts();
	SPI.setDataMode(SPI_MODE0);
	SPI.setBitOrder(MSBFIRST);
	SPI.setClockDivider(SPI_CLOCK_DIV4); // decided to slow down from DIV2 after SPI stalling in some instances, especially visible on mega1284p when RFM69 and FLASH chip both present
	digitalWrite(RFMSELPIN, LOW);
}

// UNselect the transceiver chip
void unselect() {
	digitalWrite(RFMSELPIN, HIGH);
	interrupts();
}
#endif


/*
Interface for the RFM12B Radio Module
*/

#ifdef RFM12B

#define SDOPIN 12

void rfm_init(void)
{	
	// Set up to drive the Radio Module
	pinMode (RFMSELPIN, OUTPUT);
	digitalWrite(RFMSELPIN,HIGH);
	// start the SPI library:
	SPI.begin();
	SPI.setBitOrder(MSBFIRST);
	SPI.setDataMode(0);
	SPI.setClockDivider(SPI_CLOCK_DIV8);
	// initialise RFM12
	delay(200); // wait for RFM12 POR
	rfm_write(0x0000); // clear SPI
	#ifdef RF12_868MHZ
	  rfm_write(0x80E7); // EL (ena dreg), EF (ena RX FIFO), 868 MHz, 12.0pF 
	  rfm_write(0xA640); // 868.00 MHz as used JeeLib 
	#elif defined RF12_915MHZ
	  rfm_write(0x80F7); // EL (ena dreg), EF (ena RX FIFO), 915 MHz, 12.0pF 
	  rfm_write(0xA640); // 912.00 MHz as used JeeLib 	
	#else // default to 433 MHz band
	  rfm_write(0x80D7); // EL (ena dreg), EF (ena RX FIFO), 433 MHz, 12.0pF 
	  rfm_write(0xA640); // 434.00 MHz as used JeeLib 
	#endif  
	rfm_write(0x8208); // Turn on crystal,!PA
	rfm_write(0xA640); // 433 or 868 MHz exactly
	rfm_write(0xC606); // approx 49.2 Kbps, as used by emonTx
	//rfm_write(0xC657); // approx 3.918 Kbps, better for long range
	rfm_write(0xCC77); // PLL 
	rfm_write(0x94A0); // VDI,FAST,134kHz,0dBm,-103dBm 
	rfm_write(0xC2AC); // AL,!ml,DIG,DQD4 
	rfm_write(0xCA83); // FIFO8,2-SYNC,!ff,DR 
	rfm_write(0xCEd2); // SYNC=2DD2
	rfm_write(0xC483); // @PWR,NO RSTRIC,!st,!fi,OE,EN 
	rfm_write(0x9850); // !mp,90kHz,MAX OUT 
	rfm_write(0xE000); // wake up timer - not used 
	rfm_write(0xC800); // low duty cycle - not used 
	rfm_write(0xC000); // 1.0MHz,2.2V 
}


// transmit data via the RFM12
void rfm_send(const byte *data, const byte size, const byte group, const byte node)
{
	byte i=0,next,txstate=0;
	word crc=~0;
  
	rfm_write(0x8228); // OPEN PA
	rfm_write(0x8238);

	digitalWrite(RFMSELPIN,LOW);
	SPI.transfer(0xb8); // tx register write command
  
	while(txstate<13)
	{
		while(digitalRead(SDOPIN)==0); // wait for SDO to go high
		switch(txstate)
		{
			case 0:
			case 1:
			case 2: next=0xaa; txstate++; break;
			case 3: next=0x2d; txstate++; break;
			case 4: next=group; txstate++; break;
			case 5: next=node; txstate++; break; // node ID
			case 6: next=size; txstate++; break;
			case 7: next=data[i++]; if(i==size) txstate++; break;
			case 8: next=(byte)crc; txstate++; break;
			case 9: next=(byte)(crc>>8); txstate++; break;
			case 10:
			case 11:
			case 12: next=0xaa; txstate++; break; // dummy bytes (if <3 CRC gets corrupted sometimes)
		}
		if((txstate>4)&&(txstate<9)) crc = _crc16_update(crc, next);
		SPI.transfer(next);
	}
	digitalWrite(RFMSELPIN,HIGH);

	rfm_write( 0x8208 ); // CLOSE PA
	rfm_write( 0x8200 ); // enter sleep
}


// write a command to the RFM12
word rfm_write(word cmd)
{
	word result;
  
	digitalWrite(RFMSELPIN,LOW);
	result=(SPI.transfer(cmd>>8)<<8) | SPI.transfer(cmd & 0xff);
	digitalWrite(RFMSELPIN,HIGH);
	return result;
}

#endif


