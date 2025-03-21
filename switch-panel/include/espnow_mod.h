#ifndef ESPNOW_MOD_H
#define ESPNOW_MOD_H

#include <esp_now.h>
#include <WiFi.h>
#include <Arduino.h>

// Structures for Switch and Relay Status
typedef struct
{
    bool S1 = 0;
    bool S2 = 0;
    bool S3 = 0;
    bool S4 = 0;
    bool S5 = 0;
} SWITCH_POSITION;

typedef struct
{
    bool R1 = 0;
    bool R2 = 0;
    bool R3 = 0;
    bool R4 = 0;
} RELAIS_STATS;

void espNowFirstSetup();
void espNowSecondSetup();
void InitESPNow();
void slaveScan();
bool manageSlave();
void SendStatus();
void on_data_sent(const uint8_t *mac_addr, esp_now_send_status_t status);
void on_data_recv(const uint8_t *mac_addr, const uint8_t *data, int data_len);

bool getSlaveFound();

#endif // ESPNOW_MOD_H
