#if defined(ESP8266)
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#else
#include <WiFi.h>          //https://github.com/esp8266/Arduino
#endif

//needed for library
#if defined(ESP8266)
#include <ESP8266WebServer.h>
#else
#include <WebServer.h>
#endif
#include <DNSServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager

#include <SPI.h>
#include <WiFiClientSecure.h>
#include <stdlib.h>
#include <LoRa.h>
#include <string>
#include <iostream>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "images.h"
#include "certificate.h"

#define OLED_SDA 4
#define OLED_SCL 15
#define OLED_RST 16
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define XOFFSET 6

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

#define loraCS 18
#define loraRST 14
#define loraDIO0 26

#define spreadingFactor 12
#define signalBandwidth 125E3
#define frequency 915E6
#define codingRateDenominator 5
#define preambleLength 8
#define syncWord 0x12
#define txPower 20
#define CRC true

const long displayDimTime = 25000;

#define TRIGGER_PIN 0

const char*  server = "api.pushover.net";  // Server URL

WiFiClientSecure client;

int rssi;
int snr;
long freqErr;
String loraPacket;
String LoRaData;
bool sendFlag;
bool connection = false;
bool setupFlag = false;
bool disp = true;
volatile bool buttonChanged = false;
volatile bool buttonState = false;
String currentSSID;

const char* ssid[] = {"benisville", "Airwave", "benisville guest", "biden2020", "Netgear83", "Netgear93"};
const char* password[] = {"uwplatthub", "gentlebreeze253", ""};

