#include <SPI.h>
#include <LoRa.h>
#include <ADXL345_WE.h>
#include <Adafruit_GPS.h>
#include <LowPower.h>
#define GPSSerial Serial1
Adafruit_GPS GPS(&GPSSerial);

#define GPSECHO false
#define statusInterval 2 //interval to send "no gps fix" measured in minutes

#define loraCS 4
#define accelCS 5
#define loraRST 8
#define loraDIO0 7

#define spreadingFactor 12
#define signalBandwidth 62.5E3
#define frequency 915E6
#define codingRateDenominator 5
#define preambleLength 8
#define syncWord 0x12
#define txPower 20

ADXL345_WE accel = ADXL345_WE(accelCS, true);
uint32_t timer = millis();
uint32_t activityTimer;


const int int2Pin = 2;
volatile bool activity = false;
#define activityThreshold 1.35
const float lowVoltage = 3.25;
bool startUp;

void setup() {
  Serial.begin(9600);
  //while (!Serial || millis() < 5000);
  delay(5000);
  analogReference(INTERNAL);
  pinMode(int2Pin, INPUT);
  Serial.println("gps tracker starting");
  Serial.println();
  //GPSSetup();
  //GPSSleep();
  if (!accel.init()) {
    Serial.println("accel init failed");
  }
  else {
    accel.init();
    Serial.println("accel init success");
    accel.setDataRate(ADXL345_DATA_RATE_100);
    accel.setRange(ADXL345_RANGE_4G);
    attachInterrupt(digitalPinToInterrupt(int2Pin), activityISR, RISING);
    accel.setActivityParameters(ADXL345_DC_MODE, ADXL345_XYZ, activityThreshold);
    accel.setInactivityParameters(ADXL345_DC_MODE, ADXL345_XYZ, 1.2, 5.0);
    accel.setInterrupt(ADXL345_ACTIVITY, INT_PIN_2);
  }
  activityTimer = millis();
}

void loop() {
  if (activity) {
    if (getBattVolt() <= lowVoltage) {
      loraSetup();
      if (LoRa.beginPacket()) {
        LoRa.print("Low Voltage, Shutting Down");
        LoRa.print(" Batt: ");
        LoRa.print(getBattVolt());
        LoRa.print("V");
        LoRa.endPacket();
      }
      detachInterrupt(digitalPinToInterrupt(int2Pin));
      GPSSleep();
      loraSleep();
      // Disable USB clock
      USBCON |= _BV(FRZCLK);
      // Disable USB PLL
      PLLCSR &= ~_BV(PLLE);
      // Disable USB
      USBCON &= ~_BV(USBE);
      // Add your wake up source here, example attachInterrupt()
      LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
    }
    Serial.println("activity");
    accel.readAndClearInterrupts();
    activityTimer = millis();
    detachInterrupt(digitalPinToInterrupt(int2Pin));
    loraSetup();
    sendGPS();
    accel.init();
    Serial.println("accel init success");
    accel.setDataRate(ADXL345_DATA_RATE_100);
    accel.setRange(ADXL345_RANGE_4G);
    attachInterrupt(digitalPinToInterrupt(int2Pin), activityISR, RISING);
    accel.setActivityParameters(ADXL345_DC_MODE, ADXL345_XYZ, activityThreshold);
    accel.setInactivityParameters(ADXL345_DC_MODE, ADXL345_XYZ, 1.2, 5.0);
    accel.setInterrupt(ADXL345_ACTIVITY, INT_PIN_2);
    activity = false;
  }

  if (!activity) {
    //if (millis() - activityTimer > 35000) {
    activityTimer = millis();
    Serial.println("going into sleep mode");
    detachInterrupt(digitalPinToInterrupt(int2Pin));
    loraSetup();
    if (LoRa.beginPacket()) {
      LoRa.print("Going To Sleep");
      LoRa.print(" Batt: ");
      LoRa.print(getBattVolt());
      LoRa.print("V");
      LoRa.endPacket();
    }
    GPSSetup();
    loraSleep();
    accel.init();
    accel.setDataRate(ADXL345_DATA_RATE_100);
    accel.setRange(ADXL345_RANGE_4G);
    attachInterrupt(digitalPinToInterrupt(int2Pin), activityISR, RISING);
    accel.setActivityParameters(ADXL345_DC_MODE, ADXL345_XYZ, activityThreshold);
    accel.setInactivityParameters(ADXL345_DC_MODE, ADXL345_XYZ, 1.2, 5.0);
    accel.setInterrupt(ADXL345_ACTIVITY, INT_PIN_2);
    // Disable USB clock
    USBCON |= _BV(FRZCLK);
    // Disable USB PLL
    PLLCSR &= ~_BV(PLLE);
    // Disable USB
    USBCON &= ~_BV(USBE);
    // Add your wake up source here, example attachInterrupt()
    delay(100);
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
    detachInterrupt(digitalPinToInterrupt(int2Pin));
    GPSSetup();
    loraSetup();
    accel.init();
    accel.setDataRate(ADXL345_DATA_RATE_100);
    accel.setRange(ADXL345_RANGE_4G);
    attachInterrupt(digitalPinToInterrupt(int2Pin), activityISR, RISING);
    accel.setActivityParameters(ADXL345_DC_MODE, ADXL345_XYZ, activityThreshold);
    accel.setInactivityParameters(ADXL345_DC_MODE, ADXL345_XYZ, 1.2, 5.0);
    accel.setInterrupt(ADXL345_ACTIVITY, INT_PIN_2);
    USBDevice.attach();
    //}
  }
}

