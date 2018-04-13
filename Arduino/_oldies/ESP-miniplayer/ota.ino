#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h> // OTA
#include <WiFiUdp.h>
#include <ArduinoOTA.h> // OTA

void ota_setup() {
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  String nameDevice = "esp8266-";
  nameDevice += ESP.getChipId();
  nameDevice += " ( channel ";
  nameDevice += settings_ch();
  nameDevice += " )";
  ArduinoOTA.setHostname(nameDevice.c_str());

  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"123");

  ArduinoOTA.onStart([]() {
    LOG("Start");
  });
  ArduinoOTA.onEnd([]() {
    LOG("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    LOGF("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    LOGF("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) LOG("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) LOG("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) LOG("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) LOG("Receive Failed");
    else if (error == OTA_END_ERROR) LOG("End Failed");
  });
  ArduinoOTA.begin();
}

void ota_loop() {
  ArduinoOTA.handle();
}

