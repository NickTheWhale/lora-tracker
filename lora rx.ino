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

const long displayDimTime = 10000;

#define TRIGGER_PIN 0

#define LOGO_HEIGHT   114
#define LOGO_WIDTH    64
static const unsigned char PROGMEM logo_bmp[] =
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x26, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x01, 0x65, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x03, 0xe3, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x03, 0xe1, 0xf0, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x01, 0x27, 0x5e, 0x00, 0x00, 0x00,
  0x00, 0x03, 0xfd, 0xe6, 0x03, 0x80, 0x00, 0x00,
  0x00, 0x06, 0x0b, 0x88, 0x00, 0xc0, 0x00, 0x00,
  0x00, 0x18, 0x15, 0xd8, 0x00, 0x70, 0x00, 0x00,
  0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x00,
  0x00, 0x30, 0x00, 0x10, 0x00, 0x18, 0x00, 0x00,
  0x00, 0x10, 0x00, 0x30, 0x00, 0x0c, 0x00, 0x00,
  0x00, 0x1f, 0xe8, 0x10, 0x00, 0x4c, 0x05, 0x80,
  0x00, 0x02, 0x5f, 0x10, 0x00, 0x04, 0x5e, 0x80,
  0x00, 0x00, 0x01, 0xf0, 0x81, 0x07, 0xe1, 0x80,
  0x00, 0x00, 0x01, 0xb0, 0x62, 0x06, 0x00, 0x80,
  0x00, 0x00, 0x00, 0x88, 0x00, 0x00, 0x79, 0x80,
  0x00, 0x00, 0x01, 0x98, 0x00, 0x03, 0xf9, 0x00,
  0x00, 0x00, 0x00, 0x8c, 0x00, 0x01, 0x0b, 0x00,
  0x00, 0x00, 0x00, 0x8e, 0x00, 0x01, 0x92, 0x00,
  0x00, 0x00, 0x00, 0x59, 0x80, 0x00, 0x9e, 0x00,
  0x00, 0x00, 0x00, 0x62, 0x00, 0x00, 0xdc, 0x00,
  0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x50, 0x00,
  0x00, 0x00, 0x00, 0x19, 0x04, 0x00, 0x40, 0x00,
  0x00, 0x00, 0x00, 0x1e, 0x06, 0x00, 0x60, 0x00,
  0x00, 0x00, 0x00, 0x1c, 0x05, 0x00, 0x60, 0x00,
  0x00, 0x00, 0x00, 0xe8, 0x0b, 0x00, 0x20, 0x00,
  0x00, 0x00, 0x03, 0xc0, 0x0b, 0xc0, 0x60, 0x00,
  0x00, 0x00, 0x0f, 0x00, 0x1a, 0xc0, 0x3f, 0x90,
  0x00, 0x00, 0xf8, 0x01, 0xf1, 0xe0, 0x72, 0xfe,
  0x00, 0x17, 0xc0, 0x1f, 0x0e, 0x40, 0x28, 0x02,
  0x01, 0x7c, 0x00, 0x70, 0x3b, 0xe0, 0x77, 0xf6,
  0x03, 0x80, 0x00, 0xc3, 0xc0, 0x00, 0x20, 0x26,
  0x0c, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x60, 0x44,
  0x18, 0x3c, 0xc0, 0x60, 0x00, 0x00, 0x60, 0x48,
  0x20, 0x46, 0x40, 0x00, 0x00, 0x00, 0x40, 0xd8,
  0x60, 0xc3, 0x40, 0x00, 0x00, 0x00, 0xc0, 0x70,
  0xc0, 0x71, 0x60, 0x00, 0x00, 0x00, 0xc0, 0x40,
  0x40, 0xfd, 0xa0, 0x00, 0x00, 0x01, 0x80, 0x00,
  0x80, 0x7e, 0xb0, 0x00, 0x00, 0x01, 0x00, 0x00,
  0xc0, 0x7f, 0x18, 0x00, 0x00, 0x03, 0x00, 0x00,
  0x40, 0x3f, 0x08, 0x00, 0x00, 0x06, 0x00, 0x00,
  0xc0, 0x06, 0x1c, 0x00, 0x00, 0x0c, 0x00, 0x00,
  0x40, 0x00, 0x68, 0x00, 0x00, 0x38, 0x00, 0x00,
  0x40, 0x00, 0x7c, 0x00, 0x00, 0x20, 0x00, 0x00,
  0x60, 0x06, 0x54, 0x00, 0x00, 0xe0, 0x00, 0x00,
  0x30, 0x03, 0x1e, 0x00, 0x01, 0x80, 0x00, 0x00,
  0x18, 0x07, 0xf2, 0x00, 0x07, 0x00, 0x00, 0x00,
  0x0c, 0x0c, 0xe2, 0x00, 0x1c, 0x00, 0x00, 0x00,
  0x07, 0x08, 0x43, 0x00, 0x70, 0x00, 0x00, 0x00,
  0x01, 0xcf, 0xa1, 0x0b, 0xc0, 0x00, 0x00, 0x00,
  0x00, 0x7f, 0xe1, 0xbe, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x1f, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x07, 0xe8, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
