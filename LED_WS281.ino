#include <FastLED.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>
#include "cyrillic.h"

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
int intensity = 255;
int red, green, blue;
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

void kelvinToRgb(int temperature) {
  temperature = constrain(temperature, 1000, 40000);
  temperature /= 100;
  if (temperature <= 66)
    red = intensity;
  else
    red = (int) round(329.7 * (pow(temperature - 60, -0.13)));
  red = constrain(red, 0, intensity);
  if (temperature <= 66)
    green = (int) round(99.5 * log(temperature) - 161.12);
  else
    green = (int) round(288.1 * (pow(temperature - 60, -0.08)));
  green = constrain(green, 0, intensity);
  if (temperature >= 66)
    blue = intensity;
  else if (temperature <= 19)
    blue = 0;
  else
    blue = (int) round(138.5 * log(temperature - 10) - 305.04);
  blue = constrain(blue, 0, intensity);
}

void nmToRgb(int wavelength) {
  float gamma = 0.80;
  float factor;
  float r, g, b;
  if ((wavelength >= 380.0) && (wavelength < 440.0)) {
    r = -(wavelength - 440.0) / (440.0 - 380.0);
    g = 0.0;
    b = 1.0;
  } else if ((wavelength >= 440.0) && (wavelength < 490.0)) {
    r = 0.0;
    g = (wavelength - 440.0) / (490.0 - 440.0);
    b = 1.0;
  } else if ((wavelength >= 490.0) && (wavelength < 510.0)) {
    r = 0.0;
    g = 1.0;
    b = -(wavelength - 510.0) / (510.0 - 490.0);
  } else if ((wavelength >= 510.0) && (wavelength < 580.0)) {
    r = (wavelength - 510.0) / (580.0 - 510.0);
    g = 1.0;
    b = 0.0;
  } else if ((wavelength >= 580.0) && (wavelength < 645.0)) {
    r = 1.0;
    g = -(wavelength - 645.0) / (645.0 - 580.0);
    b = 0.0;
  } else if ((wavelength >= 645.0) && (wavelength < 781.0)) {
    r = 1.0;
    g = 0.0;
    b = 0.0;
  } else {
    r = 0.0;
    g = 0.0;
    b = 0.0;
  };
  // Let the intensity fall off near the vision limits
  if ((wavelength >= 380.0) && (wavelength < 420.0)) {
    factor = 0.3 + 0.7 * (wavelength - 380.0) / (420.0 - 380.0);
  } else if ((wavelength >= 420.0) && (wavelength < 701.0)) {
    factor = 1.0;
  } else if ((wavelength >= 701.0) && (wavelength < 781.0)) {
    factor = 0.3 + 0.7 * (780.0 - wavelength) / (780.0 - 700.0);
  } else {
    factor = 0.0;
  };
  if (r != 0) {
    r = round(intensity * pow(r * factor, gamma));
  }
  if (g != 0) {
    g = round(intensity * pow(g * factor, gamma));
  }
  if (b != 0) {
    b = round(intensity * pow(b * factor, gamma));
  };
  red = constrain(r, 0, intensity);
  green = constrain(g, 0, intensity);
  blue = constrain(b, 0, intensity);
}

void refreshLEDs() {
  for (int i = 0; i <= NUM_LEDS; i++) {
    leds[i] = CRGB(red, green, blue);
    FastLED.show();
  }
}

void refreshLCD() {
  if (mode == 0) {
    lcd.setCursor(4, 0);
    lcd.print(now.Day());
    lcd.print("/");
    lcd.print(now.Month());
    lcd.print("/");
    lcd.print(now.Year());

    lcd.setCursor(5, 1);
    lcd.print(now.Hour());
    lcd.print(":");
    lcd.print(now.Minute());
    lcd.print(":");
    lcd.print(now.Second());
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
