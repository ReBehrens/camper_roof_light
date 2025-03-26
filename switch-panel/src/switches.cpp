#include "switches.h"
#include "globals.h"
#include "espnow_mod.h"
#include "display.h"
#include <Arduino.h>

static bool qs1 = false, qs2 = false, qs3 = false, qs4 = false, qs5 = false, qs6 = false, qs7 = false;
static bool engineOn = false;
bool lqs1 = false, lqs2 = false, lqs3 = false, lqs4 = false, lqs6 = false, lqs7 = false; // last qs status
static bool lastSw5 = false;                                                             // Toggle-Debounce for SW5
static bool lastSw6 = false;                                                             // Toggle-Debounce for SW6
int lastSWStatus = 0;                                                                    // 3-way Toggle (0 - 1 - 2)
bool currentSw5 = false;
bool currentSw6 = false;

void switchesSetup()
{
    pinMode(SW1, INPUT);
    pinMode(SW2, INPUT);
    pinMode(SW3, INPUT);
    pinMode(SW4, INPUT);
    pinMode(SW5, INPUT);
    pinMode(SW6, INPUT);
    pinMode(VCC, INPUT);
    pinMode(LIGHT, INPUT);

    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);
    pinMode(LED3, OUTPUT);
    pinMode(LED4, OUTPUT);
    pinMode(LED5, OUTPUT);
    pinMode(LED7, OUTPUT);
    pinMode(LED8, OUTPUT);
}

void engineIsOn()
{
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
}

void updateSensors()
{
    engineOn = digitalRead(VCC);
    qs1 = digitalRead(SW1);
    qs2 = digitalRead(SW2);
    qs3 = digitalRead(SW3);
    qs4 = digitalRead(SW4);
    currentSw6 = digitalRead(SW6);
    currentSw5 = digitalRead(SW5);
}

void checkFlasherSwitch()
{
    if (currentSw6 && !lastSw6)
    {
        if (lastSWStatus < 2)
            lastSWStatus++;
        else
            lastSWStatus = 0;
    }

    switch (lastSWStatus)
    {
    case 0:
        Serial.println("switch0");
        qs6 = false;
        qs7 = false;
        break;

    case 1:
        Serial.println("switch1");
        qs6 = true;
        break;

    case 2:
        qs7 = true;
        Serial.println("switch2");
        break;
    }

    lastSw6 = currentSw6;

    digitalWrite(LED7, qs6 ? HIGH : LOW);
    digitalWrite(LED8, qs7 ? HIGH : LOW);
}

void checkInteriorLight()
{
    if (currentSw5 && !lastSw5)
    {
        qs5 = !qs5;
    }
    lastSw5 = currentSw5;

    analogWrite(LED5, qs5 ? 10 : LOW);
}

void checkChangesToSend()
{
    // send update with espNow
    if ((qs1 != lqs1) || (qs2 != lqs2) || (qs3 != lqs3) || (qs4 != lqs4) || (qs6 != lqs6) || (qs7 != lqs7))
    {
        lqs1 = qs1;
        lqs2 = qs2;
        lqs3 = qs3;
        lqs4 = qs4;
        lqs6 = qs6;
        lqs7 = qs7;
        SendStatus();
    }
}

void checkChangesForDisplay()
{
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

bool getQs6()
{
    return qs6;
}

bool getQs7()
{
    return qs7;
}
bool isEngineOn()
{
    return engineOn;
}

void switchesLoop()
{
    updateSensors();
    engineIsOn();

    checkFlasherSwitch();
    checkInteriorLight();

    checkChangesToSend();
    checkChangesForDisplay();

    digitalWrite(LED1, qs1 ? HIGH : LOW);
    digitalWrite(LED2, qs2 ? HIGH : LOW);
    digitalWrite(LED3, qs3 ? HIGH : LOW);
    digitalWrite(LED4, qs4 ? HIGH : LOW);
}