/*
 Name:		RFID_RC522.ino
 Created:	12/20/2019 11:07:40 AM
 Author:	rob Antonisse
 Basis gebruik van RFID card alleen als toegang, identificatie. Storage op de card en verdere trucken niet bekeken.
 Gebruikt MFRC522 library BY COOQROBOT

RFID reader voor twee readers

*/

#include <SPI.h>
#include <MFRC522.h>
#define RST_PIN 9 //reset pin SPI
MFRC522 reader[2];   // Create MFRC522 instance.
byte sspin[2] = { 10,8 };  //slave select pins


void setup() {
	Serial.begin(9600);
	SPI.begin(); // Init SPI bus

	for (uint8_t card = 0; card < 2; card++) {
		reader[card].PCD_Init(sspin[card], RST_PIN); // Init each MFRC522 card
		Serial.print(F("Reader "));
		Serial.print(card);
		Serial.print(F(": "));
		reader[card].PCD_DumpVersionToSerial();
	}
}

void loop() {
	for (byte card = 0; card < 2; card++) {		

		if (reader[card].PICC_IsNewCardPresent() && reader[card].PICC_ReadCardSerial()) { // Look for new cards
			Serial.print(F("Reader "));
			Serial.print(card);
			// Show some details of the PICC (that is: the tag/card)
			Serial.print(F(": Card UID:"));

			//reader[card].uid.uidByte;

			for (byte i = 0; i < reader[card].uid.size; i++) {
				Serial.print(reader[card].uid.uidByte[i] < 0x10 ? " 0" : " ");
				Serial.print(reader[card].uid.uidByte[i], HEX);
			}


			//dump_byte_array(reader[card].uid.uidByte, reader[card].uid.size);
			Serial.println();
			Serial.print(F("PICC type: "));
			MFRC522::PICC_Type piccType = reader[card].PICC_GetType(reader[card].uid.sak);
			Serial.println(reader[card].PICC_GetTypeName(piccType));			
			reader[card].PICC_HaltA();// Halt PICC			
			reader[card].PCD_StopCrypto1();// Stop encryption on PCD
		} //if (reader[card].PICC_IsNewC
	} //for(byte reader

}
void dump_byte_array(byte *buffer, byte bufferSize) {
	for (byte i = 0; i < bufferSize; i++) {
		//Serial.print(buffer[i] < 0x10 ? " 0" : " ");
		Serial.print(buffer[i], DEC);
	}
}
