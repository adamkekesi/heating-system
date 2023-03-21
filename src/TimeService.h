#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "WiFiCredentials.h"
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <ESP8266WiFiMulti.h>

class TimeService
{
private:
    unsigned long lastFetch = 0;

    int timeFetchInterval;

    bool lastFetchFailed = false;

    void fetchTime()
    {
        WiFiClient client;
        HTTPClient http;

        http.begin(client, "https://timeapi.io/api/Time/current/zone?timeZone=Europe/Budapest");

        int httpResponseCode = http.GET();

        String payload = "";

        if (httpResponseCode > 0)
        {
            Serial.print("HTTP Response code: ");
            Serial.println(httpResponseCode);
            payload = http.getString();
        }
        else
        {
            Serial.print("Error code: ");
            Serial.println(httpResponseCode);
            http.end();
            throw new std::exception();
        }
        http.end();

        DynamicJsonDocument doc(1024);
        DeserializationError err = deserializeJson(doc, payload);
        if (err)
        {
            Serial.println("Error parsing JSON");
            throw new std::exception();
        }
        hour = doc["hour"].as<int>();
    }

public:
    int hour = -1;

    TimeService(int timeFetchInterval)
    {
        this->timeFetchInterval = timeFetchInterval;
    }

    ~TimeService() {}

    void onLoop()
    {
        unsigned long currentMillis = millis();
        if (lastFetchFailed || (currentMillis - lastFetch >= timeFetchInterval))
        {
            lastFetchFailed = false;
            try
            {
                fetchTime();
            }
            catch (...)
            {
                lastFetchFailed = true;
            }
            lastFetch = currentMillis;
        }
    }
};
