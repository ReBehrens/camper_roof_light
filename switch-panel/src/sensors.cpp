#include "sensors.h"
#include "globals.h"
#include <Arduino.h>

// Globale Objekte für die Sensoren
RTC_DS3231 rtc;
OneWire oneWire(ONE_WIRE_PIN);
DallasTemperature sensors(&oneWire);

// Device-Adressen für die Temperatursensoren
DeviceAddress insideThermometer = {0x28, 0xFF, 0x64, 0x1F, 0x57, 0xAA, 0x16, 0xAC};
DeviceAddress outsideThermometer = {0x28, 0xFF, 0x64, 0x1F, 0x57, 0xCF, 0x1C, 0x77};

void sensorsSetup()
{
    sensors.begin();
    sensors.setResolution(insideThermometer, TEMPERATURE_PRECISION);
    sensors.setResolution(outsideThermometer, TEMPERATURE_PRECISION);

    if (debugMode)
    {
        Serial.print(sensors.getDeviceCount(), DEC);
        Serial.println(" /2 Temp. sensors found on the bus.");
        Serial.print("Parasite power is: ");
        if (sensors.isParasitePowerMode())
        {
            Serial.println("ON");
        }
        else
        {
            Serial.println("OFF");
        }
    }

    if (!rtc.begin())
    {
        Serial.println("RTC nicht gefunden");
        Serial.flush();
    }

    if (rtc.lostPower())
    {
        Serial.println("RTC hat Strom verloren, setze Zeit");
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
    rtc.disable32K();
}

void sensorsLoop()
{
    // Periodische Aufgaben, z. B. Sensorwerte aktualisieren.
}

float getTemp(DeviceAddress deviceAddress)
{
    sensors.requestTemperatures();
    float tempC = sensors.getTempC(deviceAddress);
    if (tempC == DEVICE_DISCONNECTED_C)
    {
        Serial.println("Fehler: Temperatur konnte nicht gelesen werden.");
        return 999.0;
    }
    return tempC;
}
