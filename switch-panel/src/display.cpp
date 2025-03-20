#include "display.h"
#include "globals.h"
#include "sensors.h"  // Für getTemp() und RTC
#include <Arduino.h>

// Globales Display-Objekt – verwendet die in globals.h definierten Pins
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(
  U8G2_R0, 
  DISPLAY_CLOCK_PIN, 
  DISPLAY_DATA_PIN, 
  DISPLAY_RESET_PIN
);

// Dynamische Zustandsvariablen, die zur Displaylogik gehören
bool blogo = false;
unsigned long standyTimeStamp = 0;

void displaySetup() {
  u8g2.begin();
}

void displayLoop() {
  // Hier könntest du z. B. abhängig von Zuständen (aus switches.cpp) das Display aktualisieren.
  // Dieser Platzhalter lässt die Entscheidung extern (z. B. in switchesLoop) treffen.
}

void logo(bool engineOn) {
  u8g2.clearBuffer();

  // Wenn der Motor aus ist und das Cooldown abgelaufen ist, soll das Logo angezeigt werden.
  if (!engineOn && (millis() > COOLDOWN + standyTimeStamp)) {
    blogo = true;
    clockTime();
    temperature();
  } else {
    blogo = false;
  }

  if (!blogo) {
    // Beispielhafte Darstellung: Zeichne mehrere Kreise
    u8g2.drawCircle(64, 32, 31);
    u8g2.drawCircle(64, 32, 28);
    u8g2.drawCircle(64, 32, 27);
    u8g2.drawCircle(64, 32, 26);
  }
  
  // Optional: Debug-Warnsymbol zeichnen
  // warning();
  
  u8g2.sendBuffer();
}

void warning() {
  // Einfaches Warnsymbol (Beispiel)
  u8g2.drawLine(2, 20, 10, 2);
  u8g2.drawLine(2, 20, 18, 20);
  u8g2.drawLine(10, 2, 18, 20);
  u8g2.drawCircle(10, 17, 1);
}

void ready() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenR10_te);
  u8g2.drawUTF8(38, 40, "BEREIT");
  u8g2.sendBuffer();
  delay(START_INTERVAL);
}

void lightActiv() {
  u8g2.clearBuffer();
  // Beispielhafte Zeichnung eines rechteckigen Fahrzeugs mit Licht
  u8g2.drawRFrame(54, 12, 20, 40, 7);
  u8g2.setFont(u8g2_font_t0_12_tr);
  u8g2.drawUTF8(61, 23, "V");
  u8g2.drawUTF8(61, 46, "H");
  u8g2.sendBuffer();
}

void temperature() {
  u8g2.setFont(u8g2_font_t0_12_tr);
  u8g2.setCursor(5, 40);
  // Hier rufen wir getTemp() aus sensors.cpp auf.
  float tempInside = getTemp(insideThermometer);  // insideThermometer ist in sensors.cpp definiert
  float tempOutside = getTemp(outsideThermometer);
  u8g2.print("I: ");
  u8g2.print(tempInside, 1);
  u8g2.print("   A: ");
  u8g2.print(tempOutside, 1);
}

void clockTime() {
  // Holt die aktuelle Zeit von der RTC
  extern RTC_DS3231 rtc;
  DateTime now = rtc.now();
  u8g2.setFont(u8g2_font_t0_12_tr);
  u8g2.setCursor(50, 20);
  u8g2.print(now.hour(), DEC);
  u8g2.print(":");
  if (now.minute() < 10) { u8g2.print("0"); }
  u8g2.print(now.minute(), DEC);
}
