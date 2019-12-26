/*
 Name:		RFIDsingle.ino
 Created:	12/24/2019 1:18:17 PM
 Author:	Rob Antonisse

 Card reader voor 1 enkele reader, leest alleen het UID 


*/

#include <SPI.h>
#include <MFRC522.h>
#define RST_PIN 9 //reset pin SPI
MFRC522 reader;   // Create MFRC522 instance.
byte sspin = 10;  //slave select pins


void setup() {
	Serial.begin(9600);
	SPI.begin(); // Init SPI bus

	
		reader.PCD_Init(sspin, RST_PIN); // Init each MFRC522 card
		Serial.print(F("Reader "));
		Serial.print(F(": "));
		reader.PCD_DumpVersionToSerial();	
}

void loop() {
	if (reader.PICC_IsNewCardPresent() && reader.PICC_ReadCardSerial()) {
		Serial.print(F(": Card UID:"));
		for (byte i = 0; i < reader.uid.size; i++) {
			Serial.print(reader.uid.uidByte[i] < 0x10 ? " 0" : " ");
			Serial.print(reader.uid.uidByte[i], HEX);
		}
		Serial.println("");
		reader.PICC_HaltA();// Halt PICC			
		reader.PCD_StopCrypto1();// Stop encryption on PCD
	}
}
