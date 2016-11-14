//#define FASTLED_ESP8266_NODEMCU_PIN_ORDER
#define FASTLED_ESP8266_RAW_PIN_ORDER
#define DEBUGMSG
#include <FastLED.h>
#include <bitswap.h>
#include <chipsets.h>
#include <color.h>
#include <colorpalettes.h>
#include <colorutils.h>
#include <controller.h>
#include <cpp_compat.h>
#include <dmx.h>
#include <fastled_config.h>
#include <fastled_delay.h>
#include <fastled_progmem.h>
#include <fastpin.h>
#include <fastspi.h>
#include <fastspi_bitbang.h>
#include <fastspi_dma.h>
#include <fastspi_nop.h>
#include <fastspi_ref.h>
#include <fastspi_types.h>
#include <hsv2rgb.h>
#include <led_sysdefs.h>
#include <lib8tion.h>
#include <noise.h>
#include <pixelset.h>
#include <pixeltypes.h>
#include <platforms.h>
#include <power_mgt.h>

#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "LedFunctions.h"
#include "Pattern.h"


#include "Protocol.h"

#define NUM_LEDS 28
#define DATA_PIN D7


#define LED_STATUS_PIN 1
#define MAX_PACKET_SIZE 32

CRGB leds[NUM_LEDS];
Pattern pattern(leds, NUM_LEDS);

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
const int ticker_port = 1103;				//UDP Port der abgehört wird

boolean animal_selected = false;
boolean animation_running = false;

int status = WL_IDLE_STATUS;

IPAddress remoteIP;
unsigned int remotePort = 1103;
unsigned int packetSize = 0;
unsigned int commandCode = 48;

char recvBuffer[MAX_PACKET_SIZE];

WiFiUDP Udp;
timeMeasurer mess;

subscribeMessage subMesg;
synchronisingMessage syncMesg;
settingMessage setMesg;
pushingMessage pushMesg;
statusingMessage statMesg;

long millisSinceSync;
long lastSync;
long millisSinceBeat;
long lastBeat;
long millisSinceRequest;
long lastRequest;
long now;
long lastSave;

double x;

bool waitingForAck;
bool registered;

IPAddress brainIP;
IPAddress braodIP;

void debugMesg(const char* msg) {
#ifdef DEBUGMSG
	Serial.println(msg);
#endif
}

void setup() {
	pinMode(DATA_PIN, OUTPUT);
	digitalWrite(DATA_PIN, LOW);
	/*while(true){
		digitalWrite(DATA_PIN, HIGH);
		delay(5000);
		digitalWrite(DATA_PIN, LOW);
		delay(5000);
	}*/
	pinMode(D4, OUTPUT);
	delay(1000);
	Serial.begin(9600);
	//Set up Status Led
	debugMesg("Setting up Statusled");

	//Set up LEDS
	debugMesg("Setting up Leds");
	FastLED.addLeds<WS2811, DATA_PIN, BRG>(leds, NUM_LEDS);
	delay(1000);

	//Set up Wifi
	debugMesg("Setting up Wifi");
	WiFi.softAPdisconnect(true);
	WiFi.begin(ssid, password);

	while (WiFi.status() != WL_CONNECTED) {

		digitalWrite(D4, HIGH);
		fill_solid(leds, NUM_LEDS, CRGB::Red);
		FastLED.show();
		delay(250);

		digitalWrite(D4, LOW);
		fill_solid(leds, NUM_LEDS, CRGB::Black);
		FastLED.show();
		delay(250);
	}

	//Set up UDP-Listener
	debugMesg("Setting up Udp");
	Udp.begin(ticker_port);

	//Receivebuffer
	for (int i = 0; i < MAX_PACKET_SIZE; i++) {
		recvBuffer[i] = '0';
	}

	//Set vars

	millisSinceSync = 0;
	lastSync = 0;
	millisSinceBeat = 0;
	lastBeat = 0;
	millisSinceRequest = 0;
	lastRequest = 0;
	now = 0;

	waitingForAck = false;
	registered = false;

	fill_solid(leds, NUM_LEDS, CRGB::Black);
	FastLED.setCorrection(TypicalSMD5050);
	FastLED.show();
	pattern.savedVals.read();
	lastSave = 0;
	debugMesg("Setup finished");
}

void setTimes() {
	now = millis();
	millisSinceSync = now - lastSync;
	millisSinceBeat = now - lastBeat;
	millisSinceRequest = now - lastRequest;
	pattern.setMillisSinceBeat((double) millisSinceBeat);
	pattern.setStrobeTime(millis()- pattern.getStrobeStart());
}

