#include <Arduino.h>
#include <Adafruit_MAX31865.h>
#include <ESP8266WiFi.h>
#include "WiFiCredentials.h"
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <ESP8266WiFiMulti.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

Adafruit_MAX31865 thermo = Adafruit_MAX31865(2);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

#define RREF 430.0
#define RNOMINAL 100.0

int turnedOn4 = LOW;
int turnedOn5 = LOW;

float onTemp4 = 49.5;
float offTemp4 = 49;

int onTemp5 = 50;
float offTemp5 = 50.5;

void setup()
{
  Serial.begin(115200);

  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);

  WiFi.begin(SSID, PASSWORD);

  timeClient.begin();
  timeClient.setTimeOffset(3600);
  Serial.println("Adafruit MAX31865 PT100 Sensor Test!");
  thermo.begin(MAX31865_4WIRE);
}

void checkTempSensorFault()
{
  // Check and print any faults
  uint8_t fault = thermo.readFault();
  if (fault)
  {
    Serial.print("Fault 0x");
    Serial.println(fault, HEX);
    if (fault & MAX31865_FAULT_HIGHTHRESH)
    {
      Serial.println("RTD High Threshold");
    }
    if (fault & MAX31865_FAULT_LOWTHRESH)
    {
      Serial.println("RTD Low Threshold");
    }
    if (fault & MAX31865_FAULT_REFINLOW)
    {
      Serial.println("REFIN- > 0.85 x Bias");
    }
    if (fault & MAX31865_FAULT_REFINHIGH)
    {
      Serial.println("REFIN- < 0.85 x Bias - FORCE- open");
    }
    if (fault & MAX31865_FAULT_RTDINLOW)
    {
      Serial.println("RTDIN- < 0.85 x Bias - FORCE- open");
    }
    if (fault & MAX31865_FAULT_OVUV)
    {
      Serial.println("Under/Over voltage");
    }
    thermo.clearFault();
  }
}

void writeWifiStatus()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.print("Connected to ");
    Serial.println(SSID);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println("Not connected to WiFi");
  }
}

void checkTemp()
{
  float temp = thermo.temperature(RNOMINAL, RREF);

  if (temp > onTemp4)
  {
    turnedOn4 = HIGH;
  }

  if (temp < offTemp4)
  {
    turnedOn4 = LOW;
  }

  if (temp > onTemp5)
  {
    turnedOn5 = HIGH;
  }

  if (temp < offTemp5)
  {
    turnedOn5 = LOW;
  }

  digitalWrite(4, turnedOn4);
  digitalWrite(5, turnedOn5);

  Serial.println(temp);
  Serial.print("4-es: ");
  Serial.println(turnedOn4);
  Serial.print("5-ös: ");
  Serial.println(turnedOn5);
  Serial.println();

  checkTempSensorFault();
}

bool isBetweenHours(int h1, int h2, int hour, int minute)
{
  if (h1 <= h2)
  {
    if (hour >= h1 && hour < h2)
    {
      return true;
    }
    else if (hour == h2 && minute == 0)
    {
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    if (hour >= h1 || hour < h2)
    {
      return true;
    }
    else if (hour == h2 && minute == 0)
    {
      return true;
    }
    else
    {
      return false;
    }
  }
}

int getHour(struct tm *tm)
{
  int hour = tm->tm_hour;

  int month = tm->tm_mon;
  int day = tm->tm_mday;

  bool afterMarch26 = month > 2 || (month == 2 && day > 26);
  bool beforeOct29 = month < 9 || (month == 9 && day < 29);

  bool isSummerTime = afterMarch26 && beforeOct29;
  if (!isSummerTime)
  {
    return hour;
  }

  if (hour == 23)
  {
    return 0;
  }
  return hour + 1;
}

void loop()
{
  timeClient.update();
  writeWifiStatus();

  checkTemp();

  time_t epochTime = timeClient.getEpochTime();
  struct tm *ptm = gmtime((time_t *)&epochTime);

  if (timeClient.isTimeSet())
  {
    int hour = getHour(ptm);

    Serial.print("Hour: ");
    Serial.println(hour);

    int minute = ptm->tm_min;
    Serial.print("Minute: ");
    Serial.println(minute);

    if (isBetweenHours(18, 22, hour, minute))
    {
      onTemp5 = 78;
      offTemp5 = 80;
    }
    else
    {
      onTemp5 = 50;
      offTemp5 = 50.5;
    }
  }

  delay(1000);
}
