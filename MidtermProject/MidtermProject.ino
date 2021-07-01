/*
   Midterm Project
   Candy Dispenser
   Chad Waples
   6/30/2021
*/

#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Keypad.h>
#include <PWMServo.h>
#include <Adafruit_NeoPixel.h>
#include <colors.h>
#include <math.h>
#include "tones.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

File dataFile;

//PIR Proximity infrared
int pirPin = 21;
int pirStat = 0;


//Keypad
const byte ROWS = 4;
const byte COLS = 4;
char customKey;
char codeStore[7];
char password[7] = "8675309";
int reset1 = 0;
bool correct;
bool dispense = true;

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2};
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

//NeoPixels
const int PIXELPIN = 23;
const int PIXELCOUNT = 40;
const int maxPos = 39;
const int basePos = 6;
int neoPixelPos;
int randomColor;
int i;
int r;
int position;
Adafruit_NeoPixel pixel(40, 23, NEO_GRB + NEO_KHZ800);

//Music
const int BUZZER_PIN = A0;
const int NOTE_COUNT = 11;
const int TIMES_PLAYED = 2;
int melody[] = {
  NOTE_CS6, NOTE_CS6,
  NOTE_D6, NOTE_D6, NOTE_D6,
  NOTE_CS6, NOTE_CS6,
  NOTE_B5, NOTE_CS6, NOTE_B5, NOTE_A5
};

int noteDurations[] = {
  2, 2,
  4, 4, 4,
  2, 2,
  5, 4, 4, 2
};

//Servo Motor
PWMServo myServo;
const int SERVOPIN = 22;
int pos = 0;

//BME280
float tempC;
float tempF;
Adafruit_BME280 bme;
unsigned status;


void setup() {
  Serial.begin (9600);
  pinMode (pirPin, INPUT);
  status = bme.begin(0x76);


  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  display.display();
  delay(1000);
  testdrawstyles1();

  pixel.begin();
  pixel.show();
  myServo.attach(SERVOPIN);
  tempC = bme.readTemperature();
  tempF = (tempC * 1.8) + 32;
  tempPixel();
  i = 0;
  r = 40;
}

void loop() {
  pirStat = digitalRead(pirPin);
  if (pirStat == HIGH) {
    Serial.println("Hey there");
    display.clearDisplay();
    display.display();
    testdrawstyles2();
  }
  
  while (i < 7) {
    customKey = customKeypad.getKey();
    if (customKey) {
      Serial.println(customKey);
      codeStore[i] = customKey;
      i++;
    }
  }
  

  //  myServo.write(0);
  //  delay(1000);
  //  myServo.write(180);


  delay(5000);
  showPixel();
  randomColor = random(0x000000, 0xFFFFFF);

  for (int j = 0; j < TIMES_PLAYED; j++) {
    //    playNotes();
  }
  //  while (true); //ends the music and lights.
}



void showPixel() {
  for (int i = 0; i < PIXELCOUNT; i++) {
    pixel.clear();
    delay (100);
    pixel.fill(random(0x000000, 0xFFFFFF), i, 7);
    pixel.setBrightness(random(0, 60));
    pixel.show();
  }

  pixel.clear();
  pixel.show();


  for (int r = 40; r >= 0; r--) {
    pixel.clear();
    delay (100);
    pixel.fill(random(0x000000, 0xFFFFFF), r, 7);
    pixel.setBrightness(random(0, 60));
    pixel.show();
  }
  pixel.clear();
}

void tempPixel() {

  position = tempF;
  if (position >= 120) {
    position = (maxPos);
  }
  if (position < 0) {
    position = (basePos);
  }

  neoPixelPos = map(position, 0, 120, 6, 39);
  pixel.fill(0xFF0000, 0, neoPixelPos);
  pixel.setBrightness(150);
  pixel.show();
}

void testdrawstyles1(void) {
  display.clearDisplay();

  display.setRotation(0);  // "display.setRotation();

  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0, 0);            // Start at top-left corner
  display.printf("Want some candy");
  
  display.setTextSize(2);             // Draw 2X-scale text

  display.display();
  //  delay(2000);
}

void testdrawstyles2(void) {
  display.clearDisplay();

  display.setRotation(0);  // "display.setRotation();

  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0, 0);            // Start at top-left corner
  display.printf("What is the number?");
  
  display.setTextSize(2);             // Draw 2X-scale text

  display.display();
  //  delay(2000);
}

void playNotes() {
  // iterate over the notes of the melody:
  for (int thisNote = 0; thisNote < NOTE_COUNT; thisNote++) {

    // to calculate the note duration, take one second
    // divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(BUZZER_PIN, melody[thisNote], noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.1;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(BUZZER_PIN);
  }
}

bool validate() {
  for (int i = 0; i < 7; i++) {
    if (codeStore[i] != password[i]) {
      return false;
    }
    return true;
  }
}