void activityISR() {
  activity = true;
}

void sendGPS() {
  GPS.wakeup();
  char c = GPS.read();
  if (GPSECHO)
    if (c) //Serial.print(c);
      if (GPS.newNMEAreceived()) {
        Serial.print(GPS.lastNMEA());
        if (!GPS.parse(GPS.lastNMEA()))
          return;
      }
  // approximately every 2 seconds or so, print out the current stats
  if (millis() - timer > 2000 && GPS.fix) {
    timer = millis();
    if (LoRa.beginPacket()) {
      LoRa.print("Movement detected!  ");
      LoRa.print("Location: ");
      LoRa.print(GPS.latitude, 4);
      LoRa.print(GPS.lat);
      LoRa.print(", ");
      LoRa.print(GPS.longitude, 4);
      LoRa.print(GPS.lon);
      LoRa.print(" Speed (knots): ");
      LoRa.print(GPS.speed);
      LoRa.print(" Altitude: ");
      LoRa.print(GPS.altitude);
      LoRa.print(" Satellites: ");
      LoRa.print((int)GPS.satellites);
      LoRa.print(" Batt: ");
      LoRa.print(getBattVolt());
      LoRa.print("V");
      LoRa.endPacket();
      LoRa.sleep();
    }
  }
  if (!GPS.fix) {
    if (LoRa.beginPacket()) {
      LoRa.print("Movement detected!  ");
      LoRa.print("No GPS Fix");
      LoRa.print(" Batt: ");
      LoRa.print(getBattVolt());
      LoRa.print("V");
      LoRa.endPacket();
      LoRa.sleep();
    }
  }
}

void loraSetup() {
  LoRa.setPins(loraCS, loraRST, loraDIO0);
  if (!LoRa.begin(frequency)) {
    Serial.println("lora init failed");
  }
  else {
    Serial.println("lora init success");
  }
  LoRa.setFrequency(frequency);
  LoRa.setSpreadingFactor(spreadingFactor);
  LoRa.setSignalBandwidth(signalBandwidth);
  LoRa.setCodingRate4(codingRateDenominator);
  LoRa.setPreambleLength(preambleLength);
  LoRa.setSyncWord(syncWord);
  LoRa.setTxPower(txPower);
}

void loraSleep() {
  LoRa.sleep();
}

void GPSSetup() {
  //  if (!GPS.begin(9600)) {
  //    Serial.println("gps init failed");
  //  }
  //  else {
  //    Serial.println("gps init success");
  //  }
  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
}

void GPSSleep() {
  GPS.standby();
}

float getBattVolt() {
  int raw = analogRead(A0);
  float batt = (raw * 2.54) / 1024.0;
  batt = batt / 0.5;
  return batt;
}
