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
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_SDA 4
#define OLED_SCL 15
#define OLED_RST 16
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

#define loraCS 18
#define loraRST 14
#define loraDIO0 26

#define spreadingFactor 12
#define signalBandwidth 62.5E3
#define frequency 915E6
#define codingRateDenominator 5
#define preambleLength 8
#define syncWord 0x12
#define txPower 20

#define TRIGGER_PIN 0

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
String currentSSID;

const char* ssid[] = {"benisville", "Airwave", "benisville guest", "biden2020", "Netgear83", "Netgear93"};
const char* password[] = {"uwplatthub", "gentlebreeze253", ""};

//char ssid[] = "benisville";
//char password[] = "uwplatthub";

unsigned long previousMillis = 0;
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
  }
  Serial.println();
  LoRa.setFrequency(frequency);
  Serial.print("Frequency:                 "); Serial.println(frequency); Serial.println();
  LoRa.setSpreadingFactor(spreadingFactor);
  Serial.print("Spreading Factor:          "); Serial.println(spreadingFactor); Serial.println();
  LoRa.setSignalBandwidth(signalBandwidth);
  Serial.print("Signal Bandwidth:          "); Serial.println(signalBandwidth); Serial.println();
  LoRa.setCodingRate4(codingRateDenominator);
  Serial.print("Coding Rate (denominator): "); Serial.println(codingRateDenominator); Serial.println();
  LoRa.setPreambleLength(preambleLength);
  Serial.print("Preamble Length:           "); Serial.println(preambleLength); Serial.println();
  LoRa.setSyncWord(syncWord);
  Serial.print("Sync word:                 "); Serial.println(syncWord); Serial.println();
  LoRa.setTxPower(txPower);
  Serial.print("Tx Power                   "); Serial.println(txPower); Serial.println();
  LoRa.receive();
  Serial.println("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
  Serial.println();
  Serial.println("Setup Finished");
  Serial.println();
  Serial.println("Trying preset WiFi credentials");

  int count = 0;
  int ssidSize = *(&ssid + 1) - ssid;
  int passSize = *(&password + 1) - password;
  for (int i = 0; i < ssidSize; i++) {
    for (int j = 0; j < passSize; j++) {
      if (WiFi.status() != WL_CONNECTED) {
        WiFi.begin(ssid[i], password[j]);
        while (WiFi.status() != WL_CONNECTED && count < 20) {
          Serial.print(".");
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
    Serial.print("connected to ");
    Serial.println(currentSSID);
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("connected to: ");
    display.setCursor(0, 8);
    display.print(currentSSID);
    display.display();
    connection = true;
    setupFlag = true;
  }
  else {
    Serial.println("preset credentials failed");
  }
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
  unsigned long currentMillis = millis();

  int packetSize = LoRa.parsePacket();
  if (packetSize)
  {
    // received a packet
    Serial.println("Received packet '");

    // read packet
    while (LoRa.available())
    {
      LoRaData = LoRa.readString();
      Serial.println(LoRaData);
      sendFlag = true;
    }
  }

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

    sendPush("RSSI: " + rssiString + " " + LoRaData);
    //sendPush("rssi: " + rssiString + " snr: " + snrString + " freqErr: " + freqErr + " " + LoRaData);
    sendFlag = false;
  }
}

void sendPush(String message) {
  client.setCACert(test_root_ca);
  //client.setCertificate(test_client_key); // for client verification
  //client.setPrivateKey(test_client_cert);  // for client verification

  Serial.println("\nStarting connection to server...");
  if (!client.connect(server, 443))
    Serial.println("Connection failed!");
  else {
    Serial.println("Connected to server!");

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
        break;
      }
    }
    // if there are incoming bytes available
    // from the server, read them and print them:
    while (client.available()) {
      char c = client.read();
      Serial.write(c);
    }
    client.stop();
  }
}
