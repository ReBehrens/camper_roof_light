#include "espnow_mod.h"
#include "globals.h"
#include "switches.h"
#include <Arduino.h>

// Globale Variablen für ESP-NOW (nur hier definiert)
SWITCH_POSITION SP;
RELAIS_STATS RS;
bool slaveFound = false;
esp_now_peer_info_t slave;

void espNowSetup()
{
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    InitESPNow();
    esp_now_register_send_cb(on_data_sent);
    esp_now_register_recv_cb(on_data_recv);
}

void espNowLoop()
{
    // Beispiel: Falls noch kein Slave gefunden, starte die Suche
    if (!slaveFound)
    {
        SlaveScan();
    }
    else
    {
        manageSlave();
    }
}

void InitESPNow()
{
    if (esp_now_init() == ESP_OK)
    {
        Serial.println("ESPNow Init Success");
    }
    else
    {
        Serial.println("ESPNow Init Failed");
        ESP.restart();
    }
}

void SlaveScan()
{
    int8_t scanResults = WiFi.scanNetworks();
    memset(&slave, 0, sizeof(slave));
    Serial.println("Suche nach Slaves...");
    if (scanResults == 0)
    {
        Serial.println("Kein AP gefunden");
    }
    else
    {
        Serial.print(scanResults);
        Serial.println(" Geräte gefunden");
        for (int i = 0; i < scanResults; i++)
        {
            String SSID = WiFi.SSID(i);
            String BSSIDstr = WiFi.BSSIDstr(i);
            if (SSID.indexOf(WIFI_SSID) == 0)
            {
                Serial.println("Slave gefunden");
                int mac[6];
                if (6 == sscanf(BSSIDstr.c_str(), "%x:%x:%x:%x:%x:%x",
                                &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]))
                {
                    for (int ii = 0; ii < 6; ii++)
                    {
                        slave.peer_addr[ii] = (uint8_t)mac[ii];
                    }
                }
                slave.channel = ESPNOW_CHANNEL;
                slave.encrypt = 0;
                slaveFound = true;
                break;
            }
        }
    }
    WiFi.scanDelete();
}

bool manageSlave()
{
    if (slave.channel == ESPNOW_CHANNEL)
    {
        const uint8_t *peer_addr = slave.peer_addr;
        bool exists = esp_now_is_peer_exist(peer_addr);
        if (!exists)
        {
            Serial.print("Slave Status: ");
            esp_err_t addStatus = esp_now_add_peer(&slave);
            if (addStatus == ESP_OK)
            {
                Serial.println("Pairing erfolgreich");
                return true;
            }
            else
            {
                Serial.println("Pairing Fehler");
                return false;
            }
        }
    }
    else
    {
        Serial.println("Kein Slave zum Verarbeiten gefunden");
        return false;
    }
    return true;
}

void SendStatus()
{
    SWITCH_POSITION SP;
    SP.S1 = getQs1;
    SP.S2 = getQs2;
    SP.S3 = getQs3;
    SP.S4 = getQs4;
    SP.S5 = getQs5;

    uint8_t data[sizeof(SP)];
    memcpy(data, &SP, sizeof(SP));
    const uint8_t *peer_addr = slave.peer_addr;
    (debugMode) ? (Serial.println("Update sending....")) : (0);
    esp_err_t result = esp_now_send(peer_addr, data, sizeof(data));
    if (debugMode)
    {
        Serial.print("Send Status: ");
        if (result == ESP_OK)
        {
            Serial.println("Success");
            Serial.print("Switch1 = ");
            Serial.println(SP.S1);
            Serial.print("Switch2 = ");
            Serial.println(SP.S2);
            Serial.print("Switch3 = ");
            Serial.println(SP.S3);
            Serial.print("Switch4 = ");
            Serial.println(SP.S4);
        }
        else if (result == ESP_ERR_ESPNOW_NOT_INIT)
        {
            // How did we get so far!!
            Serial.println("ESPNOW not Init.");
        }
        else if (result == ESP_ERR_ESPNOW_ARG)
        {
            Serial.println("Invalid Argument");
        }
        else if (result == ESP_ERR_ESPNOW_INTERNAL)
        {
            Serial.println("Internal Error");
        }
        else if (result == ESP_ERR_ESPNOW_NO_MEM)
        {
            Serial.println("ESP_ERR_ESPNOW_NO_MEM");
        }
        else if (result == ESP_ERR_ESPNOW_NOT_FOUND)
        {
            Serial.println("Peer not found.");
        }
        else
        {
            Serial.println("Not sure what happened");
        }
    }
}

void on_data_sent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
             mac_addr[0], mac_addr[1], mac_addr[2],
             mac_addr[3], mac_addr[4], mac_addr[5]);
    Serial.print("Gesendet an: ");
    Serial.println(macStr);
    Serial.print("Status: ");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Erfolg" : "Fehler");
}

void on_data_recv(const uint8_t *mac_addr, const uint8_t *data, int data_len)
{
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
             mac_addr[0], mac_addr[1], mac_addr[2],
             mac_addr[3], mac_addr[4], mac_addr[5]);
    memcpy(&RS, data, sizeof(RS));
    Serial.print("Empfangen von: ");
    Serial.println(macStr);
    Serial.print("Relais-Status: R1=");
    Serial.print(RS.R1);
    Serial.print(" R2=");
    Serial.print(RS.R2);
    Serial.print(" R3=");
    Serial.print(RS.R3);
    Serial.print(" R4=");
    Serial.println(RS.R4);
}
