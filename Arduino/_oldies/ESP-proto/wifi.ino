/*
 * WiFiManager (https://github.com/tzapu/WiFiManager)
 */
 
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         // https://github.com/tzapu/WiFiManager

void wifiman_auto(void)
{
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  
  //reset saved settings SSID and password
  wifiManager.resetSettings();

  //fetches ssid and pass from eeprom and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  wifiManager.autoConnect("ESP8266-kxkm");

  //if you get here you have connected to the WiFi
  LOG(F("connected"));
}

void wifiman_manual(const char *ssid, const char *password) 
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    LOGINLINE(".");
  }
  LOG();

  LOGINLINE("WiFi connected - ");  
  LOGINLINE("IP address: ");
  LOG(WiFi.localIP());
}