unsigned long previousMillis = 0;
unsigned long previousDimMillis = 0;
unsigned long previousButtonMillis = 0;
const long interval = 1500;
void setup() {
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);
  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) {
    Serial.println("SSD1306 allocation failed");
  }

  Serial.begin(115200);
  Serial.println("\n Starting");
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Starting LoRa RX");
  display.display();

  pinMode(TRIGGER_PIN, INPUT);
  LoRa.setPins(loraCS, loraRST, loraDIO0);

  // Serial.println("LoRa Receiver");

  if (!LoRa.begin(frequency)) {
    Serial.println();
    Serial.println("LoRa init failed");
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("LoRa init failed");
    display.display();
    while (1);
  }
  else {
    Serial.println();
    Serial.println("LoRa init success");
    //    display.setTextColor(WHITE);
    //    display.clearDisplay();
    //    display.setTextSize(1);
    //    display.setCursor(0, 0);
    //    display.print("LoRa init success");
    //    display.display();
    //    delay(2000);
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  Serial.println();
  LoRa.setFrequency(frequency);
  Serial.print("Frequency:                 ");   Serial.println(frequency);                    Serial.println();
  display.setCursor(0, 0);                       display.print("Frequency:");
  display.setCursor(SCREEN_WIDTH - (String(frequency / 1000000.0).length() * XOFFSET), 0);     display.print(frequency / 1000000.0);

  LoRa.setSpreadingFactor(spreadingFactor);
  Serial.print("Spreading Factor:          ");   Serial.println(spreadingFactor);              Serial.println();

  display.setCursor(0, 8);                       display.print("Spread: ");
  display.setCursor(SCREEN_WIDTH - (String(spreadingFactor).length() * XOFFSET), 8);           display.print(spreadingFactor);

  LoRa.setSignalBandwidth(signalBandwidth);
  Serial.print("Signal Bandwidth:          ");   Serial.println(signalBandwidth);              Serial.println();

  display.setCursor(0, 16);                      display.print("Bandwidth: ");
  display.setCursor(SCREEN_WIDTH - (String(signalBandwidth / 1000.0).length() * XOFFSET), 16); display.print(signalBandwidth / 1000.0);

  LoRa.setCodingRate4(codingRateDenominator);
  Serial.print("Coding Rate (denominator): ");   Serial.println(codingRateDenominator);        Serial.println();

  display.setCursor(0, 24);                      display.print("Coding Rate: ");
  display.setCursor(SCREEN_WIDTH - (String(codingRateDenominator).length() * XOFFSET), 24);    display.print(codingRateDenominator);

  LoRa.setPreambleLength(preambleLength);
  Serial.print("Preamble Length:           ");   Serial.println(preambleLength);               Serial.println();

  display.setCursor(0, 32);                      display.print("Preamble len: ");
  display.setCursor(SCREEN_WIDTH - (String(preambleLength).length() * XOFFSET), 32);           display.print(preambleLength);

  LoRa.setSyncWord(syncWord);
  Serial.print("Sync Word:                 ");   Serial.println(syncWord);                     Serial.println();

  display.setCursor(0, 40);                      display.print("Sync Word: ");
  display.setCursor(SCREEN_WIDTH - (String(syncWord).length() * XOFFSET), 40);                 display.print(syncWord);

  LoRa.setTxPower(txPower);
  Serial.print("Tx Power                   ");   Serial.println(txPower);                      Serial.println();

  display.setCursor(0, 48);                      display.print("Tx Power: ");
  display.setCursor(SCREEN_WIDTH - (String(txPower).length() * XOFFSET), 48);                  display.print(txPower);

  if (CRC) {
    LoRa.enableCrc();
  }
  else {
    LoRa.disableCrc();
  }
  Serial.print("CRC                        "); if (CRC) {
    Serial.println("enabled");

    display.setCursor(0, 56); display.print("CRC: ");
    display.setCursor(SCREEN_WIDTH - (String("enabled").length() * XOFFSET), 56); display.print("enabled");
  } else {
    Serial.println("disabled");

    display.setCursor(0, 56); display.print("CRC: ");
    display.setCursor(SCREEN_WIDTH - (String("disabled").length() * XOFFSET), 56); display.print("disabled");
  }
  display.display();
  int cnt = 0;
  bool pass = false;
  while (!pass && cnt < 10000) {
    pass = !digitalRead(TRIGGER_PIN);
    delay(1);
    cnt++;
  }
  LoRa.receive();
  Serial.println("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
  Serial.println();
  Serial.println("Setup Complete");
  Serial.println();
  Serial.println("Trying preset WiFi credentials");
  //  display.clearDisplay();
  //  display.setTextColor(WHITE);
  //  display.setTextSize(1);
  //  display.setCursor(0, 0);  display.print("Setup Complete");
  //  delay(1000);
  //  display.setCursor(0, 16); display.print("Trying preset WiFi    credentials");
  //  display.display();
  //  display.setCursor(0, 32);


  int count = 0;
  int blink = 0;
  int ssidSize = *(&ssid + 1) - ssid;
  int passSize = *(&password + 1) - password;
  for (int i = 0; i < ssidSize; i++) {
    for (int j = 0; j < passSize; j++) {
      if (WiFi.status() != WL_CONNECTED) {
        WiFi.begin(ssid[i], password[j]);
        while (WiFi.status() != WL_CONNECTED && count < 30) {
          Serial.print(".");
          if (blink <= 4) {
            wifiBMP(blink);
            blink += 1;
          }
          else {
            blankBMP();
            blink = 0;
          }
          count++;
          delay(350);
        }
        count = 0;
      }
      else {
        connection = true;
        setupFlag = true;
      }
    }
  }
  if (WiFi.status() == WL_CONNECTED) {
    currentSSID = WiFi.SSID();
    Serial.print("Connected to ");
    Serial.println(currentSSID);
    display.clearDisplay();    display.setTextColor(WHITE);     display.setTextSize(1);
    display.setCursor((SCREEN_WIDTH + 5 - currentSSID.length() * XOFFSET) / 2, 28);  display.print(currentSSID);
    display.display();
    connection = true;
    sendPush("LoRa Receiver Setup Finished");
  }
  else {
    Serial.println("Preset Credentials Failed");
  }

  attachInterrupt(TRIGGER_PIN, buttonChangedISR, CHANGE);

  previousDimMillis = millis();
  previousButtonMillis = millis();
}

void loop() {
  // is configuration portal requested?
  while (!connection) {
    if ( digitalRead(TRIGGER_PIN) == LOW ) {
      //WiFiManager
      //Local intialization. Once its business is done, there is no need to keep it around
      WiFiManager wifiManager;

      if (!wifiManager.startConfigPortal("esp32 LoRa Rx")) {
        Serial.println("failed to connect and hit timeout");
        delay(3000);
        //reset and try again, or maybe put it to deep sleep
        ESP.restart();
        delay(5000);
      }
      else {
        connection = true;
        sendPush("LoRa Receiver Setup Finished");
      }

      //if you get here you have connected to the WiFi
      Serial.println("connected...:)");
    }
  }

  int packetSize = LoRa.parsePacket();
  if (packetSize)
  {
    // received a packet
    Serial.println("Received packet ");

    // read packet
    while (LoRa.available())
    {
      LoRaData = LoRa.readString();
      Serial.println(LoRaData);
      sendFlag = true;
    }
  }

  unsigned long currentMillis = millis();
  unsigned long currentDimMillis = millis();

  if (currentMillis - previousMillis >= interval && sendFlag == true) {
    previousMillis = currentMillis;

    rssi = LoRa.packetRssi();
    snr = LoRa.packetSnr();
    freqErr = LoRa.packetFrequencyError();

    String rssiString = String(rssi);
    String snrString = String(snr);
    String freqErrString = String(freqErrString);

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("connected to: ");
    display.setCursor(0, 8);
    display.print(currentSSID);
    display.setCursor(0, 16);
    display.print("rssi: ");
    display.println(rssiString);
    display.setCursor(0, 24);
    display.print("mes: ");
    display.println(LoRaData);
    display.display();
    disp = true;
    previousDimMillis = currentDimMillis;

    sendPush("RSSI: " + rssiString + " " + LoRaData);
    //sendPush("rssi: " + rssiString + " snr: " + snrString + " freqErr: " + freqErr + " " + LoRaData);
    sendFlag = false;
  }

  if (currentDimMillis - previousDimMillis >= displayDimTime && disp) {
    disp = false;
    Serial.println();
    Serial.println("Turning off display");
    Serial.println();
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(50, 28);
    display.print("Zzz...");
    display.display();
    delay(1000);
    display.clearDisplay();
    display.display();
  }

  if (buttonChanged) {
    dickBMP();
    previousDimMillis = currentDimMillis;
    disp = true;
    buttonChanged = false;
    Serial.println("button changed");
    Serial.print("button state: ");
    Serial.println(buttonState);
  }
}

void sendPush(String message) {
  client.setCACert(test_root_ca);
  Serial.println("\nStarting connection to server...");
  Serial.println();
  if (!client.connect(server, 443))
    Serial.println("Connection failed!");
  else {
    Serial.println("Connected to server!");
    Serial.println();
    //String content = "url=" + url + "&token=a57xzp7f57badgc3zyan6rvum6qzzb&user=utdnuh4tt1hd4hutfmk7q95uyoxpdm&device=iphone&title=PushNotfier&message=" + message;
    String content = "token=a57xzp7f57badgc3zyan6rvum6qzzb&user=utdnuh4tt1hd4hutfmk7q95uyoxpdm&device=iphone&title=LoRa&message=" + message;
    // Make a HTTP request:
    client.println("POST /1/messages.json HTTP/1.1");
    client.println("Host: api.pushover.net");
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.println("Connection: close");
    client.println("Content-Length: " + String(content.length()));
    client.println();
    client.println(content);
    client.println();

    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") {
        Serial.println("headers received");
        Serial.println();
        break;
      }
    }
    // if there are incoming bytes available
    // from the server, read them and print them:
    while (client.available()) {
      char c = client.read();
      Serial.write(c);
    }
    Serial.println();
    client.stop();
  }
}

