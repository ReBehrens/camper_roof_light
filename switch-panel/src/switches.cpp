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

    bool currentSw6 = digitalRead(SW6);
    Serial.println(digitalRead(SW6));
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

    // if (currentSw6 && !lastSw6)
    // {
    //     qs6 = !qs6;
    //     qs7 = !qs7;
    // }
    lastSw6 = currentSw6;

    // Toggle-Logik for SW5
    bool currentSw5 = digitalRead(SW5);
    if (currentSw5 && !lastSw5)
    {
        qs5 = !qs5;
    }
    lastSw5 = currentSw5;

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

    digitalWrite(LED1, qs1 ? HIGH : LOW);
    digitalWrite(LED2, qs2 ? HIGH : LOW);
    digitalWrite(LED3, qs3 ? HIGH : LOW);
    digitalWrite(LED4, qs4 ? HIGH : LOW);
    analogWrite(LED5, qs5 ? 10 : LOW);
    digitalWrite(LED7, qs6 ? HIGH : LOW);
    digitalWrite(LED8, qs7 ? HIGH : LOW);

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