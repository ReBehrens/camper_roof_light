#include "switches.h"
#include "globals.h"
#include "espnow_mod.h"
#include "display.h"
#include <Arduino.h>

static bool qs1 = false, qs2 = false, qs3 = false, qs4 = false, qs5 = false;
static bool engineOn = false;
bool lqs1 = false, lqs2 = false, lqs3 = false, lqs4 = false;
static bool lastSw5 = false; // Toggle-Debounce

void switchesSetup()
{
    pinMode(SW1, INPUT_PULLDOWN);
    pinMode(SW2, INPUT_PULLDOWN);
    pinMode(SW3, INPUT_PULLDOWN);
    pinMode(SW4, INPUT_PULLDOWN);
    pinMode(SW5, INPUT);
    pinMode(VCC, INPUT);
    pinMode(LIGHT, INPUT);

    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);
    pinMode(LED3, OUTPUT);
    pinMode(LED4, OUTPUT);
    pinMode(LED5, OUTPUT);
    pinMode(LED6, OUTPUT);
}

void switchesLoop()
{

    engineOn = digitalRead(VCC);
    if (engineOn)
    {
        Serial.println("vcc on");
        analogWrite(LED6, 10);
    }
    else
    {
        Serial.println("vcc off");
        analogWrite(LED6, LOW);
    }

    qs1 = digitalRead(SW1);
    qs2 = digitalRead(SW2);
    qs3 = digitalRead(SW3);
    qs4 = digitalRead(SW4);

    // Toggle-Logik for SW5
    bool currentSw5 = digitalRead(SW5);
    if (currentSw5 && !lastSw5)
    {
        qs5 = !qs5;
    }
    lastSw5 = currentSw5;

    // send update with espNow
    if ((qs1 != lqs1) || (qs2 != lqs2) || (qs3 != lqs3) || (qs4 != lqs4))
    {
        lqs1 = qs1;
        lqs2 = qs2;
        lqs3 = qs3;
        lqs4 = qs4;
        SendStatus();
    }

    digitalWrite(LED1, qs1 ? HIGH : LOW);
    digitalWrite(LED2, qs2 ? HIGH : LOW);
    digitalWrite(LED3, qs3 ? HIGH : LOW);
    digitalWrite(LED4, qs4 ? HIGH : LOW);
    analogWrite(LED5, qs5 ? 10 : LOW);

    // logo or Light-information on Display
    if (qs1 || qs2 || qs3 || qs4)
    {
        lightActiv();
    }
    else
    {
        logo(engineOn);
    }
}

bool getQs1()
{
    return qs1;
}

bool getQs2()
{
    return qs2;
}

bool getQs3()
{
    return qs3;
}

bool getQs4()
{
    return qs4;
}

bool getQs5()
{
    return qs5;
}

bool isEngineOn()
{
    return engineOn;
}