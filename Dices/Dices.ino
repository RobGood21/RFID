/*
 Name:		Dices.ino
 Created:	september 2020 9:53:29 PM
 Author:	Rob Antonisse

 Sketch for escape room puzzle "Dices"
 4 RFID-RC522 readers controlled by 1 Arduino uno. 
 Multiplexing done by pulling only one of the RST lines to VCC others hold to GND.
 Further use seperate 3.3V supply supply of arduino is not sufficient

*/
#include <require_cpp11.h>
#include <MFRC522Extended.h>
#include <MFRC522.h>
#include <deprecated.h>
#include <EEPROM.h>
#include <SPI.h>
#include <FastLED.h>
#define fl GPIOR0 |=(1<<1); //request fastled show
MFRC522 reader;   // Create MFRC522 instance.
CRGB pix[30]; //aantal pixels WS2812B
unsigned long tijd;
byte code[10];
byte cardcount;
byte solved; // als 0x0F puzzel opgelost....bit0=reader 1 enz
byte countll;
byte stopcount[4];
void setup() {
	Serial.begin(9600);
	SPI.begin(); // Init SPI bus
	FastLED.addLeds<NEOPIXEL, 7>(pix, 30);
	//portC as output
	DDRC |= (15 << 0);
	DDRB |= (1 << 0); //PIN8 output temp led
	DDRD &= ~(1 << 6); //pin6 input
	PORTD |= (1 << 6); //pull up to PIN6
	DDRD |= (1 << 5); //PIN5, relais als output
	PORTD |= (1 << 5); //Relais low active
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
			fl;
		}
	}
	else {
		if (GPIOR0 & (1 << 0)) {
			fill_solid(pix, 30, CRGB::Black);
			GPIOR0 &= ~(1 << 0); //program mode off
			fl;
		}
	}
	cardcount++;
	if (cardcount > 3) cardcount = 0;
	PORTC &= ~(15 << 0);
	PORTC |= (1 << cardcount);
	reader.PCD_Init(10, 9); // Init each MFRC522 card (10 slave select, 9 reset wordt niet gebruikt)
	if (reader.PICC_IsNewCardPresent() && reader.PICC_ReadCardSerial()) {
		PORTB |= (1 << 0); //light test led
		for (byte i = 0; i < reader.uid.size; i++) {
			id = id ^ reader.uid.uidByte[i];
		}
		if (GPIOR0 & (1 << 0)) { //programmode
			if (EEPROM.read(100 + cardcount) != id) {
				EEPROM.update(100 + cardcount, id); //position
				EEPROM.update(104 + cardcount, id); //memory color
				setcolor(cardcount, cardcount);
				//wissen duplicaten, nog niet helemaal goed, kleuren moeten nog uit bij verplaatsen dice
				for (byte i = 0; i < 4; i++) {
					if (i != cardcount) {
						if (EEPROM.read(100 + i) == id) EEPROM.write(100 + i, 0xFF); //clear positions
						if (EEPROM.read(104 + i) == id) EEPROM.write(104 + i, 0xFF); //clear colors
					}
				}
			}
		}
		else { //in bedrijf, alleen lezen
			//check kleur, dobbelsteen
			for (byte i = 0; i < 4; i++) {
				if (EEPROM.read(104 + i) == id) {
					//kleur gevonden
					setcolor(cardcount, i);
					if (EEPROM.read(100 + cardcount) == id) {
						//positie en kleur ok
						solved |= (1 << cardcount);
					}
				}
			}
		}
		stopcount[cardcount] = 0;
	}
	else { //geen card aanwezig...meerdere stopperiodes periodes nodig
		if(stopcount[cardcount]>2) { //4x een niet-card-aanwezig voordat uitgaat
			solved &= ~(1 << cardcount);
			setcolor(cardcount, 4);
			stopcount[cardcount] = 0;
		}
		else {
			stopcount[cardcount] ++;
		}
	}
	//waar is dit voor? Eens uitzoeken
	//reader.PICC_HaltA();// Halt PICC			
	//reader.PCD_StopCrypto1();// Stop encryption on PCD	
//}
	if (GPIOR0 & (1 << 1)) {
		Serial.println("show");
		FastLED.show();
		GPIOR0 &= ~(1 << 1);
	}
	if (solved == 0x0F) {
		//puzzel opgelost, deuropenen
		PORTD &= ~((1 << 5));
	}
	else {
		PORTD |= (1 << 5);
	}
}
void setcolor(byte pos, byte color) {
	byte rd; byte gr; byte bl;
	switch (color) {
	case 0: //groen
		rd = 0x00; gr = 0xFF; bl = 0x00;
		break;
	case 1: //paars 0x9400D3
		rd = 0x94; gr = 0x00; bl = 0xD3;
		break;
	case 2: //oranje 0xFF4500
		rd = 0xFF; gr = 0x45; bl = 0x00;
		break;
	case 3: //blauw
		rd = 0x00; gr = 0x00; bl = 0xFF;
		break;
	case 4: //zwart, uit
		rd = 0x00; gr = 0x00; bl = 0x00;
		break;
	}
	for (byte i = (pos * 7); i < (pos + 1) * 7; i++) {
		pix[i].r = rd; pix[i].g = gr; pix[i].b = bl;
	}
	fl;
}
void loop() {
	if (millis() - tijd > 20) { //timer 20ms
		tijd = millis();
		RFID_read();	
	}
}
