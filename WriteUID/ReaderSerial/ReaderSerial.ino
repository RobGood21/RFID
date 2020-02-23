/*
 Name:		RFIDaccess.ino
 Created:	12/26/2019 12:28:06 PM
 Author:	Rob Antonisse

RFID kaart wordt gelezen, als nog geen UID vastgelegd, dus eerste kaart na een reset,
wordt deze UID opgeslagen in EEPROM en voortaan als toegang UID gebruikt.
Als output een led

Geschikt voor enkelvoudige RFID toegang projecten
Voor debugging alle serial pronts weer aanzetten


*/

#include <require_cpp11.h>
#include <MFRC522Extended.h>
#include <MFRC522.h>
#include <deprecated.h>
#include <EEPROM.h>
#include <SPI.h>


#define RST_PIN 9 //reset pin SPI
MFRC522 reader;   // Create MFRC522 instance.
byte sspin = 10;  //slave select pins


byte oldcard;

byte switchstatus;
unsigned long tijd;
byte code[10];
byte ledmode = 0;
byte count;
byte countlong;

void setup() {
	Serial.begin(9600);

	SPI.begin(); // Init SPI bus
	DDRD &= ~(1 << 7); //PIN7 as input
	PORTD |= (1 << 7); //PIN7 pull up
	DDRD |= (1 << 6); //pin6 as output
	DDRD |= (1 << 5); //pin5 as output
	
	PORTD &= ~(1 << 5);
	PORTD |= (1 << 6);
	
	//PORTD |= (3 << 5); //outputs high, low active for relaismodules


	reader.PCD_Init(sspin, RST_PIN); // Init each MFRC522 card
	Serial.print(F("Reader "));
	Serial.print(F(": "));
	reader.PCD_DumpVersionToSerial();




	MEM_load();
}
void MEM_load() {
	for (byte i = 0; i < 10; i++) {
		code[i] = EEPROM.read(100 + i);
	}
}
void MEM_clear() {
	for (byte i = 0; i < 10; i++) {
		EEPROM.update(100 + i, 0xFF);
		code[i] = 0;
	}
}
void switches() {
	byte sw;
	byte ch;
	sw = PIND >> 7; //shift out leave used switches
	sw = sw << 7; //leafves only bit7 (PIN7)
	ch = sw ^ switchstatus;
	if (ch > 0) {
		for (byte i = 0; i < 8; i++) {
			if (bitRead(sw, i) == false & bitRead(ch, i) == true) {
				switchon(i);
			}
		}
	}
	switchstatus = sw;
}
void switchon(byte s) {
	//	Serial.println("ingedrukt");
	MEM_clear();
}
void loop() {
	byte card;

	if (millis() - tijd > 20) { //timer 20ms
		tijd = millis();
		switches();

		reader.PCD_Init(sspin, RST_PIN); // Init each MFRC522 card
		reader.PCD_SetAntennaGain(reader.RxGain_43dB);

		if (reader.PICC_IsNewCardPresent() && reader.PICC_ReadCardSerial()) { //
			card = reader.uid.uidByte[0] ^ reader.uid.uidByte[1];

			if (card != oldcard) {
				Serial.println(card, HEX);
				oldcard = card;
				PORTD &= ~(1 << 6);
				PORTD |= (1 << 5);
				countlong = 0;
			}

			//for (byte i = 0; i < reader.uid.size; i++) {
			//	if (reader.uid.uidByte[i] != code[i])GPIOR0 &= ~(1 << 1); //reset flag
			//	Serial.print(reader.uid.uidByte[i] < 0x10 ? " 0" : " ");
			//	Serial.print(reader.uid.uidByte[i], HEX);
			//}
			//Serial.println("");

			reader.PICC_HaltA();// Halt PICC			
			reader.PCD_StopCrypto1();// Stop encryption on PCD
			count = 0;
		}
		countlong++;
		if (countlong == 0) {
			oldcard = 0x00;
			PORTD &= ~(1 << 5);
			PORTD |= (1 << 6);			
		}



	}
}