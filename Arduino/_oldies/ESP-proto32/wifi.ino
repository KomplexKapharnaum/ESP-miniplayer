#include <WiFi.h> 

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



