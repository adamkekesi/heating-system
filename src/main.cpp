#include <Arduino.h>
#include <Adafruit_MAX31865.h>
#include <ESP8266WiFi.h>
#include "WiFiCredentials.h"
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <ESP8266WiFiMulti.h>
#include "TimeService.h"

ESP8266WiFiMulti WiFiMulti;

Adafruit_MAX31865 thermo = Adafruit_MAX31865(2);

#define RREF 430.0
#define RNOMINAL 100.0

int turnedOn = LOW;

int onTemp = 42;
int offTemp = 41;

TimeService timeService(30 * 60 * 1000);

void setup()
{
  Serial.begin(115200);
  Serial.println("Adafruit MAX31865 PT100 Sensor Test!");

  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);

  WiFi.begin(SSID, PASSWORD);

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

  if (temp > onTemp)
  {
    turnedOn = HIGH;
  }

  if (temp < offTemp)
  {
    turnedOn = LOW;
  }

  digitalWrite(4, turnedOn);
  digitalWrite(5, HIGH);

  Serial.println(temp);
  Serial.println(turnedOn);
  Serial.println();

  checkTempSensorFault();
}

void loop()
{
  writeWifiStatus();

  checkTemp();

  if (WiFiMulti.run() == WL_CONNECTED)
  {
    timeService.onLoop();
  }
  Serial.println(timeService.hour);

    delay(1000);
}
