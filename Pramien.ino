#include <FastLED.h>
#include <Wire.h>
#include <Adafruit_SH1106.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>
#include "photometrics.h"

#define OLED_RESET 4
Adafruit_SH1106 display(OLED_RESET);

#define STEP_TEMPERATURE 200
#define STEP_WAVELENGTH 2.5
#define POTENT_MIN 0
#define POTENT_MAX 1024
#define TIMESTEP 25
#define potentPin A0
#define buttonPin A1
#define photoPin A2

const char dayInWords[7][13] = {"NIADZIELIA", "PANIADZIELAK", "AUTORAK", "SERADA", "CZACWIER", "PIATNICA", "SUBOTA"};

short mode;
float step;
float value, oldValue;

int lcdOnDelay = 5000;

long time;

RtcDateTime now;
ThreeWire myWire(6, 7, 5);        // DAT, CLK, RST
RtcDS1302<ThreeWire> rtc(myWire);    // RTC Object

#define LED_PIN 2
#define NUM_LEDS 8
CRGB leds[NUM_LEDS];

void setup() {
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS).setCorrection(0xFF5270);
  mode = 0;
  step = 0;
  rtc.Begin();

  //  RtcDateTime currentTime = RtcDateTime(__DATE__ , __TIME__);
  //  rtc.SetDateTime(currentTime);

  display.begin(SH1106_SWITCHCAPVCC, 0x3C);
  display.display();
  delay(500);

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.SH1106_command(0x81);
  display.setTextSize(3);
  display.println("PRAMIEN");
  display.setTextSize(1);
  display.println("electronic clock");
  display.display();

  delay(2000);

  Serial.begin(9600);
}

void loop() {
  int potentiometer = constrain(analogRead(potentPin), POTENT_MIN, POTENT_MAX);
  if (analogRead(buttonPin) > 800) {
    delay(75);
    if (analogRead(buttonPin) > 800)
      switchMode();
  }
  if (mode == 0) {
    now = rtc.GetDateTime();
    //refreshLCD();
  }
  else if (mode == 1) {
    oldValue = value;
    value = map(potentiometer, POTENT_MAX, POTENT_MIN, 1000, 6500);
    value = round(value / step) * step;
    kelvinToRgb(value);
  }
  else if (mode == 2) {
    oldValue = value;
    value = map(potentiometer, POTENT_MAX, POTENT_MIN, 780, 380);
    value = round(value / step) * step;
    nmToRgb(value);
  }
  if (abs(value - oldValue) >= step) {
    refreshLEDs();
    time = millis();
  }
  refreshLCD();
  //  if ((millis() - time) >= lcdOnDelay) {
  //    if (mode == 0) {
  //      refreshLCD();
  //      time = millis();
  //    }
  //    else display.clearDisplay();
  //  }

  Serial.print("Mode: ");
  Serial.print(mode);
  Serial.print(", step=");
  Serial.print(step);
  Serial.print(", light=");
  Serial.print(analogRead(photoPin));
  Serial.print(", potentiometer=");
  Serial.print(analogRead(potentPin));
  Serial.print(", oldvalue=");
  Serial.print(oldValue);
  Serial.print(", value=");
  Serial.println(value);

  delay(TIMESTEP);
}

void refreshLEDs() {
  for (int i = 0; i <= NUM_LEDS; i++) {
    leds[i] = CRGB(red, green, blue);
    FastLED.show();
  }
}

void refreshLCD() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  if (mode == 0) {
    char str1[8];
    sprintf(str1, " %02d:%02d", now.Hour(), now.Minute(), now.Second());
    display.setTextSize(3);
    display.print(str1);
    char str2[10];
    sprintf(str2, "%02d/%02d/%04d", now.Day(), now.Month(), now.Year());
    display.setTextSize(2);
    display.println();
    display.println();
    display.println(str2);
    //    display.println(dayInWords[0]);
  }
  else if (mode == 1) {
    display.print(value);
    display.println(" K");
    kelvinToRgb(value);
    showRGB();
  }
  else if (mode == 2) {
    display.print(value);
    display.println(" nm");
    nmToRgb(value);
    showRGB();
  }
  display.display();
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
    refreshLEDs();
    refreshLCD();
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
  display.setTextSize(1);
  display.println();
  char str[20];
  sprintf(str, "RGB (%d, %d, %d)", red, green, blue);
  display.println(str);
  display.print("photosens. ");
  display.println(analogRead(photoPin) * 5.0);
}
