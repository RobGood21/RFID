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

#define RST_PIN 9 //reset pin SPI
MFRC522 reader;   // Create MFRC522 instance.
byte sspin = 10;  //slave select pins

unsigned long tijd;
byte code[10];

void setup() {
	Serial.begin(9600);
	SPI.begin(); // Init SPI bus

	reader.PCD_Init(sspin, RST_PIN); // Init each MFRC522 card
	Serial.print(F("Reader "));
	Serial.print(F(": "));
	reader.PCD_DumpVersionToSerial();


}


void loop() {
	if (millis() - tijd > 20) { //timer 20ms
		tijd = millis();
		if (reader.PICC_IsNewCardPresent() && reader.PICC_ReadCardSerial()) {
			Serial.print(F(": Card UID:"));
			
			for (byte i = 0; i < reader.uid.size; i++) {
				if (reader.uid.uidByte[i] != code[i])GPIOR0 &= ~(1 << 1); //reset flag
				Serial.print(reader.uid.uidByte[i] < 0x10 ? " 0" : " ");
				Serial.print(reader.uid.uidByte[i], HEX);
			}
			Serial.println("");
			reader.PICC_HaltA();// Halt PICC			
			reader.PCD_StopCrypto1();// Stop encryption on PCD
			//count = 0;/

		}
	}
}
