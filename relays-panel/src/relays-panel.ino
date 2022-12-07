//--- ESP32 Dev kit1
//---SLAVE---

#include <Arduino.h>
#include <esp_now.h>              //ESP Wifi Bussystem lib
#include <WiFi.h>
//#include <Wire.h>


#define CHANNEL 1
const char *SSID = "Slave_1";


const int r1 = 15;         // definition der Relais-ausgänge
const int r2 = 2;
const int r3 = 0;
const int r4 = 4;

const int in1 = 34;       // definition der Stromkreisüberwachung
const int in2 = 35;
const int in3 = 32;
const int in4 = 33;


// Datenblock zur übermittlung der Daten sowie die benennung der variablen
typedef struct SWITCH_POSITION {    //Schalter Position
  int S1 = 0;
  int S2 = 0;
  int S3 = 0;
  int S4 = 0;
};
SWITCH_POSITION SP;       //mit SP. können die variablen abgeruffen etc. werden

typedef struct RELAIS_STATS {     //Feedback vom Relais
  int R1 = 0;
  int R2 = 0;
  int R3 = 0;
  int R4 = 0;
};
RELAIS_STATS RS;       //mit RS. können die variablen abgeruffen etc. werden

int qs1 = 0;         // abfrage Variablen der schalter
int qs2 = 0;
int qs3 = 0;
int qs4 = 0;

int qin1 = 0;         // abfrage Variablen der Relais
int qin2 = 0;
int qin3 = 0;
int qin4 = 0;

// intervallzeit bestimmen
const long interval = 10000; 
unsigned long previousMillis = 0;

//________________________________________________________________________________________________

// Init ESP Now with fallback

void InitESPNow() {
  WiFi.disconnect();
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESPNow Init Success");
  }
  else {
    Serial.println("ESPNow Init Failed");
    // Retry InitESPNow, add a counte and then restart?
    // InitESPNow();
    // or Simply Restart
    ESP.restart();
  }
}

//_________________________________________________________________________________________________

//WiFi Access Point
/*
void configDeviceAP() {
  bool result = WiFi.softAP(SSID, rssipass, CHANNEL, 0);
  if (!result) {
    Serial.println("AP Config failed.");
  } else {
    Serial.println("AP Config Success. Broadcasting with AP: " + String(SSID));
  }
}
*/

void configDeviceAP() {
  bool result = WiFi.softAP(SSID, "Slave_1_Password", CHANNEL, 0);
  if (!result) {
    Serial.println("AP Config failed.");
  } else {
    Serial.println("AP Config Success. Broadcasting with AP: " + String(SSID));
  }
}


//________________________________________________________________________________________________


/*
// info an den Master (was ist an?)
void SendStatus() {
  RELAIS_STATS RS;
    RS.R1 = digitalRead(in1);
    RS.R2 = digitalRead(in2);
    RS.R3 = digitalRead(in3);
    RS.R4 = digitalRead(in4);
    uint8_t data[sizeof(RS)]; memcpy(data, &RS, sizeof(RS));
    const uint8_t *peer_addr = slave.peer_addr;
  Serial.println("Update sending....");
  esp_err_t result = esp_now_send(peer_addr, data, sizeof(data));
  Serial.print("Send Status: ");
  if (result == ESP_OK) {
    Serial.println("Success");
    Serial.print("Stromkreis1 = "); Serial.println(RS.R1);
    Serial.print("Stromkreis2 = "); Serial.println(RS.R2);
    Serial.print("Stromkreis3 = "); Serial.println(RS.R3);
    Serial.print("Stromkreis4 = "); Serial.println(RS.R4);
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
*/
//____________________________________________________________________________________________

//Callback funktion wenn Daten empfangen wurden 
void OnDataRecv(const uint8_t *mac, const uint8_t *r_data, int len) {
    //zur Information wandeln wir die MAC Adresse des Absenders in einen String und geben sie aus
    char MACmaster[18];
    sprintf(MACmaster, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
  Serial.print("Last Packet Recv from: "); Serial.println(MACmaster);
  Serial.println("");
    
 
   
    //wir kopieren die empfangenen Daten auf die Datenstruktur um
    //über die Datenstruktur zugreifen zu können
    memcpy(&SP, r_data, sizeof(SP));
    Serial.println("Success");
    qs1 = SP.S1;
    qs2 = SP.S2;
    qs3 = SP.S3;
    qs4 = SP.S4;
    Serial.print("Switch1 = "); Serial.println(qs1);
    Serial.print("Switch2 = "); Serial.println(qs2);
    Serial.print("Switch3 = "); Serial.println(qs3);
    Serial.print("Switch4 = "); Serial.println(qs4);

    RS.R1 = qin1;
    RS.R2 = qin2;
    RS.R3 = qin3;
    RS.R4 = qin4;

    //kopieren der Daten der Relaisüberwachung in einen Speicherblock und zurücksenden an den Absender
    uint8_t s_data[sizeof(RS)]; memcpy(s_data, &RS, sizeof(RS));
    esp_now_send(mac, s_data, sizeof(s_data));

    Serial.println("Gesendet:");
    Serial.print("R1"); Serial.println(RS.R1);
    Serial.print("R2"); Serial.println(RS.R2);
    Serial.print("R3"); Serial.println(RS.R3);
    Serial.print("R4"); Serial.println(RS.R4);

    
  
 
}
//_____________________________________________________________________________________________



void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_AP);
  configDeviceAP();

  pinMode(r1, OUTPUT);
  pinMode(r2, OUTPUT);
  pinMode(r3, OUTPUT);
  pinMode(r4, OUTPUT);

  pinMode(in1, INPUT);
  pinMode(in2, INPUT);
  pinMode(in3, INPUT);
  pinMode(in4, INPUT);

  //ESOP-Now initialisieren  
  InitESPNow();
  
  //***MAC Adressenb zur Info ***//
  Serial.print("AP MAC: "); Serial.println(WiFi.softAPmacAddress());
  Serial.print("STA MAC: "); Serial.println(WiFi.macAddress());
 
  //und callback funktion registrieren
  esp_now_register_recv_cb(OnDataRecv); 
}

void loop() {
  // TEST CODE
  qin1 = digitalRead(in1);
  qin2 = digitalRead(in2);
  qin3 = digitalRead(in3);
  qin4 = digitalRead(in4);

  if (qs1 == 1){
    digitalWrite(r1, HIGH);
  } else {
    digitalWrite(r1, LOW);
  }

  if (qs2 == 1){
    digitalWrite(r2, HIGH);
  } else {
    digitalWrite(r2, LOW);
  }

  if (qs3 == 1){
    digitalWrite(r3, HIGH);
  } else {
    digitalWrite(r3, LOW);
  }

  if (qs4 == 1){
    digitalWrite(r4, HIGH);
  } else {
    digitalWrite(r4, LOW);
  }

  // Serial.print("R1 intern: "); Serial.println(qs1);
  // Serial.print("R2 intern: "); Serial.println(qs2);
  // Serial.print("R3 intern: "); Serial.println(qs3);
  // Serial.print("R4 intern: "); Serial.println(qs4);
  
  // Serial.print("RT1: "); Serial.println(qin1);
  // Serial.print("RT2: "); Serial.println(qin2);
  // Serial.print("RT3: "); Serial.println(qin3);
  // Serial.print("RT4: "); Serial.println(qin4);


//   digitalWrite(in1, HIGH);
//   delay(500);
//   digitalWrite(in1, LOW);
//   delay(500);
 
 }
