//--- ESP32 DEV KIT Board
//-- MASTER----

#include <Arduino.h>
#include <WiFi.h>     //WiFi lib
#include <esp_now.h>  //ESP Wifi Bussystem lib
#include <U8g2lib.h>
#include <Wire.h>

#define CHANNEL 1

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* clock=*/22, /* data=*/21, /* reset=*/U8X8_PIN_NONE);
esp_now_peer_info_t slave;

String ssidName = "Slave";
String rssipass = "Slave_1_Password";
bool slaveFound = false;

int screen_h = 0;
int screen_v = 10;
String screenOut;


// Datenblock zur übermittlung der Daten sowie die benennung der variablen
typedef struct SWITCH_POSITION {  //Schalter Position
  int S1 = 0;
  int S2 = 0;
  int S3 = 0;
  int S4 = 0;
};
SWITCH_POSITION SP;  //mit SP. können die variablen abgeruffen etc. werden

typedef struct RELAIS_STATS {  //Feedback vom Relais
  int R1 = 0;
  int R2 = 0;
  int R3 = 0;
  int R4 = 0;
};
RELAIS_STATS RS;  //mit RS. können die variablen abgeruffen etc. werden


const int sw1 = 33;  // definition der schalterpins
const int sw2 = 25;
const int sw3 = 26;
const int sw4 = 27;

const int led1 = 19;  // definition der LED pins
const int led2 = 18;
const int led3 = 5;
const int led4 = 17;

int qs1 = 0;  // abfrage Variablen der schalter
int qs2 = 0;
int qs3 = 0;
int qs4 = 0;

bool Blogo = true;  // Logo bedingungen
int startup = 0;
bool rWhileStop = false;

// intervallzeit bestimmen
const long startInterval = 5000;
const long sendInterval = 1000;
unsigned long timeStamp = 0;

//__________________________________________________________________________________________________
//===============================
// Debug mode for bugs and more =
bool debugMode = false;        //=
//===============================


//__________________________________________________________________________________________________
// Initialisierung des ESP-NOW systems inkl. reset bei Fehler
void InitESPNow() {
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESPNow Init Succsess");
  } else {
    Serial.println("ESPNow Init Failed");
    ESP.restart();
  }
}

//________________________________________________________________________________________________
// Suchen nach Slaves
void SlaveScan() {
  int8_t scanResults = WiFi.scanNetworks();
  memset(&slave, 0, sizeof(slave));

  Serial.println("");
  Serial.println("suche nach Slaves");
  if (scanResults == 0) {
    Serial.println("No AP found");
  } else {
    Serial.print(scanResults);
    Serial.println(" Geräte gefunden ");

    for (int i = 0; i < scanResults; ++i) {
      String SSID = WiFi.SSID(i);
      int32_t RSSI = WiFi.RSSI(i);
      String BSSIDstr = WiFi.BSSIDstr(i);

      // Prüfen ob AP Gerät ein Slave (ssidName) ist
      if (SSID.indexOf(ssidName) == 0) {
        Serial.println("Slave  gefunden");

        //MAC-Adresse aus der BSSID des Slaves ermitteln und in die Slave info Speichern
        int mac[6];
        if (6 == sscanf(BSSIDstr.c_str(), "%x:%x:%x:%x:%x:%x%c", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5])) {
          for (int ii = 0; ii < 6; ++ii) {
            slave.peer_addr[ii] = (uint8_t)mac[ii];
          }
        }

        slave.channel = CHANNEL;
        slave.encrypt = 0;
        slaveFound = true;
        //Ohne dieses break könnte man auch mit mehr als einem Slave verbinden
        //Wenn keine Verschlüsselung verwendet wird sind bis zu 20 Slaves möglich
        break;
      }
    }
  }

  //Ram aufreumen
  WiFi.scanDelete();
}

