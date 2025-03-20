//--- ESP32 DEV KIT Board
//-- MASTER----

#include <Arduino.h>
#include <WiFi.h>     //WiFi lib
#include <esp_now.h>  //ESP bus-system lib
#include <U8g2lib.h>  //display
#include <Wire.h>     //displ. + RTC bus
#include <OneWire.h>  //temp. bus
#include <DallasTemperature.h>  //tempsensors
#include <RTClib.h>   // RTC clock
#include <math.h>

//___ Display___
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* clock=*/22, /* data=*/21, /* reset=*/U8X8_PIN_NONE);
char screen_h = 0;
char screen_v = 10;
String screenOut;
//___

//___RTC-Clock___
#define DS3231_ADDRESS 0x68
RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
//__

//___ Tempsensors___
#define CHANNEL 1
#define ONE_WIRE_BUS 15
#define TEMPERATURE_PRECISION 12
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

DeviceAddress insideThermometer  = { 0x28, 0xFF, 0x64, 0x1F, 0x57, 0xAA, 0x16, 0xAC };
DeviceAddress outsideThermometer = { 0x28, 0xFF, 0x64, 0x1F, 0x57, 0xCF, 0x1C, 0x77 };
//__

//___ESPMOW___
String ssidName = "Slave";
String rssipass = "Slave_1_Password";
bool slaveFound = false;
esp_now_peer_info_t slave;
//___



// Switch-status Datablock for Sync with the Slaves
typedef struct SWITCH_POSITION {  //Switch pofsition
  bool S1 = 0;
  bool S2 = 0;
  bool S3 = 0;
  bool S4 = 0;
  bool S5 = 0;
};
SWITCH_POSITION SP;  //Short name

typedef struct RELAIS_STATS {  //Feedback from Slave (relais-status)
  bool R1 = 0;
  bool R2 = 0;
  bool R3 = 0;
  bool R4 = 0;
};
RELAIS_STATS RS;  //Short name


const char sw1 = 33;  // variable of inputs (Switche ect.)
const char sw2 = 25;
const char sw3 = 26;
const char sw4 = 27;
const char sw5 = 34;  //32
const char vcc = 36;  //34
const char light = 39;

const char led1 = 19;  // variable of outputs (led)
const char led2 = 18;
const char led3 = 5;
const char led4 = 17;
const char led5 = 16;
const char led6 = 23;

bool qs1 = 0;  // Temp Switch-status
bool qs2 = 0;
bool qs3 = 0;
bool qs4 = 0;
bool qs5 = 0;

bool lqs1 = 0;  // last switch positions
bool lqs2 = 0;
bool lqs3 = 0;
bool lqs4 = 0;


bool blogo = false;  // Logo condition

char startup = 0;
bool rWhileStop = false;
bool engineOn = false;

// Intervals
const int startInterval = 5000;
const int sendInterval = 500;
const int cooldown = 1000;           // 600000 / 10 min
//milli()
unsigned long timeStamp = 0;
unsigned long standyTimeStamp = 0;           // for display standby by Engine is off

//__________________________________________________________________________________________________
//===============================
// Debug mode for bugs and more =
bool debugMode = false;        //=
//===============================


//__________________________________________________________________________________________________
// Initialization of the ESP-NOW system incl. reset in case of errors
void InitESPNow() {
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESPNow Init Succsess");
  } else {
    Serial.println("ESPNow Init Failed");
    ESP.restart();
  }
}

