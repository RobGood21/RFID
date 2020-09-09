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

//#define RST_PIN 9 //reset pin SPI
MFRC522 reader;   // Create MFRC522 instance.
//byte sspin = 7;  //slave select pins

unsigned long tijd;
byte code[10];
byte cardcount;

void setup() {
	Serial.begin(9600);
	SPI.begin(); // Init SPI bus
	//portC as output
	DDRC |= (15 << 0);

}
void RFID_read() {
	byte id = 0x00;
	cardcount++;
	if (cardcount > 3) cardcount = 0;
	PORTC &= ~(15 << 0); 
	PORTC |= (1 << cardcount);

	reader.PCD_Init(10, 9); // Init each MFRC522 card (7 slave select, 9 reset)
	//Serial.println("read.....");
	//Serial.print(F("Reader "));
	//Serial.print(F(": "));
	//reader.PCD_DumpVersionToSerial();
	//if(reader.PICC_ReadCardSerial()){
	if (reader.PICC_IsNewCardPresent() && reader.PICC_ReadCardSerial()) {
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
}

void loop() {
	if (millis() - tijd > 200) { //timer 20ms
		tijd = millis();
		RFID_read();
	}
}
