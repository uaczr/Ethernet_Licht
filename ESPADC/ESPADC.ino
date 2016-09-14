#include "Arduino.h"
/*

 A sketch to control the 10-Bit, 8-channel ADC MCP3008 on the Rheingold Heavy
 I2C and SPI Education Shield.  The code supposes the use of the Education Shield,
 but if you're using a breakout board, connect the CS pin to Digital 4, and the
 SPI pins in their usual locations.

 Website:   http://www.rheingoldheavy.com/mcp3008-tutorial-02-sampling-dc-voltage/
 Datasheet: http://ww1.microchip.com/downloads/en/DeviceDoc/21295d.pdf


 */

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <SPI.h>                         // Include the SPI library
#include "Protocol.h"

char ssid[] 				= "test";			// your network SSID (name)
char password[] 			= "testtest";		// your network password
/*
 char ssid[] 				= "SpaceShitToTheMoon";			// your network SSID (name)
 char password[] 			= "Niko25.10.1989";		// your network password
*/
/*
char ssid[] = "UPC1861266";			// your network SSID (name)
char password[] = "XEDHDAWS";		// your network password
*/
const int LED_STATUS_PIN = D0;
WiFiUDP udp_sender;
IPAddress localIP;
IPAddress broadcast;
const uint32_t port = 1103;

SPISettings MCP3008(2000000, MSBFIRST, SPI_MODE0);

const int CS_MCP3008 = D8;          // ADC Chip Select
const byte adc_single_ch0 = (0x08);     // ADC Channel 0
const byte adc_single_ch1 = (0x09);     // ADC Channel 1
const byte adc_single_ch2 = (0x0A);     // ADC Channel 2
const byte adc_single_ch3 = (0x0B);     // ADC Channel 3
const byte adc_single_ch4 = (0x0C);     // ADC Channel 4
const byte adc_single_ch5 = (0x0D);     // ADC Channel 5
const byte adc_single_ch6 = (0x0E);     // ADC Channel 6
const byte adc_single_ch7 = (0x0F);     // ADC Channel 7

uint32_t adc_value_ch0 = 0;
uint32_t adc_value_ch1 = 0;
uint32_t adc_value_ch2 = 0;
uint32_t adc_value_ch3 = 0;
uint32_t adc_value_ch4 = 0;
uint32_t adc_value_ch5 = 0;
uint32_t adc_value_ch6 = 0;
uint32_t adc_value_ch7 = 0;

uint32_t temp0 = 0;
uint32_t temp1 = 0;
uint32_t temp2 = 0;
uint32_t temp3 = 0;
uint32_t temp4 = 0;
uint32_t temp5 = 0;
uint32_t temp6 = 0;
uint32_t temp7 = 0;

bool changed = false;
bool pressed = false;

settingMessage setMesg;

long oneSec = 0;

void setup() {

	SPI.begin();
	Serial.begin(9600);

	pinMode(CS_MCP3008, OUTPUT);
	digitalWrite(CS_MCP3008, LOW);      // Cycle the ADC CS pin as per datasheet
	digitalWrite(CS_MCP3008, HIGH);

	pinMode(LED_STATUS_PIN, OUTPUT);
	digitalWrite(LED_STATUS_PIN, HIGH);
	Serial.println("PINS Configured");
	delay(100);
	Serial.print("Connecting to: ");
	Serial.println(ssid);

	WiFi.begin(ssid, password);
	while (WiFi.status() != WL_CONNECTED) {
		Serial.print(".");
		digitalWrite(LED_STATUS_PIN, HIGH);
		delay(250);
		digitalWrite(LED_STATUS_PIN, LOW);
		delay(250);
	}
	Serial.println();
	Serial.println("Connection established");
	Serial.print("IP address: ");
	localIP = WiFi.localIP();
	Serial.println(localIP.toString());
	broadcast = WiFi.localIP();
	broadcast[3] = 255;
	digitalWrite(LED_STATUS_PIN, HIGH);

	if (udp_sender.begin(port) == 1) {
		Serial.printf("UDP Service Started at port %d", port);
	}
	oneSec = millis();

}