void loop() {
	//Lade neue Zeiten für den Loop
	setTimes();
	if(millis() - lastSave > 30000){
		pattern.savedVals.write();
		lastSave = millis();
	}

	/*
	 * Kommunikationsteil
	 */
	//Schaue ob ein Packet angekommen ist und extrahiere Länge und remoteIP
	packetSize = Udp.parsePacket();
	remoteIP = Udp.remoteIP();

	//Lese das Packet und reagiere
	if (packetSize > 0) {
		if (packetSize <= MAX_PACKET_SIZE) {

			Udp.read(recvBuffer, packetSize);
			//Serial.print(recvBuffer);
			//Checke ob Synchronisierungspacket
			if (synchronising(recvBuffer, packetSize)) {
				syncMesg.create(recvBuffer, packetSize);
				if (syncMesg.direction == '0') {
					pattern.setBeatPeriodMillis((double)syncMesg.beat_period_millis);
					pattern.setBeatDistinctiveness((double)syncMesg.beat_distinctivness);
					pattern.setMillisSinceBeat(0);
					millisSinceBeat = 0;
					lastBeat = millis();
					/*Serial.print("Packetsize: ");
					Serial.print(packetSize);
					Serial.print(" BeatPeriod: ");
					Serial.print(syncMesg.beat_period_millis);
					Serial.print(" BeatDistinctiveness: ");
					Serial.print(syncMesg.beat_distinctivness);
					Serial.println();*/
					pattern.baseChoser();
					pattern.frontChoser();
					pattern.strobeChoser();
					FastLED.setCorrection(TypicalSMD5050);
					FastLED.show((uint8_t)pattern.getDimVal());

				}
			}
			if (setting(recvBuffer, packetSize)) {
				setMesg.create(recvBuffer, packetSize);
				pattern.setNbasePattern((double) setMesg.basePattern);
				pattern.setNbaseColor((double) setMesg.baseColor);
				pattern.setNbaseSpeed((double) setMesg.baseSpeed);
				pattern.setNbaseDim((double) setMesg.baseBrightness);
				pattern.setNfrontPattern((double) setMesg.frontPattern);
				pattern.setNfrontColor((double) setMesg.frontColor);
				pattern.setNfrontSpeed((double) setMesg.frontSpeed);
				pattern.setNfrontDim((double) setMesg.frontBrightness);
				pattern.setNstrobePattern((double) setMesg.strobePattern);
				pattern.setNstrobeColor((double) setMesg.strobeColor);
				pattern.setNstrobeSpeed((double) setMesg.strobeSpeed);
				pattern.setNstrobeDim((double) setMesg.strobeBrightness);
				pattern.setDimVal((double) setMesg.mainDim);
				pattern.setDutyCycle(((double) setMesg.dutyCycle ));
				pattern.setSettings();
				pattern.setStrobeStart(millis());
				/*Serial.println("Set Message:\t");
				Serial.print("Basepattern:\t");
				Serial.println(pattern.getNbasePattern());
				Serial.print("Basecolor:\t");
				Serial.println(pattern.getNbaseColor());
				Serial.print("Basespeed:\t");
				Serial.println(pattern.getNbaseSpeed());
				Serial.print("Basedim:\t");
				Serial.println(pattern.getNbaseDim());
				Serial.print("Frontpattern:\t");
				Serial.println(pattern.getNfrontPattern());
				Serial.print("Frontcolor:\t");
				Serial.println(pattern.getNfrontColor());
				Serial.print("Frontspeed:\t");
				Serial.println(pattern.getNfrontSpeed());
				Serial.print("Frontdim:\t");
				Serial.println(pattern.getNfrontDim());
				Serial.print("Strobepattern:\t");
				Serial.println(pattern.getNstrobePattern());
				Serial.print("Strobecolor:\t");
				Serial.println(pattern.getNstrobeColor());
				Serial.print("StrobeSpeed:\t");
				Serial.println(pattern.getNstrobeSpeed());
				Serial.print("StrobeDim:\t");
				Serial.println(pattern.getNstrobeDim());
				Serial.print(" dutyCycle:\t");
				Serial.println(pattern.getDutyCycle());
				Serial.print(" mainDim:\t");
				Serial.println(pattern.getDimVal());
				Serial.println();*/

			}

		}

	}
	/*
	 * Lichtteil
	 */
	//Ausnahmen
	//pattern.fillBlack();
	pattern.baseChoser();
	pattern.frontChoser();
	pattern.strobeChoser();
	FastLED.setCorrection(TypicalSMD5050);
	FastLED.show((uint8_t)pattern.getDimVal());
}