const char*  server = "api.pushover.net";  // Server URL

// api.pushover.net root certificate authority
// SHA1 fingerprint is broken now!
const char* test_root_ca = \
                           "-----BEGIN CERTIFICATE-----\n" \
                           "MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh\n" \
                           "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
                           "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\n" \
                           "QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT\n" \
                           "MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n" \
                           "b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG\n" \
                           "9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB\n" \
                           "CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97\n" \
                           "nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt\n" \
                           "43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P\n" \
                           "T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4\n" \
                           "gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO\n" \
                           "BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR\n" \
                           "TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw\n" \
                           "DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr\n" \
                           "hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg\n" \
                           "06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF\n" \
                           "PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls\n" \
                           "YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk\n" \
                           "CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=\n" \
                           "-----END CERTIFICATE-----\n";

WiFiClientSecure client;

//SSD1306Wire display(0x3c, SDA_OLED, SCL_OLED, RST_OLED);

int rssi;
int snr;
long freqErr;
String loraPacket;
String LoRaData;
bool sendFlag;
bool connection = false;
bool setupFlag = false;
bool disp = true;
volatile bool button = false;
String currentSSID;

const char* ssid[] = {"benisville", "Airwave", "benisville guest", "biden2020", "Netgear83", "Netgear93"};
const char* password[] = {"uwplatthub", "gentlebreeze253", ""};

//char ssid[] = "benisville";
//char password[] = "uwplatthub";

unsigned long previousMillis = 0;
unsigned long previousDimMillis = 0;
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
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("LoRa init success");
    display.display();
    delay(2000);
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
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);  display.print("Setup Complete");
  delay(1000);
  display.setCursor(0, 16); display.print("Trying preset WiFi    credentials");
  display.display();
  display.setCursor(0, 32);

  int count = 0;
  int blink = 0;
  int ssidSize = *(&ssid + 1) - ssid;
  int passSize = *(&password + 1) - password;
  for (int i = 0; i < ssidSize; i++) {
    for (int j = 0; j < passSize; j++) {
      if (WiFi.status() != WL_CONNECTED) {
        WiFi.begin(ssid[i], password[j]);
        while (WiFi.status() != WL_CONNECTED && count < 20) {
          Serial.print(".");
          if (blink < 3) {
            blink += 1;
            display.print(".");
            display.display();
          }
          else {
            display.clearDisplay();
            display.setTextColor(WHITE);
            display.setTextSize(1);
            display.setCursor(0, 0);  display.print("Setup Complete");
            display.setCursor(0, 16); display.print("Trying preset WiFi    credentials");
            display.display();
            display.setCursor(0, 32);
            blink = 0;
          }
          count++;
          delay(500);
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
    display.setCursor(0, 0);   display.print("Setup Complete");
    display.setCursor(0, 16);  display.print("Trying preset WiFi    credentials");
    display.setCursor(0, 40);  display.print("Connected to: ");
    display.setCursor(0, 48);  display.print(currentSSID);
    display.display();
    connection = true;
    setupFlag = true;
  }
  else {
    Serial.println("Preset Credentials Failed");
  }
  attachInterrupt(TRIGGER_PIN, ISR, FALLING);
  previousDimMillis = millis();
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
        setupFlag = true;
      }

      //if you get here you have connected to the WiFi
      Serial.println("connected...:)");
    }
  }
  if (setupFlag) {
    setupFlag = false;
    sendPush("LoRa Receiver Setup Finished");
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

  if (button) {
    //    display.clearDisplay();
    //    display.setTextSize(1);
    //    display.setCursor(0, 0);
    //    display.print("we back");
    //    display.display();
    dickButt();
    previousDimMillis = currentDimMillis;
    disp = true;
    button = false;
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



void dickButt(void) {
  display.clearDisplay();
  display.setRotation(1);
  display.drawBitmap(
    (display.width()  - LOGO_WIDTH ) / 2,
    (display.height() - LOGO_HEIGHT) / 2,
    logo_bmp, LOGO_WIDTH, LOGO_HEIGHT, 1);
  display.display();
  display.setRotation(0);
}

void ISR() {
  button = true;
}
