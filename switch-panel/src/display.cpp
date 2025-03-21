#include "display.h"
#include "globals.h"
#include "switches.h"
#include "sensors.h" // Für getTemp() und RTC
#include <Arduino.h>

// Globales Display-Objekt – verwendet die in globals.h definierten Pins
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(
    U8G2_R0,
    DISPLAY_CLOCK_PIN,
    DISPLAY_DATA_PIN,
    DISPLAY_RESET_PIN);

// Dynamische Zustandsvariablen, die zur Displaylogik gehören
bool blogo = true;
bool rWhileStop = false;
unsigned long readyTimeStamp = 0;
unsigned long standyTimeStamp = 0;

void displaySetup()
{
  u8g2.begin();
}

void displayLoop()
{
  // Hier könntest du z. B. abhängig von Zuständen (aus switches.cpp) das Display aktualisieren.
  // Dieser Platzhalter lässt die Entscheidung extern (z. B. in switchesLoop) treffen.
}

void logo(bool engineOn)
{
  u8g2.clearBuffer();

  // Wenn der Motor aus ist und das Cooldown abgelaufen ist, soll das Logo angezeigt werden.
  if (!engineOn && (millis() > COOLDOWN + standyTimeStamp))
  {
    blogo = true;
    clockTime();
    temperature();
  }
  else
  {
    blogo = false;
  }

  if (!blogo)
  {
    char KV = 32;
    char KH = 64;
    char Kr = 31;
    char vLH1 = 56;
    char vLV1 = 8;
    char vLH2 = 64;
    char vLV2 = 27;
    char wLH1 = 64;
    char wLV1 = 37;
    char wLH2 = 56;
    char wLV2 = 56;
    char wLH3 = 41;
    char wLV3 = 20;

    //____________________________________________
    u8g2.clearBuffer();
    u8g2.drawCircle(KH, KV, Kr);
    u8g2.drawCircle(KH, KV, Kr - 3);
    u8g2.drawCircle(KH, KV, Kr - 4);
    u8g2.drawCircle(KH, KV, Kr - 5);
    //----------
    u8g2.drawLine(vLH1, vLV1, vLH2, vLV2);
    u8g2.drawLine(vLH1 + 16, vLV1, vLH2, vLV2);
    u8g2.drawLine(vLH1 - 4, vLV1, vLH2 - 2, vLV2 + 3);
    u8g2.drawLine(vLH2 - 2, vLV2 + 3, vLH2 + 2, vLV2 + 3);
    u8g2.drawLine(vLH1 + 20, vLV1, vLH2 + 2, vLV2 + 3);
    //----------
    u8g2.drawLine(wLH1, wLV1, wLH2, wLV2);
    u8g2.drawLine(wLH1, wLV1, wLH2 + 16, wLV2);
    u8g2.drawLine(wLH3, wLV3, wLH2, wLV2);
    u8g2.drawLine(wLH3 + 46, wLV3, wLH2 + 16, wLV2);
    u8g2.drawLine(wLH1 - 2, wLV1 - 2, wLH2, wLV2 - 6);
    u8g2.drawLine(wLH1 - 2, wLV1 - 2, wLH1 + 2, wLV1 - 2);
    u8g2.drawLine(wLH1 + 2, wLV1 - 2, wLH2 + 16, wLV2 - 6);
    u8g2.drawLine(wLH3 + 1, wLV3 - 5, wLH2, wLV2 - 6);
    u8g2.drawLine(wLH3 + 45, wLV3 - 5, wLH2 + 16, wLV2 - 6);
  }

  if (debugMode)
  {
    warning();
  }

  u8g2.sendBuffer();
}

void warning()
{
  // if the debugmode is activ
  char debDeH1 = 2;
  char debDeV1 = 20;
  char debDeH2 = 10;
  char debDeV2 = 2;
  char debDeH3 = 18;
  char debDeV3 = 20;

  char debDAIH1 = 9;
  char debDAIV1 = 8;
  char debDAIV2 = 14;

  u8g2.drawLine(debDeH1, debDeV1, debDeH2, debDeV2);
  u8g2.drawLine(debDeH1, debDeV1, debDeH3, debDeV3);
  u8g2.drawLine(debDeH2, debDeV2, debDeH3, debDeV3);

  u8g2.drawLine(debDAIH1, debDAIV1, debDAIH1 + 2, debDAIV1);
  u8g2.drawLine(debDAIH1, debDAIV1, debDAIH1, debDAIV2);
  u8g2.drawLine(debDAIH1 + 2, debDAIV1, debDAIH1 + 2, debDAIV2);
  u8g2.drawLine(debDAIH1, debDAIV2, debDAIH1 + 2, debDAIV2);

  u8g2.drawCircle(10, 17, 1);
}

