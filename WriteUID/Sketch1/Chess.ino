/*
 Name:		Chess.ino
 Created:	september 2020 9:53:29 PM
 Author:	Rob Antonisse

Sketch based on RFIDmulti
For Arduino UNO
Chess game, 4 pieces to be placed to checkmate black and solve puzzle.

Xtra hardware red led to PIN4 LP4 (RLP4 1Kohm)


GPIOR0 bit 0 = program mode

*/
#include <require_cpp11.h>
#include <MFRC522Extended.h>
#include <MFRC522.h>
#include <deprecated.h>
#include <EEPROM.h>
#include <SPI.h>
MFRC522 reader;   // Create MFRC522 instance.
//in te stellen constanten
byte aantalreaders = 4; //aantal readers, max 4 altijd aaneengesloten 1,2,3 enz (dus niet op plek 3 en 4 alleen)
byte stoptijd = 2;// readers missen af en toe een leesactie, aantal keren dat de reader niks leest voor stop


unsigned long tijd;
byte RFM_slowcount;
byte RFM_cardcount;
int RFM_carduid[4];



byte RFM_stopcount[4];
byte RFM_shiftbyte;
byte RFM_switchstatus;

//for CHESS
int CHESS_uid[4];
byte CHESS_play;
byte CHESS_solvedcount;


void setup() {
	Serial.begin(9600);
	SPI.begin(); // Init SPI bus
	//portC as input voor 4 switches
	PORTC |= (15 << 0);// pullups 
	DDRD |= (B11110000 << 0); //pin7 RCLK; Pin6 SRCLK; Pin5,pin4 Serial pins as output
	DDRB |= (1 << 0); //PIN8 output
	RFM_switchstatus = 0xFF;
	MEM_read();
}

void RFM_read() {
	unsigned int uid = 0; byte id = 0;
	RFM_cardcount++;
	if (RFM_cardcount > aantalreaders - 1) RFM_cardcount = 0;
	//PORTC &= ~(15 << 0);
	//PORTC |= (1 << RFM_cardcount);
	RFM_shiftbyte &= ~(B11110000);
	RFM_shiftbyte |= (1 << 7 - RFM_cardcount);
	RFM_shift(); //p15=bit7 ,p1=bit6,p2=bit5,p3=bit4,

	reader.PCD_Init(10, 9); // Init each MFRC522 card (10 slave select, 9 reset wordt niet gebruikt)
	if (reader.PICC_IsNewCardPresent() && reader.PICC_ReadCardSerial()) {
		//PORTB |= (1 << 0); //light test led
		for (byte i = 0; i < 4; i++) {
			uid = uid + reader.uid.uidByte[i];
			id = id ^ reader.uid.uidByte[i];
		}
		uid = uid + id;
		if (RFM_carduid[RFM_cardcount] != uid) {
			RFM_carduid[RFM_cardcount] = uid;
			RFM_on(uid, RFM_cardcount);
		}
		RFM_stopcount[RFM_cardcount] = 0;
	}
	else { //geen card aanwezig...meerdere stopperiodes periodes nodig
		RFM_stopcount[RFM_cardcount]++;
		if (RFM_stopcount[RFM_cardcount] > stoptijd)RFM_free(RFM_cardcount);
	}
}
void RFM_on(int uid, byte reader) {
	if (GPIOR0 & (1 << 0)) { //program mode
		CHESS_uid[reader] = uid;
		EEPROM.put(100 + (reader * 10), CHESS_uid[reader]);
		//clear copies of this uid (not 1 pieces for two readers)
		for (byte i = 0; i < 4; i++) {
			if (i != reader) {
				if (CHESS_uid[i] == uid) {
					EEPROM.put(100 + (10 * i), 0xFFFF);
					CHESS_uid[i] = 0xFFFF;
				}
			}
		}
	}
	if (CHESS_uid[reader] == uid) {
		RFM_Output(reader, true);

		CHESS_play |= (1 << reader);
		CHESS_solved();
	}
	else {
		RFM_Output(reader, false);
		CHESS_play &= ~(1 << reader);
	}

	//check if uid is active on other reader
	for (byte i = 0; i < 4; i++) {
		if (reader != i) {
			if (RFM_carduid[i] == uid) RFM_free(i);
		}
	}



	Serial.print("Reader: "); Serial.print(reader); Serial.print(", UID= "); Serial.println(uid);
}
void RFM_free(byte reader) {
	if (RFM_carduid[reader] != 0) {
		Serial.print("reader: "); Serial.print(reader); Serial.println(" free.");
		RFM_carduid[reader] = 0;
		CHESS_play &= ~(1 << reader);
		CHESS_solved();
		RFM_Output(reader, false);
	}
}
void RFM_shift() {
	for (byte i = 0; i < 8; i++) { //p7=bit0~p15=bit7
		PORTD &= ~(1 << 5);
		if (RFM_shiftbyte & (1 << i))PORTD |= (1 << 5);  //set serial data
		PIND |= (1 << 6); PIND |= (1 << 6); //make shift puls
	}
	PIND |= (1 << 7); PIND |= (1 << 7); //make latch puls
}
void RFM_Output(byte poort, boolean onoff) { //poort 0~3
	if (poort < 4) {
		if (onoff) {
			RFM_shiftbyte |= (1 << poort);
		}
		else {
			RFM_shiftbyte &= ~(1 << poort);
		}
	}
}
void RFM_SW_exe() {
	byte changed; byte ss;
	ss = PINC;
	ss = ss << 4;
	ss = ss >> 4;
	changed = ss ^ RFM_switchstatus;
	if (changed > 0) {
		for (byte i = 0; i < 4; i++) {
			if (changed & (1 << i)) {
				if (ss & (1 << i)) { //switch released
					RFM_SW_off(i);
				}
				else { //switch pushed
					RFM_SW_on(i);
				}
			}
		}

	}
	RFM_switchstatus = ss;
}
void RFM_SW_on(byte sw) {
	Serial.print("switch aan: "); Serial.println(sw);
	if (sw == 0)GPIOR0 |= (1 << 0); //programmode on
	for (byte i = 0; i < 4; i++) { //clear memory for allready places cards
		RFM_carduid[i] = 0;
	}
	PORTD |= (1 << 4);
}
void RFM_SW_off(byte sw) {
	Serial.print("switch uit: "); Serial.println(sw);
	if (sw == 0)GPIOR0 &= ~(1 << 0); //programmode off
	PORTD &= ~(1 << 4);
}
void MEM_read() {
	for (byte i = 0; i < 4; i++) {
		EEPROM.get(100 + (10 * i), CHESS_uid[i]);
		//CHESS_uid[i] = EEPROM.get(100 + (10 * i));
	}
}
void CHESS_solved() {
	if (CHESS_play == B00001111) { //puzzel opgelost
		GPIOR0 |= (1 << 1);
		CHESS_solvedcount++;
		if (CHESS_solvedcount > 15) {
			PORTB |= (1 << 0); //set PIN8
			CHESS_solvedcount = 0;
			GPIOR0 &= ~(1 << 1);
		}
	}
	else {
		//niet opgelost
		PORTB &= ~(1 << 0); //clear pin 8
		CHESS_solvedcount = 0;
		GPIOR0 &= ~(1 << 1);
	}
}

void loop() {
	RFM_slowcount++;
	if (RFM_slowcount == 0) {
		RFM_read();
		RFM_SW_exe();
		if(GPIOR0 & (1<<1))CHESS_solved();
	}
}

