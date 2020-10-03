/*
 Name:		Dices.ino
 Created:	september 2020 9:53:29 PM
 Author:	Rob Antonisse

Sketch to be used with universal shield for arduino uno. "RFIDmulti"
Reads 4 RFID-RC522 card readers
4 extra ouputs for results bits 0~3 of shiftregister
4 ports voor switches and controls
Pins:
P1-free; P2-free;P3-free;P4-free; P5-Serial data shift;P6 Shift register clock; P7 shift register latch
p8-free; p9-reserved for SPI; p10 SPI SDA;p11 SPI MOSI; p12 SPI MISO; p13 SPI CLK.
PA0- switch 1;PA1- switch 2;PA2- switch 3;PA3- switch 4;PA4-free;PA5-free
RST for readers comes from bit7~4 of shift register

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

void setup() {
	Serial.begin(9600);
	SPI.begin(); // Init SPI bus
	//portC as input voor 4 switches
	PORTC |= (15 << 0);// pullups 
	DDRD |= (B11100000 << 0); //pin7 RCLK; Pin6 SRCLK; Pin5 Serial pins as output
	RFM_switchstatus = 0xFF;
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
	RFM_Output(reader, true);
	Serial.print("Reader: "); Serial.print(reader); Serial.print(", UID= "); Serial.println(uid);
}
void RFM_free(byte reader) {
	if (RFM_carduid[reader] != 0) {
		Serial.print("reader: "); Serial.print(reader); Serial.println(" free.");
		RFM_carduid[reader] = 0;
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
}
void RFM_SW_off(byte sw) {
	Serial.print("switch uit: "); Serial.println(sw);
}

void loop() {
	RFM_slowcount++;
	if (RFM_slowcount == 0)RFM_read();
	RFM_SW_exe();
	//if (millis() - tijd > 10) { //timer 20ms
	//	tijd = millis();
	//	RFID_read();
	//}
}