//_____________________________________________________________________________________________
// Prüfe ob ein Slave bereits gepaart is
// Sonst wird der Slave mit dem Master gepaart
bool manageSlave() {
  if (slave.channel == CHANNEL) {
    const esp_now_peer_info_t *peer = &slave;
    const uint8_t *peer_addr = slave.peer_addr;
    // check if the peer exists
    bool exists = esp_now_is_peer_exist(peer_addr);
    if (!exists) {
      // Slave not paired, attempt pair
      Serial.print("Slave Status: ");
      esp_err_t addStatus = esp_now_add_peer(peer);
      if (addStatus == ESP_OK) {
        // Pair success
        Serial.println("Pair success");
        return true;
      } else if (addStatus == ESP_ERR_ESPNOW_NOT_INIT) {
        // How did we get so far!!
        Serial.println("ESPNOW Not Init");
        return false;
      } else if (addStatus == ESP_ERR_ESPNOW_ARG) {
        Serial.println("Invalid Argument");
        return false;
      } else if (addStatus == ESP_ERR_ESPNOW_FULL) {
        Serial.println("Peer list full");
        return false;
      } else if (addStatus == ESP_ERR_ESPNOW_NO_MEM) {
        Serial.println("Out of memory");
        return false;
      } else if (addStatus == ESP_ERR_ESPNOW_EXIST) {
        Serial.println("Peer Exists");
        return true;
      } else {
        Serial.println("Not sure what happened");
        return false;
      }
    }
  } else {
    // No slave found to process
    Serial.println("No Slave found to process");
    return false;
  }
  return true;
}

//______________________________________________________________________________________________
void SendStatus() {
  SWITCH_POSITION SP;
  SP.S1 = qs1;
  SP.S2 = qs2;
  SP.S3 = qs3;
  SP.S4 = qs4;
  uint8_t data[sizeof(SP)];
  memcpy(data, &SP, sizeof(SP));
  const uint8_t *peer_addr = slave.peer_addr;
  (debugMode) ? ( Serial.println("Update sending....")) : (0);
  esp_err_t result = esp_now_send(peer_addr, data, sizeof(data));
  if (debugMode){
    Serial.print("Send Status: ");
    if (result == ESP_OK) {
      Serial.println("Success");
      Serial.print("Switch1 = ");
      Serial.println(SP.S1);
      Serial.print("Switch2 = ");
      Serial.println(SP.S2);
      Serial.print("Switch3 = ");
      Serial.println(SP.S3);
      Serial.print("Switch4 = ");
      Serial.println(SP.S4);

    } else if (result == ESP_ERR_ESPNOW_NOT_INIT) {
      // How did we get so far!!
      Serial.println("ESPNOW not Init.");
    } else if (result == ESP_ERR_ESPNOW_ARG) {
      Serial.println("Invalid Argument");
    } else if (result == ESP_ERR_ESPNOW_INTERNAL) {
      Serial.println("Internal Error");
    } else if (result == ESP_ERR_ESPNOW_NO_MEM) {
      Serial.println("ESP_ERR_ESPNOW_NO_MEM");
    } else if (result == ESP_ERR_ESPNOW_NOT_FOUND) {
      Serial.println("Peer not found.");
    } else {
      Serial.println("Not sure what happened");
    }
  }
}

//____________________________________________________________________________________________
// callback wenn die Daten zum Slave gesendet wurden
// hier sieht man wenn der Slave nicht erreichbar war
void on_data_sent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  if (debugMode) {
    Serial.print("Last Packet Sent to: ");
    Serial.println(macStr);
    Serial.print("Last Packet Send Status: ");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  }
}

//______________________________________________________________________________________________
// callback wenn wir Daten vom Slave bekommen
void on_data_recv(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  char macStr[18];
  //MAC Adresse des Slaves zur Info
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  //wir kopieren die Daten in die Datenstruktur
  memcpy(&RS, data, sizeof(RS));
  //und zeigen sie an
  if (debugMode) {
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
  }
}