//________________________________________________________________________________________________
// searching for Slaves
void SlaveScan() {
  int8_t scanResults = WiFi.scanNetworks();
  memset(&slave, 0, sizeof(slave));
  
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_t0_12_tf);
  u8g2.drawUTF8(25, 15, "Pairing Mode");
  if (debugMode) warning();
  u8g2.sendBuffer();

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

      // Check if the AP devices are Slave (SSIDNAME)
      if (SSID.indexOf(ssidName) == 0) {
        Serial.println("Slave  gefunden");

        
        // Get Mac-adrass from BSSID of slaves and stored it 
        int mac[6];
        if (6 == sscanf(BSSIDstr.c_str(), "%x:%x:%x:%x:%x:%x%c", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5])) {
          for (int ii = 0; ii < 6; ++ii) {
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

  //clear ram
  WiFi.scanDelete();
}

//_____________________________________________________________________________________________
// check if a slave connected, otherwise start connecting slaves
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
// sending Datablock to the Slaves
void SendStatus() {
  SWITCH_POSITION SP;
  SP.S1 = qs1;
  SP.S2 = qs2;
  SP.S3 = qs3;
  SP.S4 = qs4;
  SP.S5 = qs5;

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
// callback when the data has been sent to the slave
// here you can see if the slave was not reachable
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
// callback when recv data from Slave
void on_data_recv(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  char macStr[18];
  //MAC adress from Slave
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  //copy the data into the local array
  memcpy(&RS, data, sizeof(RS));
  //show it
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
void logo(bool engineOn) {
  // H = 1-127; V= 1-64;
  u8g2.clearBuffer();

  //clockTime();  // Clock

if (engineOn == true) {
  blogo = false;

} else {
  if (millis() > cooldown + standyTimeStamp) {
    blogo = true;
    clockTime();
    temperature();
  }
}

if (blogo == false) {

  char KV = 32;
  char KH = 64;
  char Kr = 31;
  char vLH1 = 56;
  char vLV1 = 8;
  char vLH2 = 64;
  char vLV2 = 27; 
  char wLH1 = 64;
  char wLV1 = 37; 
  char wLH2 = 56;
  char wLV2 = 56;
  char wLH3 = 41;
  char wLV3 = 20; 

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
}
  

  if (debugMode) {
    warning();
  }

  u8g2.sendBuffer();
}
//____________________________________________________________________________________________
void warning() {
  //if the debugmode is activ
  char debDeH1 = 2;
  char debDeV1 = 20;
  char debDeH2 = 10;
  char debDeV2 = 2;
  char debDeH3 = 18;
  char debDeV3 = 20;
  
  char debDAIH1 = 9;
  char debDAIV1 = 8;
  char debDAIV2 = 14;

  u8g2.drawLine(debDeH1, debDeV1, debDeH2, debDeV2);
  u8g2.drawLine(debDeH1, debDeV1, debDeH3, debDeV3);
  u8g2.drawLine(debDeH2, debDeV2, debDeH3, debDeV3);
  
  u8g2.drawLine(debDAIH1, debDAIV1, debDAIH1 + 2, debDAIV1);
  u8g2.drawLine(debDAIH1, debDAIV1, debDAIH1, debDAIV2);
  u8g2.drawLine(debDAIH1 + 2, debDAIV1, debDAIH1 + 2, debDAIV2);
  u8g2.drawLine(debDAIH1, debDAIV2, debDAIH1 + 2, debDAIV2);

  u8g2.drawCircle(10, 17, 1);
}
//____________________________________________________________________________________________
void ready() {
  if (rWhileStop == false) {
    //Serial.print("startup: " + startup);
      
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenR10_te);
    u8g2.drawUTF8(38, 40, "BEREIT");
    if (debugMode) warning();
    u8g2.sendBuffer();
      
    while (rWhileStop == false) {
      //Serial.print("startup: " + startup);
      if (millis() > startInterval + timeStamp) {
        rWhileStop = true;
      }
    }
    u8g2.clearBuffer();
    startup++;
  }
}
//____________________________________________________________________________________________
void lightActiv(){
  char LBoxB = 3;
  char LBoxL = 20;
  char LBox1PosH = 54;
  char LBox1PosV = 6;

  char LBox2PosH = 78;
  char LBox2PosV = 22;

  char LBox3PosH = 54;
  char LBox3PosV = 55;

  char LBox4PosH = 47;
  char LBox4PosV = 22;

  u8g2.clearBuffer();
  
  //---- car model 
  u8g2.drawRFrame(54,12,20,40,7);
  u8g2.setFont(u8g2_font_t0_12_tr );
  u8g2.drawUTF8(61, 23, "V");
  u8g2.drawUTF8(61, 46, "H");




  //-----light option
  if (qs1) {
    // front light
    u8g2.drawBox(LBox1PosH, LBox1PosV, LBoxL, LBoxB);
    u8g2.drawBox(LBox1PosH - 5, LBox1PosV - LBoxB, LBoxL + 10, LBoxB);
    u8g2.drawBox(LBox1PosH - 10, LBox1PosV - (2 * LBoxB), LBoxL + 20, LBoxB);
  } else {
    //..
  }

  if (qs2) {
    // right light
   u8g2.drawBox(LBox2PosH, LBox2PosV, LBoxB, LBoxL);
   u8g2.drawBox(LBox2PosH + LBoxB, LBox2PosV - 5, LBoxB, LBoxL + 10);
   u8g2.drawBox(LBox2PosH + (2 * LBoxB), LBox2PosV - 10, LBoxB, LBoxL + 20);
  } else {
    //..
  }

  if (qs3) {
    // rear light
    u8g2.drawBox(LBox3PosH, LBox3PosV, LBoxL, LBoxB);
    u8g2.drawBox(LBox3PosH - 5, LBox3PosV + 3, LBoxL + 10, LBoxB);
    u8g2.drawBox(LBox3PosH - 10, LBox3PosV + 6, LBoxL + 20, LBoxB);
  } else {
    //..
  }

  if (qs4) {
    // left light
    u8g2.drawBox(LBox4PosH, LBox4PosV, LBoxB, LBoxL);
    u8g2.drawBox(LBox4PosH - LBoxB, LBox4PosV - 5, LBoxB, LBoxL + 10);
    u8g2.drawBox(LBox4PosH - (2 * LBoxB), LBox4PosV - 10, LBoxB, LBoxL + 20);

  } else {
    //..
  }

  if (debugMode) warning();
  u8g2.sendBuffer();

}

//____________________________________________________________________________________________
void temperature(){
  float diff = 0.10;

  u8g2.setFont(u8g2_font_t0_12_tr);
  u8g2.setCursor(5, 40);
  u8g2.print("I: "); u8g2.print(roundf(getTemp(insideThermometer)), 1); u8g2.print("   A: "); u8g2.print(roundf(getTemp(outsideThermometer) + diff), 1);
  if (debugMode){
    Serial.print("I:"); Serial.println(getTemp(insideThermometer));
    Serial.print("A:"); Serial.println(getTemp(outsideThermometer));
  }
}
//===========================================================================================
//Get Temperature from sensors
float getTemp(DeviceAddress deviceAddress) {
  sensors.requestTemperatures();
  float tempC = sensors.getTempC(deviceAddress);
  
  if (tempC == DEVICE_DISCONNECTED_C) {
    if (debugMode) {
      Serial.println("Error: Could not read temp. data");

      for (uint8_t i = 0; i < 8; i++) {
      // zero pad the address if necessary
      if (deviceAddress[i] < 16) Serial.print("0");
      Serial.print(deviceAddress[i], HEX);
      }
      Serial.println("");
    }
    return 999.0;
  }
  return tempC;
}
//____________________________________________________________________________________________
//RTC Clock
void clockTime() {
  DateTime now = rtc.now();

  if (debugMode) {
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.println(now.minute(), DEC);
    
  }

    u8g2.setFont(u8g2_font_t0_12_tr);
    u8g2.setCursor(50, 20);
    u8g2.print(now.hour(), DEC); u8g2.print(":"); (now.minute() < 10) ? (u8g2.print("0")) : (0); u8g2.print(now.minute(), DEC);
    
}
//____________________________________________________________________________________________
//Switch status check
void switchCheck() {
  //Serial.print("startup: " + startup);

  if (digitalRead(vcc)) {
    Serial.println("vcc on");
    analogWrite(led6, 10);
    engineOn = true;
    standyTimeStamp = millis();
  } else {
    analogWrite(led6, LOW);
    engineOn = false;
  }
  
  // if (digitalRead(light)){
  //   Serial.println("Light on");
  //   analogWrite(led6, 100);
  //   engineOn = true;
  //   standyTimeStamp = millis(); 
  // } else {
  //   analogWrite(led6, LOW);
  //   engineOn = false;
  // }

  

  qs1 = digitalRead(sw1);
  qs2 = digitalRead(sw2);
  qs3 = digitalRead(sw3);
  qs4 = digitalRead(sw4);
  
  if (digitalRead(sw5) == 1) {
    qs5 = (qs5 == 0) ? (qs5 = 1) : (qs5 = 0);
  }
  
  if ((qs1 != lqs1) || (qs2 != lqs2) || (qs3 != lqs3) || (qs4 != lqs4)) {
    SendStatus();
    lqs1 = qs1;
    lqs2 = qs2;
    lqs3 = qs3;
    lqs4 = qs4;
  }

  if (qs1 == 1) {
    digitalWrite(led1, HIGH);
  } else {
    digitalWrite(led1, LOW);
  }

  if (qs2 == 1) {
    digitalWrite(led2, HIGH);
  } else {
    digitalWrite(led2, LOW);
  }

  if (qs3 == 1) {
    digitalWrite(led3, HIGH);
  } else {
    digitalWrite(led3, LOW);
  }

  if (qs4 == 1) {
    digitalWrite(led4, HIGH);
  } else {
    digitalWrite(led4, LOW);
  }

  if (qs1 || qs2 || qs3 || qs4 == 1) {
    lightActiv();
  } else {
    logo(engineOn);
  }

  
  if (qs5 == 1) {
    analogWrite(led5, 10);
  } else {
    analogWrite(led5, LOW);
  }

}

//===========================================================================================
void setup(void) {
  u8g2.begin();
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  sensors.begin();

  sensors.setResolution(insideThermometer, TEMPERATURE_PRECISION);
  sensors.setResolution(outsideThermometer, TEMPERATURE_PRECISION);
  if (debugMode) {
    Serial.print(sensors.getDeviceCount(), DEC); Serial.println(" /2 Temp. sensors found on the bus.");
    Serial.print("Parasite power is: ");
    if (sensors.isParasitePowerMode()) {
      Serial.println("ON");
    }
    else {
      Serial.println("OFF");
    }
  }
  
  // Must be called before search()
  oneWire.reset_search();
  // assigns the first address found to insideThermometer
  if (!oneWire.search(insideThermometer)) {
    Serial.println("Unable to find address for insideThermometer");
  }
  // assigns the seconds address found to outsideThermometer
  if (!oneWire.search(outsideThermometer)){
    Serial.println("Unable to find address for outsideThermometer");
  }

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power,set time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  rtc.disable32K();

  pinMode(sw1, INPUT_PULLDOWN);
  pinMode(sw2, INPUT_PULLDOWN);
  pinMode(sw3, INPUT_PULLDOWN);
  pinMode(sw4, INPUT_PULLDOWN);
  pinMode(sw5, INPUT_PULLDOWN);
  pinMode(vcc, INPUT);
  pinMode(light, INPUT);

  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);
  pinMode(led5, OUTPUT);
  pinMode(led6, OUTPUT);

  Serial.println("ESPNow ESP32 als Master");
  // Mac adress from master
  Serial.print("STA MAC: ");
  Serial.println(WiFi.macAddress());
  // Init ESPNow with a fallback logic
  InitESPNow();

  // reg. Callback function after sending
  esp_now_register_send_cb(on_data_sent);
  // reg. callback function after recv. data 
  esp_now_register_recv_cb(on_data_recv);
}

//---------------------
void loop(void) {
  // if not connected with slaves, continue pairing
  if (!slaveFound) SlaveScan();

  if (slaveFound) {
    // if is a Slave found and not connected, start connecting
    bool isPaired = manageSlave();
    if (isPaired) {
      if (startup == 0) {
        //Serial.print("startup: " + startup);
        timeStamp = millis();
        SendStatus();
        ready();
      }

      if (startup == 1) {
        //Serial.print("startup: " + startup);
        if (millis() > sendInterval + timeStamp) {
          switchCheck();
          timeStamp = millis();
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
