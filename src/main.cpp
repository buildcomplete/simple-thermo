/*-----( Import needed libraries )-----*/
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "DHT.h"

#include <SPI.h>
#include <SD.h>

#define BUTTON 2      // Button used to turn on display backlight
#define RELAY1_PIN 4  // relay controlled by sensor 2
#define RELAY2_PIN 5  // relay controlled by sensor 1
#define STATUS_PIN 6  // a status indicator pin indicating the loop is running 
#define DHTPIN1 7     // what pin we're connected to
#define DHTPIN2 8     // what pin we're connected to
#define DHTPIN3 9     // what pin we're connected to
#define DHTTYPE DHT11   // DHT 11

/*-----( Declare Constants )-----*/
// change this to match your SD shield or module;
// Arduino Ethernet shield: pin 4
// Adafruit SD shields and modules: pin 10
// Sparkfun SD shield: pin 8
const int chipSelect = 10;
const int logDelay = 20; // Log every 'logDelay' cycle

LiquidCrystal_I2C lcd(0x3f, 20, 4);  // Set the LCD I2C address

// Variables for controlling the relay with hysteresis.
int measuredTemperatures[3] = {0,0,0}; // measured temperatures (we have 3 sensors...)
int measuredHumidity[3] = {0,0,0}; // measured humidities (we have 3 sensors, these are stored to streamline log)
int relayPins[2] = {RELAY1_PIN, RELAY2_PIN}; // The relay pins
int relayOnTemp[2] = {7,7}; // Temperature in celcius to turn on relay
int relayOffTemp[2] = {11,11};  // Temperature in celcius to turn off relay
int relayState[2] = {LOW,LOW}; // Initial relay state

/*-----( Declare Variables )-----*/
DHT dht1(DHTPIN1, DHTTYPE);
DHT dht2(DHTPIN2, DHTTYPE);
DHT dht3(DHTPIN3, DHTTYPE);

// set up variables using the SD utility library functions:
File dataFile;
int  logDelayCounter = 0;

void setup()   /*----( SETUP: RUNS ONCE )----*/
{
	pinMode(BUTTON, INPUT);
	for (int i=0;i<2;++i)
	{	
		pinMode(relayPins[i], OUTPUT);
		digitalWrite(relayPins[i], relayState[i]);
	}
	pinMode(STATUS_PIN, OUTPUT);
	Serial.begin(9600);  // Used to type in characters
	dht1.begin();
	dht2.begin();
	dht3.begin();

	lcd.init();
	lcd.begin(20,4);         // initialize the lcd for 20 chars 4 lines, turn on backlight
	lcd.backlight();
	
	// On the Ethernet Shield, CS is pin 4. It's set as an output by default.
	// Note that even if it's not used as the CS pin, the hardware SS pin 
	// (10 on most Arduino boards, 53 on the Mega) must be left as an output 
	// or the SD library functions will not work. 
	pinMode(SS, OUTPUT);
  
	// we'll use the initialization code from the utility libraries
	// since we're just testing if the card is working!
	Sd2Card card;
	while (!card.init(SPI_HALF_SPEED, chipSelect)) {
		lcd.clear();
		lcd.setCursor(0,0);
		lcd.printstr("SD-init failed. check:");
		lcd.setCursor(0,1);
		lcd.printstr("*card inserted?");
		lcd.setCursor(0,2);
		lcd.printstr("*wiring correct?");
		lcd.setCursor(0,3);
		lcd.printstr("*chipSel correct?");
	} 
	
	lcd.clear();
	lcd.setCursor(0,0);
	// print the type of card
	lcd.printstr("Card type: ");
	switch(card.type()) {
		case SD_CARD_TYPE_SD1:
			lcd.printstr("SD1");
			break;
		case SD_CARD_TYPE_SD2:
			lcd.printstr("SD2");
			break;
		case SD_CARD_TYPE_SDHC:
			lcd.printstr("SDHC");
			break;
		default:
			lcd.printstr("Unknown");
	}
	delay(1000);

	if (!SD.begin(chipSelect)) {
		lcd.printstr("Card failed, or not present");
		// don't do anything more:
		while (1)
			delay(1000);
		
	}
	
	// Open up the file we're going to log to!
	dataFile = SD.open("datalog.tsv", FILE_WRITE);
	if (! dataFile) {
		lcd.printstr("error opening datalog.txt");
		// Wait forever since we cant write data
		while (1) 
			delay(1000);
	}
	dataFile.println("***New Measurement***");
	
	lcd.clear();

}/*--(end setup )---*/