void loop() {
	delay(50);
	changed = false;
	pressed = false;

	temp0 = adc_single_channel_read(adc_single_ch0);
	temp1 = adc_single_channel_read(adc_single_ch1);
	temp2 = adc_single_channel_read(adc_single_ch2);
	temp3 = adc_single_channel_read(adc_single_ch3);
	temp4 = adc_single_channel_read(adc_single_ch4);
	temp5 = adc_single_channel_read(adc_single_ch5);
	temp6 = adc_single_channel_read(adc_single_ch6);
	temp7 = adc_single_channel_read(adc_single_ch7);

	if (temp0 == 1023) {
		pressed = true;
	}
	if (pressed == false) {
		if (temp1 != adc_value_ch1) {

			adc_value_ch1 = temp1;
			Serial.printf("In1 %d \n", adc_value_ch1);
			setMesg.basePattern= temp1 / 4;
			if(!setMesg.basePattern)
				setMesg.basePattern = 1;
			changed = true;
		}

		if (temp2 != adc_value_ch2) {

			adc_value_ch2 = temp2;
			Serial.printf("In2 %d \n", adc_value_ch2);
			setMesg.mainDim=temp2 / 4;
			if(!setMesg.mainDim)
				setMesg.mainDim = 1;
			changed = true;
		}

		if (temp3 != adc_value_ch3) {
			digitalWrite(D0, HIGH);
			adc_value_ch3 = temp3;
			Serial.printf("In3 %d \n", adc_value_ch3);
			setMesg.strobeColor= temp3 / 4;
			if(!setMesg.strobeColor)
				setMesg.strobeColor = 1;
			changed = true;
		}

		if (temp4 != adc_value_ch4) {
			digitalWrite(D0, HIGH);
			adc_value_ch4 = temp4;
			Serial.printf("In4 %d \n", adc_value_ch4);
			setMesg.strobePattern= temp4 / 4;
			if(!setMesg.strobePattern)
				setMesg.strobePattern = 1;
			changed = true;
		}

		if (temp5 != adc_value_ch5) {
			digitalWrite(D0, HIGH);
			adc_value_ch5 = temp5;
			Serial.printf("In5 %d \n", adc_value_ch5);
			setMesg.frontColor = temp5 / 4;
			if(!setMesg.frontColor)
				setMesg.frontColor = 1;
			changed = true;
		}

		if (temp6 != adc_value_ch6) {
			digitalWrite(D0, HIGH);
			adc_value_ch6 = temp6;
			Serial.printf("In6 %d \n", adc_value_ch6);
			setMesg.frontPattern= temp6 / 4;
			if(!setMesg.frontPattern)
				setMesg.frontPattern = 1;
			changed = true;
		}

		if (temp7 != adc_value_ch7) {
			digitalWrite(D0, HIGH);
			adc_value_ch7 = temp7;
			Serial.printf("In7 %d \n", adc_value_ch7);
			setMesg.baseColor = temp7 / 4;
			if(!setMesg.baseColor)
				setMesg.baseColor = 1;
			changed = true;

		}
	}
	else{
		if (temp1 != adc_value_ch1) {

			adc_value_ch1 = temp1;
			Serial.printf("In1 %d \n", adc_value_ch1);
			setMesg.baseBrightness= temp1 / 4;
			if(!setMesg.baseBrightness)
				setMesg.baseBrightness = 1;
			changed = true;
		}

		if (temp2 != adc_value_ch2) {
			adc_value_ch2 = temp2;
			Serial.printf("In2 %d \n", adc_value_ch2);
			setMesg.dutyCycle =  temp2 / 4;
			if(!setMesg.dutyCycle)
				setMesg.dutyCycle = 1;
			changed = true;
		}
		if (temp3 != adc_value_ch3) {
			digitalWrite(D0, HIGH);
			adc_value_ch3 = temp3;
			Serial.printf("In3 %d \n", adc_value_ch3);
			setMesg.strobeSpeed= temp3 / 4;
			if(!setMesg.strobeSpeed)
				setMesg.strobeSpeed = 1;
			changed = true;
		}

		if (temp4 != adc_value_ch4) {
			digitalWrite(D0, HIGH);
			adc_value_ch4 = temp4;
			Serial.printf("In4 %d \n", adc_value_ch4);
			setMesg.strobeBrightness= temp4 / 4;
			if(!setMesg.strobeBrightness)
				setMesg.strobeBrightness = 1;
			changed = true;
		}

		if (temp5 != adc_value_ch5) {
			digitalWrite(D0, HIGH);
			adc_value_ch5 = temp5;
			Serial.printf("In5 %d \n", adc_value_ch5);
			setMesg.frontSpeed = temp5 / 4;
			if(!setMesg.frontSpeed)
				setMesg.frontSpeed = 1;
			changed = true;
		}

		if (temp6 != adc_value_ch6) {
			digitalWrite(D0, HIGH);
			adc_value_ch6 = temp6;
			Serial.printf("In6 %d \n", adc_value_ch6);
			setMesg.frontBrightness= temp6 / 4;
			if(!setMesg.frontBrightness)
				setMesg.frontBrightness = 1;
			changed = true;
		}

		if (temp7 != adc_value_ch7) {
			digitalWrite(D0, HIGH);
			adc_value_ch7 = temp7;
			Serial.printf("In7 %d \n", adc_value_ch7);
			setMesg.baseSpeed = temp7 / 4;
			if(!setMesg.baseSpeed)
				setMesg.baseSpeed = 1;
			changed = true;

		}


	}

	if(changed){
		udp_sender.beginPacket(broadcast, port);
		udp_sender.write(setMesg.buffer);
		for(int i = 0; i < setMesg.maxSize; i++){
			/*Serial.print(" ");
			Serial.print(setMesg.buffer[i]);*/
		}
		Serial.println();
		udp_sender.endPacket();
		Serial.println("Send Package");
	}

	if(millis() - oneSec > 1000){
		udp_sender.beginPacket(broadcast, port);
		udp_sender.write(setMesg.buffer);
		for(int i = 0; i < setMesg.maxSize; i++){
			Serial.print(" ");
			Serial.print(setMesg.buffer[i]);
		}
		Serial.println();
		udp_sender.endPacket();
		Serial.println("Send Package");
		oneSec = millis();
	}

	delay(50);
	digitalWrite(D0, LOW);

}

int adc_single_channel_read(byte readAddress) {

	byte dataMSB = 0;
	byte dataLSB = 0;
	byte JUNK = 0x00;

	SPI.beginTransaction(MCP3008);
	digitalWrite(CS_MCP3008, LOW);
	SPI.transfer(0x01);                                 // Start Bit
	dataMSB = SPI.transfer(readAddress << 4) & 0x03; // Send readAddress and receive MSB data, masked to two bits
	dataLSB = SPI.transfer(JUNK);      // Push junk data and get LSB byte return
	digitalWrite(CS_MCP3008, HIGH);
	SPI.endTransaction();

	return dataMSB << 8 | dataLSB;

}
