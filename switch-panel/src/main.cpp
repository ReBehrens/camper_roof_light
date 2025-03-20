#include <Arduino.h>
#include "globals.h"
#include "display.h"
#include "espnow_mod.h"
#include "sensors.h"
#include "switches.h"

void setup()
{
    Serial.begin(115200);
    displaySetup();
    sensorsSetup();
    espNowSetup();
    switchesSetup();
}

void loop()
{
    switchesLoop();
    displayLoop();
    sensorsLoop();
    espNowLoop();
    delay(10); // kleiner Delay zur Schonung der CPU
}