void ready()
{
  if (rWhileStop == false)
  {
    // Serial.print("startup: " + startup);

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenR10_te);
    u8g2.drawUTF8(38, 40, "BEREIT");

    if (debugMode)
      warning();
    u8g2.sendBuffer();

    while (rWhileStop == false)
    {
      // Serial.print("startup: " + startup);
      if (millis() > START_INTERVAL + readyTimeStamp)
      {
        rWhileStop = true;
      }
    }
    u8g2.clearBuffer();
    warmup++;
  }
}

void lightActiv()
{
  char LBoxB = 3;
  char LBoxL = 20;
  char LBox1PosH = 54;
  char LBox1PosV = 6;

  char LBox2PosH = 78;
  char LBox2PosV = 22;

  char LBox3PosH = 54;
  char LBox3PosV = 55;

  char LBox4PosH = 47;
  char LBox4PosV = 22;

  u8g2.clearBuffer();

  //---- car model
  u8g2.drawRFrame(54, 12, 20, 40, 7);
  u8g2.setFont(u8g2_font_t0_12_tr);
  u8g2.drawUTF8(61, 23, "V");
  u8g2.drawUTF8(61, 46, "H");

  //-----light option
  if (getQs1)
  {
    // front light
    u8g2.drawBox(LBox1PosH, LBox1PosV, LBoxL, LBoxB);
    u8g2.drawBox(LBox1PosH - 5, LBox1PosV - LBoxB, LBoxL + 10, LBoxB);
    u8g2.drawBox(LBox1PosH - 10, LBox1PosV - (2 * LBoxB), LBoxL + 20, LBoxB);
  }
  else
  {
    //..
  }

  if (getQs2)
  {
    // right light
    u8g2.drawBox(LBox2PosH, LBox2PosV, LBoxB, LBoxL);
    u8g2.drawBox(LBox2PosH + LBoxB, LBox2PosV - 5, LBoxB, LBoxL + 10);
    u8g2.drawBox(LBox2PosH + (2 * LBoxB), LBox2PosV - 10, LBoxB, LBoxL + 20);
  }
  else
  {
    //..
  }

  if (getQs3)
  {
    // rear light
    u8g2.drawBox(LBox3PosH, LBox3PosV, LBoxL, LBoxB);
    u8g2.drawBox(LBox3PosH - 5, LBox3PosV + 3, LBoxL + 10, LBoxB);
    u8g2.drawBox(LBox3PosH - 10, LBox3PosV + 6, LBoxL + 20, LBoxB);
  }
  else
  {
    //..
  }

  if (getQs4)
  {
    // left light
    u8g2.drawBox(LBox4PosH, LBox4PosV, LBoxB, LBoxL);
    u8g2.drawBox(LBox4PosH - LBoxB, LBox4PosV - 5, LBoxB, LBoxL + 10);
    u8g2.drawBox(LBox4PosH - (2 * LBoxB), LBox4PosV - 10, LBoxB, LBoxL + 20);
  }
  else
  {
    //..
  }

  if (debugMode)
    warning();
  u8g2.sendBuffer();
}

void temperature()
{
  u8g2.setFont(u8g2_font_t0_12_tr);
  u8g2.setCursor(5, 40);
  // Hier rufen wir getTemp() aus sensors.cpp auf.
  float tempInside = getTemp(insideThermometer);
  float tempOutside = getTemp(outsideThermometer);
  u8g2.print("I: ");
  u8g2.print(tempInside, 1);
  u8g2.print("   A: ");
  u8g2.print(tempOutside, 1);
}

void clockTime()
{
  // Holt die aktuelle Zeit von der RTC
  extern RTC_DS3231 rtc;
  DateTime now = rtc.now();
  u8g2.setFont(u8g2_font_t0_12_tr);
  u8g2.setCursor(50, 20);
  u8g2.print(now.hour(), DEC);
  u8g2.print(":");
  (now.minute() < 10) ? (u8g2.print("0")) : (0);
  u8g2.print(now.minute(), DEC);
}

void searchingSlaves()
{
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_t0_12_tf);
  u8g2.drawUTF8(25, 15, "Pairing Mode");
  if (debugMode)
    warning();
  u8g2.sendBuffer();
}