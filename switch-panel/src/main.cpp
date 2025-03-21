#include <Arduino.h>
#include "globals.h"
#include "display.h"
#include "espnow_mod.h"
#include "sensors.h"
#include "switches.h"

unsigned long timeStamp = 0;

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
    if (!getSlaveFound)
        slaveScan();

    if (getSlaveFound)
    {
        bool isPaired = manageSlave();
        if (isPaired)
        {
            if (warmup == 0)
            {
                timeStamp = millis();
                SendStatus();
                ready();
            }

            if (warmup == 1)
            {
                if (millis() > SEND_INTERVAL + timeStamp)
                {
                    switchesLoop();
                    timeStamp = millis();
                }
            }
        }
    }
    else
    {
        if (debugMode)
        {
            Serial.println("Slave pair failed!");
        }
    }
    delay(15); // kleiner Delay zur Schonung der CPU
}
