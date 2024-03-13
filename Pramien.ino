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
#define POTENT_MIN 10
#define POTENT_MAX 1005
#define TIMESTEP 20
#define potentPin A0
#define buttonPin A1
#define photoPin A2

short mode;
int step;
float value, oldValue;

int lcdOnDelay = 5000;

long time;

RtcDateTime now;

CRGB leds[NUM_LEDS];
LiquidCrystal_I2C lcd(0x27, 16, 2);
ThreeWire myWire(7, 6, 8);        // DAT, CLK, RST
RtcDS1302<ThreeWire> rtc(myWire);    // RTC Object

void setup() {
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS).setCorrection(0xFF5270);
  mode = 0;
  step = 0;
  rtc.Begin();
  lcd.init();
  lcd.createChar(0, customChar3);
  lcd.createChar(1, customChar4);
  lcd.createChar(2, customChar6);
  lcd.createChar(3, customChar15);
  lcd.backlight();
  
//  Serial.begin(9600);
}

void loop() {
  int potentiometer = constrain(analogRead(potentPin), 15, 980);
  if (analogRead(buttonPin) > 800) {
    delay(100);
    if (analogRead(buttonPin) > 800)
      switchMode();
  }
  if (mode == 0) {
    now = rtc.GetDateTime();
  }
  else if (mode == 1) {
    oldValue = value;
    value = map(potentiometer, POTENT_MAX, POTENT_MIN, 1000, 6500);
    value = round(value / step) * step;
    kelvinToRgb(value);
  }
  else if (mode == 2) {
    oldValue = value;
    value = map(potentiometer, POTENT_MAX, POTENT_MIN, 785, 380);
    value = round(value / step) * step;
    nmToRgb(value);
  }
  if (abs(value - oldValue) >= step && mode != 0) {
    lcd.backlight();
    refreshLEDs();
    Serial.println("Refreshed: LEDS");
    time = millis();
    refreshLCD();
  }
  if ((millis() - time) >= lcdOnDelay) {
    if (mode == 0) {
      refreshLCD();
      time = millis();
    }
    else lcd.noBacklight();
  }

//  Serial.print("Mode: ");
//  Serial.print(mode);
//  Serial.print(", step=");
//  Serial.print(step);
//  Serial.print(", light=");
//  Serial.print(analogRead(photoPin));
//  Serial.print(", potentiometer=");
//  Serial.print(analogRead(potentPin));
//  Serial.print(", oldvalue=");
//  Serial.print(oldValue);
//  Serial.print(", value=");
//  Serial.println(value);

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
    showRGB();
  }
  else if (mode == 2) {
    lcd.clear();
    lcd.setCursor(5, 0);
    lcd.print(value);
    lcd.print(" nm");
    nmToRgb(value);
    showRGB();
  }
}

void switchMode() {
  mode++;
  if (mode > 2) {
    mode = 0;
  }
  if (mode == 0) {
    step = 0;
    red = 0;
    green = 0;
    blue = 0;
    lcd.clear();
    lcd.backlight();
    refreshLEDs();
  }
  else if (mode == 1) {
    step = STEP_TEMPERATURE;
    intensity = 255;
  }
  else if (mode == 2) {
    step = STEP_WAVELENGTH;
    intensity = 255;
  }
}

void showRGB() {
  lcd.setCursor(0, 1);
  char str[16];
  sprintf(str, "(%d, %d, %d)", red, green, blue);
  lcd.print(str);
}
