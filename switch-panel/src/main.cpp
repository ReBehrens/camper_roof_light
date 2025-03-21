#include <Arduino.h>
#include "globals.h"
#include "display.h"
#include "espnow_mod.h"
#include "sensors.h"
#include "switches.h"

unsigned long timeStamp = 0;
int warmup = 0;

void setup()
{
    Serial.println("Start Setup");
    displaySetup();
    startSystem();
    Serial.begin(115200);
    espNowFirstSetup();
    sensorsSetup();
    switchesSetup();
    espNowSecondSetup();
    Serial.println("Setup Done");
}

void loop()
{
    Serial.println("Start loop");
    Serial.println(getSlaveFound());
    if (!getSlaveFound())
    {
        Serial.println("Start search");
        slaveScan();
    }

    if (getSlaveFound())
    {
        Serial.println();
        Serial.println("connecting");
        bool isPaired = manageSlave();
        Serial.println(warmup);
        if (isPaired)
        {
            if (warmup == 0)
            {
                timeStamp = millis();
                setReadyTimeStamp(millis());
                SendStatus();
                ready();
                warmup++;
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
    delay(100); // kleiner Delay zur Schonung der CPU
}
