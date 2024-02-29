#include <LiquidCrystal_I2C.h>
#include <ThreeWire.h>
#include <RtcDS1302.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);   //LCD Object

ThreeWire myWire(7, 6, 8);        // DAT, CLK, RST
RtcDS1302<ThreeWire> Rtc(myWire);    // RTC Object

void setup ()
{
  lcd.init();
  lcd.backlight();
  lcd.clear();

  Rtc.Begin();

  // RtcDateTime currentTime = RtcDateTime(__DATE__ , __TIME__);
  // Rtc.SetDateTime(currentTime);
}

void loop ()
{
  RtcDateTime now = Rtc.GetDateTime();
  lcd.home();
  lcd.print("Date: ");
  lcd.print(now.Day());
  lcd.print("/");
  lcd.print(now.Month());
  lcd.print("/");
  lcd.print(now.Year());

  lcd.setCursor(0, 1);
  lcd.print("Time: ");
  lcd.print(now.Hour());
  lcd.print(":");
  lcd.print(now.Minute());
  lcd.print(":");
  lcd.print(now.Second());
  
  delay(500);
}
