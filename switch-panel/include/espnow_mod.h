#ifndef ESPNOW_MOD_H
#define ESPNOW_MOD_H

#include <esp_now.h>
#include <WiFi.h>
#include <Arduino.h>

// Strukturen für Schalter- und Relais-Status
typedef struct
{
    bool S1;
    bool S2;
    bool S3;
    bool S4;
    bool S5;
} SWITCH_POSITION;

typedef struct
{
    bool R1;
    bool R2;
    bool R3;
    bool R4;
} RELAIS_STATS;

// Funktionsprototypen
void espNowSetup();
void espNowLoop();
void InitESPNow();
void SlaveScan();
bool manageSlave();
void SendStatus();
void on_data_sent(const uint8_t *mac_addr, esp_now_send_status_t status);
void on_data_recv(const uint8_t *mac_addr, const uint8_t *data, int data_len);

#endif // ESPNOW_MOD_H
