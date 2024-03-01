#include <FastLED.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>
#include "cyrillic.h"
#include "photometrics.h"

#define LED_PIN 2
#define NUM_LEDS 8
#define STEP_TEMPERATURE 250
#define STEP_WAVELENGTH 5
#define STEP_STROBE 1
#define POTENT_MIN 10
#define POTENT_MAX 1005
#define TIMESTEP 20
#define potentPin A0
#define buttonPin A1
#define photoPin A2

short mode;
int step;
int value, oldValue;

int lcdOnDelay = 5000;

long time;
long strobeTime;

RtcDateTime now;

CRGB leds[NUM_LEDS];
LiquidCrystal_I2C lcd(0x27, 16, 2);
ThreeWire myWire(7, 6, 8);        // DAT, CLK, RST
RtcDS1302<ThreeWire> rtc(myWire);    // RTC Object

void setup() {
  mode = 0;
  step = STEP_TEMPERATURE;
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS).setCorrection(0xFF5270);
  rtc.Begin();
  lcd.init();
  lcd.createChar(0, customChar3);
  lcd.createChar(1, customChar4);
  lcd.createChar(2, customChar6);
  lcd.createChar(3, customChar15);
  lcd.backlight();
  Serial.begin(9600);
}

void loop() {
  int potentiometer = constrain(analogRead(potentPin), 15, 980);
  oldValue = value;
  if (analogRead(buttonPin) > 800) {
    switchMode();
    delay(500);
  }
  if (mode == 0) {
    now = rtc.GetDateTime();
    refreshLCD();
  }
  else if (mode == 1) {
    value = map(potentiometer, POTENT_MAX, POTENT_MIN, 1000, 6500);
    value = round(value / step) * step;
    kelvinToRgb(value);
  }
  else if (mode == 2) {
    value = map(potentiometer, POTENT_MAX, POTENT_MIN, 785, 380);
    value = round(value / step) * step;
    nmToRgb(value);
  }
  else if (mode == 3) {
    value = map(potentiometer, POTENT_MAX, POTENT_MIN, 1, 50);
    value = round(value / step) * step;
    if ((millis() - time) >= 500.0 / value) {
      if (intensity == 255) {
        intensity = 0;
      } else {
        intensity = 255;
      }
      time = millis();
      red = intensity;
      green = intensity;
      blue = intensity;
      refreshLEDs();
    }
  }
  if (abs(value - oldValue) >= step) {
    if (mode != 0) {
      lcd.backlight();
      refreshLEDs();
      time = millis();
    }
    refreshLCD();
  }
  if ((millis() - time) >= lcdOnDelay && mode != 0)
    lcd.noBacklight();
  Serial.print("Mode is ");
  Serial.print(mode);
  Serial.print(", light ");
  Serial.print(analogRead(photoPin));
  Serial.print(", potentiometer ");
  Serial.println(analogRead(potentPin));
  delay(TIMESTEP);
}

void refreshLEDs() {
  for (int i = 0; i <= NUM_LEDS; i++) {
    leds[i] = CRGB(red, green, blue);
    FastLED.show();
  }
}

void refreshLCD() {
  if (mode == 0) {
    lcd.home();
    char str[16];
    sprintf(str, "%02d/%02d/%04d %02d:%02d", now.Day(), now.Month(), now.Year(), now.Hour(), now.Minute());
    lcd.print(str);
  }
  else if (mode == 1) {
    lcd.clear();
    lcd.setCursor(5, 0);
    lcd.print(value);
    lcd.print(" K");
    kelvinToRgb(value);
    //  lcd.setCursor(0, 1);
    //  char str[16];
    //  sprintf(str, "(%d, %d, %d)", red, green, blue);
    //  lcd.print(str);
  }
  else if (mode == 2) {
    lcd.clear();
    lcd.setCursor(5, 0);
    lcd.print(value);
    lcd.print(" nm");
    nmToRgb(value);

  }
  else if (mode == 3) {
    lcd.clear();
    lcd.setCursor(5, 0);
    lcd.print(value);
    lcd.createChar(4, customChar2);
    lcd.setCursor(8, 0); // move cursor to (2, 0)
    lcd.write((byte)4);  // print the custom char at (2, 0)
    lcd.createChar(5, customChar12);
    lcd.setCursor(9, 0); // move cursor to (2, 0)
    lcd.write((byte)5);  // print the custom char at (2, 0)
  }
}

void switchMode() {
  mode++;
  if (mode > 3) {
    mode = 0;
  }
  if (mode == 1) {
    step = STEP_TEMPERATURE;
  }
  else if (mode == 2) {
    step = STEP_WAVELENGTH;
  }
  else if (mode == 3) {
    step = STEP_STROBE;
  }
}
