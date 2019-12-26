/*
 Name:		RFIDaccess.ino
 Created:	12/26/2019 12:28:06 PM
 Author:	Rob Antonisse

RFID kaart wordt gelezen, als nog geen UID vastgelegd, dus eerste kaart na een reset,
wordt deze UID opgeslagen in EEPROM en voortaan als toegang UID gebruikt.
Als output een led

Geschikt voor enkelvoudige RFID toegang projecten


*/


#include <EEPROM.h>
#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN 9 //reset pin SPI
MFRC522 reader;   // Create MFRC522 instance.
byte sspin = 10;  //slave select pins

byte switchstatus;
unsigned long tijd;

byte code[10];
byte ledmode=0;
byte count;

void setup() {
	Serial.begin(9600);
	SPI.begin(); // Init SPI bus

	DDRD &= ~(1 << 7); //PIN7 as input
	PORTD |= (1 << 7); //PIN7 pull up

	DDRD |= (1 << 6); //pin6 as output
	DDRD |= (1 << 5); //pin5 as output
	PORTD |= (3 << 5); //outputs high, low active for relaismodules


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

void MEM_new() {
	GPIOR0 |= (1 << 0); //set flag new card
	//gebruik 10 byte 100~109
	for (byte i = 0; i < 10; i++) {
		if (EEPROM.read(100 + 1) != 0xFF) {
			GPIOR0 &= ~(1 << 0); //reset flag
			i = 10;
		}
	}

	if (bitRead(GPIOR0, 0) == true) { //new card, write to memorie
		Serial.println("nieuwe kaart");
		for (byte i = 0; i < 10; i++) {
			code[i]=reader.uid.uidByte[i]; //write uid in memory
			EEPROM.write(100 + i, code[i]);
		}
	}
}

void printum() {
	Serial.print("IN geheugen: ");
	for (byte i = 0; i < 10; i++) {
		Serial.print(" ");
		Serial.print(EEPROM.read(100 + i), HEX);
	}
	Serial.println("");
}

void switches() {
	byte sw;
	byte ch;
	sw = PIND >> 7; //shift out leave used switches
	sw = sw << 7; //leafves only bit7 (PIN7)
	ch = sw ^ switchstatus;
	if (ch > 0) {
		for (byte i = 0; i < 8; i++) {
			if (bitRead(sw,i) == false & bitRead(ch, i) == true) {
				switchon(i);
			}
		}
	}
	switchstatus = sw;
}

void switchon(byte s) {
	Serial.println("ingedrukt");
	MEM_clear();
}
void blink() {
count++;
	switch (ledmode) {
	case 0:
		break;
	case 1:		
		if (count == 1)PORTD &= ~(1 << 6);
		if (count == 50) {
			PORTD |= (1 << 6);
			count = 0;
			ledmode = 0;
		}
		break;
	case 2:
		if (count == 1)PORTD &= ~(1 << 5);
		if (count == 10) {
			PORTD |= (1 << 5);
			count = 0;
			ledmode = 0;
		}
		break;
	}
}

void loop() {
	if (millis() - tijd > 20) { //timer 20ms
		tijd = millis();
		switches();
		blink();

		if (reader.PICC_IsNewCardPresent() && reader.PICC_ReadCardSerial()) {
			printum();
			MEM_new(); //check if there is a new to assign card

			GPIOR0 |= (1 << 1); //set flag
			Serial.print(F(": Card UID:"));
			for (byte i = 0; i < reader.uid.size; i++) {

				if (reader.uid.uidByte[i] != code[i])GPIOR0 &= ~(1 << 1); //reset flag

				Serial.print(reader.uid.uidByte[i] < 0x10 ? " 0" : " ");
				Serial.print(reader.uid.uidByte[i], HEX);
							   				 
			}
			Serial.println("");
			reader.PICC_HaltA();// Halt PICC			
			reader.PCD_StopCrypto1();// Stop encryption on PCD

			count = 0;
			if (bitRead(GPIOR0, 1) == true) {
				Serial.println("open");
				ledmode = 1;
			}
			else {
				Serial.println("dicht");
				ledmode = 2;
			}		
		}
	}
}