bool status = false;
char indicator[] = {'.', ' '};
int indicatorIdx = 0;


void loop()   /*----( LOOP: RUNS CONSTANTLY )----*/
{
	// Make a output blink to show that the loop is processing.
	digitalWrite(STATUS_PIN, status);
	status = !status;

	lcd.backlight( );
	
	// Wait a few seconds between measurements.
	delay(1000);
	DHT sensors[3] = {dht1, dht2, dht3};
	  
	lcd.setCursor(0,0);
	lcd.write(indicator[indicatorIdx]);
	++indicatorIdx;
	indicatorIdx = indicatorIdx % 2;
	
	lcd.printstr("  s1  s2  s3");
	lcd.setCursor(0,1);
	lcd.printstr("T");
	lcd.setCursor(0,2);
	lcd.printstr("H");
	lcd.setCursor(0,3);
	lcd.print("Releay");
	for (int i=0;i<2;++i)
	{
		lcd.print(" ");
		lcd.print(i+1);
		lcd.print("=");
		lcd.print((relayState[i] == HIGH) ? "On " : "Off");
	}
	int colIndexMeasurement[3] = {3, 7, 11};
	
	for (int sensorId = 0; sensorId < 3; ++sensorId)
	{
		DHT dht = sensors[sensorId];

		// Reading temperature or humidity takes about 250 milliseconds!
		// Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
		
		lcd.setCursor(colIndexMeasurement[sensorId], 1);
		
		float t = dht.readTemperature();
		
		// clear 'NAN'
		lcd.printstr("   ");
		lcd.setCursor(colIndexMeasurement[sensorId], 1);
		
		if ( ! isnan(t))
		{
			lcd.print((int)t);
			measuredTemperatures[sensorId] = (int)t;
		}
		else
		{
			lcd.printstr("NAN");
		}

		lcd.setCursor(colIndexMeasurement[sensorId], 2);
		
		float h = dht.readHumidity();
		
		// clear 'NAN'
		lcd.printstr("   ");
		lcd.setCursor(colIndexMeasurement[sensorId], 2);
		if ( ! isnan(h))
		{
			lcd.print((int)h);
			measuredHumidity[sensorId] = (int)h;
		}
		else
		{
			lcd.printstr("NAN");
		}
	}
	
	// Update relay outputs
	for (int i=0;i<2;++i)
	{		
		if (relayState[i] == LOW // Is relay off 
			&& measuredTemperatures[i] <= relayOnTemp[i] ) // and temperature below or equal to 'OnTemp'
		{
			// Turn on relay
			relayState[i] = HIGH;
			digitalWrite(relayPins[i], relayState[i]);
		}
		else if (relayState[i] == HIGH // Is relay on
			&& measuredTemperatures[i] > relayOffTemp[i]) // And temperature is above 'OffTemp'
		{ 
			// Turn off relay
			relayState[i] = LOW;
			digitalWrite(relayPins[i], relayState[i]);
		}		
	}
	
	if (logDelayCounter == 0)
	{
		lcd.setCursor(19,0);
		lcd.printstr("+");
		
		// Write to log file
		for (int sensorId=0;sensorId<3;++sensorId)
		{
			dataFile.print(sensorId);
			dataFile.print("\t");
			dataFile.print(measuredTemperatures[sensorId]);
			dataFile.print("\t");
			dataFile.print(measuredHumidity[sensorId]);
			dataFile.print("\t");
		}		
		for (int i=0;i<2;++i)
		{
			dataFile.print(relayState[i]);
			dataFile.print("\t");
		}
		
		
		dataFile.println();
		dataFile.flush();

		logDelayCounter = logDelay+1;		
	}
	else if (logDelayCounter == logDelay)
	{
		lcd.setCursor(19,0);
		lcd.printstr(" ");
	}
	--logDelayCounter;
  
}/* --(end main loop )-- */


/* ( THE END ) */

