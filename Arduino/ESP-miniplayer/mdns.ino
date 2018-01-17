#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

void mdns_start(const char* hostname) 
{
  if(MDNS.begin(hostname))  LOG("MDNS responder started.");
  else LOG("MDNS responder failed");
  
}

void mdns_add(const char* name, const char* proto, int port) 
{
  // Add service to MDNS
  // MDNS.addService(name, proto, port);
  MDNS.addService("http", "tcp", 80);
}
