/*
 Name:		Dices.ino
 Created:	9/8/2020 9:53:29 PM
 Author:	Rob Antonisse

 Sketch for escape room puzzle "Dices"
*/



#include <require_cpp11.h>
#include <MFRC522Extended.h>
#include <MFRC522.h>
#include <deprecated.h>
#include <EEPROM.h>
#include <SPI.h>

#include <FastLED.h>

//#define RST_PIN 9 //reset pin SPI
MFRC522 reader;   // Create MFRC522 instance.
//byte sspin = 7;  //slave select pins
CRGB pix[30];
unsigned long tijd;
byte code[10];
byte cardcount;

byte countll;

void setup() {
	Serial.begin(9600);
	SPI.begin(); // Init SPI bus
	FastLED.addLeds<NEOPIXEL, 7>(pix, 30);
	//portC as output
	DDRC |= (15 << 0);
	DDRB |= (1 << 0); //PIN8 output temp led
	DDRD &= ~(1 << 6); //pin6 input
	PORTD |= (1 << 6); //pull up to PIN6

	fill_solid(pix, 30, CRGB(5, 0, 0));
	FastLED.show();
}
void RFID_read() {
	byte id = 0x00;
	PORTB &= ~(1 << 0);

	//check for program request
	if (~PIND & (1 << 6)) {
		//enter program mode
		if (~GPIOR0 & (1 << 0)) {
		fill_solid(pix, 30, CRGB(20, 20, 20));
		GPIOR0 |= (1 << 0); //program mode on
		}
	}
	else {
		if (GPIOR0 & (1 << 0)) {
			fill_solid(pix, 30, CRGB::Black);
			GPIOR0 &= ~(1 << 0); //program mode off

		}		
	}


	cardcount++;
	if (cardcount > 3) cardcount = 0;
	PORTC &= ~(15 << 0);
	PORTC |= (1 << cardcount);

	reader.PCD_Init(10, 9); // Init each MFRC522 card (10 slave select, 9 reset wordt niet gebruikt)
	//reader.PCD_DumpVersionToSerial();
	if (reader.PICC_IsNewCardPresent() && reader.PICC_ReadCardSerial()) {
		PORTB |= (1 << 0); //light test led

		Serial.print(F("**"));

		//Serial.print(reader.uid.uidByte[0], HEX);

		for (byte i = 0; i < reader.uid.size; i++) {
			id = id ^ reader.uid.uidByte[i];
			//if (reader.uid.uidByte[i] != code[i])GPIOR0 &= ~(1 << 1); //reset flag
			//Serial.print(reader.uid.uidByte[i] < 0x10 ? " 0" : " ");
			//Serial.print(reader.uid.uidByte[i], HEX);
		}
		Serial.print(id);
		reader.PICC_HaltA();// Halt PICC			
		reader.PCD_StopCrypto1();// Stop encryption on PCD
		//count = 0;/


	}
	FastLED.show();
}
void LOOPlicht() {
	//tijdelijke test
	countll++;
	if (countll > 29)countll = 0;
	pix[countll].r = random(0, 255);
	pix[countll].g = random(0, 255);
	pix[countll].b = random(0, 255);
	FastLED.show();
}
void loop() {
	if (millis() - tijd > 50) { //timer 20ms
		tijd = millis();
		RFID_read();
		//LOOPlicht();

	}
}
