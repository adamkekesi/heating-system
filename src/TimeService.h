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

    int fetchTime()
    {
        WiFiClient client;
        HTTPClient http;

        http.begin(client, "http://timeapi.io/api/Time/current/zone?timeZone=Europe/Budapest");

        int httpResponseCode = http.GET();

        String payload = http.getString();

        if (httpResponseCode == 200)
        {
            Serial.print("HTTP Response code: ");
            Serial.println(httpResponseCode);
        }
        else
        {
            Serial.print("Error code: ");
            Serial.println(httpResponseCode);
            Serial.println(payload);
            http.end();
             
            return -1;
        }
        http.end();

        DynamicJsonDocument doc(10024);
        DeserializationError err = deserializeJson(doc, payload);
        if (err)
        {
            Serial.println("Error parsing JSON");
             
            return -1;
        }
        hour = doc["hour"].as<int>();
        return 0;
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

        if (hour == -1 || lastFetchFailed || (currentMillis - lastFetch >= timeFetchInterval))
        {
            lastFetchFailed = false;
            try
            {
                int code = fetchTime();
                if (code < 0)
                {
                    lastFetchFailed = true;
                }
            }
            catch (...)
            {

                lastFetchFailed = true;
            }
            
            lastFetch = currentMillis;
        }
    }
};