void dickBMP(void) {
  display.clearDisplay();
  display.setRotation(1);
  display.drawBitmap(
    (display.width()  - DICKBUTT_WIDTH ) / 2,
    (display.height() - DICKBUTT_HEIGHT) / 2,
    dickbutt_bmp, DICKBUTT_WIDTH, DICKBUTT_HEIGHT, 1);
  display.display();
  display.setRotation(0);
}

void wifiBMP(int num) {
  display.clearDisplay();
  display.setRotation(1);
  if (num == 0) {
    display.clearDisplay();
    display.display();
  }
  else if (num == 1) {
    display.drawBitmap(
      (display.width()  - WIFI_WIDTH ) / 2,
      (display.height() - WIFI_HEIGHT) / 2,
      wifi_1_bmp, WIFI_WIDTH, WIFI_HEIGHT, 1);
  }

  else if (num == 2) {
    display.drawBitmap(
      (display.width()  - WIFI_WIDTH ) / 2,
      (display.height() - WIFI_HEIGHT) / 2,
      wifi_2_bmp, WIFI_WIDTH, WIFI_HEIGHT, 1);
  }

  else if (num == 3) {
    display.drawBitmap(
      (display.width()  - WIFI_WIDTH ) / 2,
      (display.height() - WIFI_HEIGHT) / 2,
      wifi_3_bmp, WIFI_WIDTH, WIFI_HEIGHT, 1);
  }

  else {
    display.drawBitmap(
      (display.width()  - WIFI_WIDTH ) / 2,
      (display.height() - WIFI_HEIGHT) / 2,
      wifi_4_bmp, WIFI_WIDTH, WIFI_HEIGHT, 1);
  }
  display.display();
  display.setRotation(0);
}

void blankBMP() {
  display.clearDisplay();
  display.setRotation(1);
  display.drawBitmap(
    (display.width()  - 128 ) / 2,
    (display.height() - 64) / 2,
    blank_bmp, 128, 64, 1);
  display.display();
  display.setRotation(0);
}

void buttonChangedISR() {
  buttonChanged = true;
  buttonState = !digitalRead(TRIGGER_PIN);
}