//==========================================Display==========================================
//____________________________________________________________________________________________
void logo() {

  int KV = 32;
  int KH = 64;
  int Kr = 31;
  int vLH1 = 56;
  int vLV1 = 8;
  int vLH2 = 64;
  int vLV2 = 27;  //V30
  int wLH1 = 64;
  int wLV1 = 37;  //V34
  int wLH2 = 56;
  int wLV2 = 56;
  int wLH3 = 41;
  int wLV3 = 20;  // H 44 V15
  //____________________________________________
  u8g2.clearBuffer();
  u8g2.drawCircle(KH, KV, Kr);
  u8g2.drawCircle(KH, KV, Kr - 3);
  u8g2.drawCircle(KH, KV, Kr - 4);
  u8g2.drawCircle(KH, KV, Kr - 5);
  //----------
  u8g2.drawLine(vLH1, vLV1, vLH2, vLV2);
  u8g2.drawLine(vLH1 + 16, vLV1, vLH2, vLV2);
  u8g2.drawLine(vLH1 - 4, vLV1, vLH2 - 2, vLV2 + 3);
  u8g2.drawLine(vLH2 - 2, vLV2 + 3, vLH2 + 2, vLV2 + 3);
  u8g2.drawLine(vLH1 + 20, vLV1, vLH2 + 2, vLV2 + 3);
  //----------
  u8g2.drawLine(wLH1, wLV1, wLH2, wLV2);
  u8g2.drawLine(wLH1, wLV1, wLH2 + 16, wLV2);
  u8g2.drawLine(wLH3, wLV3, wLH2, wLV2);
  u8g2.drawLine(wLH3 + 46, wLV3, wLH2 + 16, wLV2);
  u8g2.drawLine(wLH1 - 2, wLV1 - 2, wLH2, wLV2 - 6);
  u8g2.drawLine(wLH1 - 2, wLV1 - 2, wLH1 + 2, wLV1 - 2);
  u8g2.drawLine(wLH1 + 2, wLV1 - 2, wLH2 + 16, wLV2 - 6);
  u8g2.drawLine(wLH3 + 1, wLV3 - 5, wLH2, wLV2 - 6);
  u8g2.drawLine(wLH3 + 45, wLV3 - 5, wLH2 + 16, wLV2 - 6);

  u8g2.sendBuffer();
}
//____________________________________________________________________________________________
void ready() {
  if (rWhileStop == false) {
   //Serial.print("startup: " + startup);
       
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_ncenR10_te);
        u8g2.drawUTF8(38, 40, "BEREIT");
        u8g2.sendBuffer();
      
      while (rWhileStop == false) {
        //Serial.print("startup: " + startup);
        if (millis() > startInterval + timeStamp) {
          rWhileStop = true;
        }
      }
      timeStamp = millis();
      u8g2.clearBuffer();
      startup++;
  }
}
//____________________________________________________________________________________________
//Switch status check
void switchCheck() {
  
}

//____________________________________________________________________________________________
void setup(void) {
  u8g2.begin();
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  pinMode(sw1, INPUT);
  pinMode(sw2, INPUT);
  pinMode(sw3, INPUT);
  pinMode(sw4, INPUT);

  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);

  Serial.println("ESPNow ESP32 als Master");
  // das ist die mac Adresse vom Master
  Serial.print("STA MAC: ");
  Serial.println(WiFi.macAddress());
  // Init ESPNow with a fallback logic
  InitESPNow();

  //Wir registrieren die Callback Funktion am Ende des sendevorgangs
  esp_now_register_send_cb(on_data_sent);
  //Wir registrieren die Callback Funktion für den Empfang
  esp_now_register_recv_cb(on_data_recv);
}

//---------------------
void loop(void) {
  // Wenn wir noch keinen Slave gefunden haben suchen wir weiter
  if (!slaveFound) SlaveScan();
  if (slaveFound) {
    //haben wir einen Slave muss er gepaart werden
    //falls das noch nicht geschehen ist
    bool isPaired = manageSlave();
    if (isPaired) {
      if (startup == 0) {
        //Serial.print("startup: " + startup);
        timeStamp = millis();
        ready();
      }

      if (startup == 1) {
        //Serial.print("startup: " + startup);
        if (millis() > sendInterval + timeStamp) {
          timeStamp = millis();
          // save the last time you updated the DHT values

          qs1 = digitalRead(sw1);
          qs2 = digitalRead(sw2);
          qs3 = digitalRead(sw3);
          qs4 = digitalRead(sw4);

          SendStatus();

          if (qs1 == 1) {
            digitalWrite(led1, HIGH);
            Blogo = false;
          } else {
            digitalWrite(led1, LOW);
          }

          if (qs2 == 1) {
            digitalWrite(led2, HIGH);
            Blogo = false;
          } else {
            digitalWrite(led2, LOW);
          }

          if (qs3 == 1) {
            digitalWrite(led3, HIGH);
            Blogo = false;
          } else {
            digitalWrite(led3, LOW);
          }

          if (qs4 == 1) {
            digitalWrite(led4, HIGH);
            Blogo = false;
          } else {
            digitalWrite(led4, LOW);
          }

          if (qs1 || qs2 || qs3 || qs4 == 1) {
            u8g2.clearBuffer();
            u8g2.setFont(u8g2_font_ncenR10_te);
            u8g2.drawUTF8(38, 30, "Lampe(n)");
            u8g2.drawUTF8(38, 50, "EIN!");
            u8g2.sendBuffer();
          } else {
            Blogo = true;
            logo();
          }
        }
      }
    }
  } else {
    // slave pair failed
    if (debugMode) {
      Serial.println("Slave pair failed!");
    }
  }
}