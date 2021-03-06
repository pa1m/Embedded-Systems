#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include "EEPROM.h"
#include "tactile.h"
#include <SoftwareSerial.h>
#include <Wire.h>
#include "SSD1306.h"
#include "oled_display.h"

#define PIN_BUTTON 0

Tactile button0(PIN_BUTTON);

#define EEPROM_SIZE 128

#define OLED_ADDR 0x3C
#define OLED_SDA 4
#define OLED_SCL 15

TinyGPSPlus gps;
HardwareSerial SerialGPS(1);
SSD1306 display(OLED_ADDR, OLED_SDA, OLED_SCL);
OledDisplay oledDisplay(&display);

double originLat = 0;
double originLon = 0; 
double originAlt = 0;
double distMax = 0;
double dist = 0;
double altMax = -999999;
double altMin = 999999;
double spdMax = 0;

double prevDist = 0;

#define TASK_OLED_RATE 200
#define TASK_SERIAL_RATE 500

uint32_t nextSerialTaskTs = 0;
uint32_t nextOledTaskTs = 0;

void setup() {

    Serial.begin(115200);
    SerialGPS.begin(19200, SERIAL_8N1, 16, 17);
    button0.start();

    while (!EEPROM.begin(EEPROM_SIZE)) {
        true;
    }

    long readValue; 
    EEPROM_readAnything(0, readValue);
    originLat = (double)readValue / 1000000;

    EEPROM_readAnything(4, readValue);
    originLon = (double)readValue / 1000000;

    EEPROM_readAnything(8, readValue);
    originAlt = (double)readValue / 1000000;

    oledDisplay.init();
    oledDisplay.page(OLED_PAGE_STATS);
}

template <class T> int EEPROM_writeAnything(int ee, const T& value)
{
   const byte* p = (const byte*)(const void*)&value;
   int i;
   for (i = 0; i < sizeof(value); i++)
       EEPROM.write(ee++, *p++);
   return i;
}

template <class T> int EEPROM_readAnything(int ee, T& value)
{
   byte* p = (byte*)(void*)&value;
   int i;
   for (i = 0; i < sizeof(value); i++)
       *p++ = EEPROM.read(ee++);
   return i;
}

void loop() {
    button0.loop();

    // Store new origin point
    if (button0.getState() == TACTILE_STATE_LONG_PRESS) {
        originLat = gps.location.lat();
        originLon = gps.location.lng();
        originAlt = gps.altitude.meters();

        long writeValue;
        writeValue = originLat * 1000000;
        EEPROM_writeAnything(0, writeValue);
        writeValue = originLon * 1000000;
        EEPROM_writeAnything(4, writeValue);
        writeValue = originAlt * 1000000;
        EEPROM_writeAnything(8, writeValue);
        EEPROM.commit();

        distMax = 0;
        altMax = -999999;
        spdMax = 0;
        altMin = 999999;
    } else if (button0.getState() == TACTILE_STATE_SHORT_PRESS) {
        oledDisplay.nextPage();
    }

    while (SerialGPS.available() > 0) {
        gps.encode(SerialGPS.read());
    }

    if (gps.satellites.value() > 4) {
        dist = TinyGPSPlus::distanceBetween(gps.location.lat(), gps.location.lng(), originLat, originLon);

        if (dist > distMax && abs(prevDist - dist) < 50) {
            distMax = dist;
        }

        prevDist = dist;

        if (gps.altitude.meters() > altMax) {
            altMax = gps.altitude.meters();
        }

        if (gps.speed.mps() > spdMax) {
            spdMax = gps.speed.mps();
        }

        if (gps.altitude.meters() < altMin) {
            altMin = gps.altitude.meters();
        }
    }

    if (nextSerialTaskTs < millis()) {

        Serial.print("LAT=");  Serial.println(gps.location.lat(), 6);
        Serial.print("LONG="); Serial.println(gps.location.lng(), 6);
        Serial.print("ALT=");  Serial.println(gps.altitude.meters());
        Serial.print("Sats=");  Serial.println(gps.satellites.value());
        Serial.print("Date=");
        Serial.print(gps.date.month());
        Serial.print(F("/"));
        Serial.print(gps.date.day());
        Serial.print(F("/"));
        Serial.print(gps.date.year());
        Serial.println();
        Serial.print("Time:");
        if (gps.time.isValid())
        {
          if (gps.time.hour() < 10) Serial.print(F("0"));
          Serial.print(gps.time.hour());
          Serial.print(F(":"));
          if (gps.time.minute() < 10) Serial.print(F("0"));
          Serial.print(gps.time.minute());
          Serial.print(F(":"));
          if (gps.time.second() < 10) Serial.print(F("0"));
          Serial.print(gps.time.second());
          Serial.print(F("."));
          if (gps.time.centisecond() < 10) Serial.print(F("0"));
          Serial.print(gps.time.centisecond());
        }
        Serial.println();
        Serial.print("DST: ");
        Serial.println(dist,1);

        nextSerialTaskTs = millis() + TASK_SERIAL_RATE;
    }

    oledDisplay.loop();

}
