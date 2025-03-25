#include "espnow_mod.h"
#include "globals.h"
#include "display.h"
#include "switches.h"
#include <Arduino.h>

// Global variable for ESP-NOW
SWITCH_POSITION SP;
RELAIS_STATS RS;
bool slaveFound = false;
esp_now_peer_info_t slave;

void espNowFirstSetup()
{
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
}

void espNowSecondSetup()
{
    InitESPNow();
    esp_now_register_send_cb(on_data_sent);
    esp_now_register_recv_cb(on_data_recv);
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

void slaveScan()
{
    searchingSlaves();

    int8_t scanResults = WiFi.scanNetworks();
    memset(&slave, 0, sizeof(slave));

    if (scanResults == 0)
    {
        Serial.println("No AP found");
    }
    else
    {
        Serial.print(scanResults);
        Serial.println(" Geräte gefunden ");

        for (int i = 0; i < scanResults; ++i)
        {
            String SSID = WiFi.SSID(i);
            int32_t RSSI = WiFi.RSSI(i);
            String BSSIDstr = WiFi.BSSIDstr(i);

            // Check if the AP devices are Slave (SSIDNAME)
            if (SSID.indexOf(WIFI_SSID) == 0)
            {
                Serial.println("Slave  gefunden");

                // Get Mac-adrass from BSSID of slaves and stored it
                int mac[6];
                if (6 == sscanf(BSSIDstr.c_str(), "%x:%x:%x:%x:%x:%x%c", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]))
                {
                    for (int ii = 0; ii < 6; ++ii)
                    {
                        slave.peer_addr[ii] = (uint8_t)mac[ii];
                    }
                }

                slave.channel = CHANNEL;
                slave.encrypt = 0;
                slaveFound = true;
                // for up to 20 slaves (without crypting) must delete the break;
                break;
            }
        }
    }

    WiFi.scanDelete();
}

// check if a slave connected, otherwise start connecting slaves
bool manageSlave()
{
    if (slave.channel == CHANNEL)
    {
        const esp_now_peer_info_t *peer = &slave;
        const uint8_t *peer_addr = slave.peer_addr;
        // check if the peer exists
        bool exists = esp_now_is_peer_exist(peer_addr);
        if (!exists)
        {
            // Slave not paired, attempt pair
            Serial.print("Slave Status: ");
            esp_err_t addStatus = esp_now_add_peer(peer);
            if (addStatus == ESP_OK)
            {
                Serial.println("Pair success");
                return true;
            }
            else if (addStatus == ESP_ERR_ESPNOW_NOT_INIT)
            {
                Serial.println("ESPNOW Not Init");
                return false;
            }
            else if (addStatus == ESP_ERR_ESPNOW_ARG)
            {
                Serial.println("Invalid Argument");
                return false;
            }
            else if (addStatus == ESP_ERR_ESPNOW_FULL)
            {
                Serial.println("Peer list full");
                return false;
            }
            else if (addStatus == ESP_ERR_ESPNOW_NO_MEM)
            {
                Serial.println("Out of memory");
                return false;
            }
            else if (addStatus == ESP_ERR_ESPNOW_EXIST)
            {
                Serial.println("Peer Exists");
                return true;
            }
            else
            {
                Serial.println("Not sure what happened");
                return false;
            }
        }
    }
    else
    {
        // No slave found to process
        Serial.println("No Slave found to process");
        return false;
    }
    return true;
}

void SendStatus()
{
    SWITCH_POSITION SP;
    SP.S1 = getQs1();
    SP.S2 = getQs2();
    SP.S3 = getQs3();
    SP.S4 = getQs4();
    SP.S5 = getQs5();
    SP.S6 = getQs6();
    SP.S7 = getQs7();

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
            Serial.print("Switch5 = ");
            Serial.println(SP.S5);
            Serial.print("Switch6 = ");
            Serial.println(SP.S6);
            Serial.print("Switch7 = ");
            Serial.println(SP.S7);
        }
        else if (result == ESP_ERR_ESPNOW_NOT_INIT)
        {
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
             mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    if (debugMode)
    {
        Serial.print("Last Packet Sent to: ");
        Serial.println(macStr);
        Serial.print("Last Packet Send Status: ");
        Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
    }
}

void on_data_recv(const uint8_t *mac_addr, const uint8_t *data, int data_len)
{
    char macStr[18];
    // MAC adress from Slave
    snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
             mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    // copy the data into the local array
    memcpy(&RS, data, sizeof(RS));
    if (debugMode)
    {
        Serial.print("Empfangen von ");
        Serial.println(macStr);
        Serial.print("Relais1 = ");
        Serial.print(RS.R1);
        Serial.print("Relais2 = ");
        Serial.print(RS.R2);
        Serial.print("Relais3 = ");
        Serial.print(RS.R3);
        Serial.print("Relais4 = ");
        Serial.print(RS.R4);
        Serial.print("Relais6 = ");
        Serial.print(RS.R6);
        Serial.print("Relais7 = ");
        Serial.print(RS.R7);
    }
}

bool getSlaveFound()
{
    return slaveFound;
}
