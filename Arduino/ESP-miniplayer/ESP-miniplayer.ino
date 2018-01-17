#include <Arduino.h>
#include <WebSocketsServer.h>   // https://github.com/Links2004/arduinoWebSockets
#include "debug.h"


//
// REPLACE NODE ID (comment once it has been done !)
//
#define NODE_ID 7
#define NODE_CH 2


void setup() {
  LOGSETUP();

  settings_setup();

  // WIFI ENGINE
  //wifiman_auto();
  wifiman_manual("kxkm-wifi", "KOMPLEXKAPHARNAUM");

  // MDNS
  //mdns_start(settings_name());
  //mdns_add("osc", "udp", 3737);

  // AUDIO ENGINE
  audio_setup();

  // INTERFACE ENGINE
  // webserver_setup();
  // coremidi_setup();
  udp_setup();

  LOG("Ready to go !");
}

void loop() {
  
  // AUDIO ENGINE
  audio_loop();
  ESP.wdtFeed();

  // INTERFACE ENGINE
  //webserver_loop();
  //coremidi_loop();
  udp_loop();
  ESP.wdtFeed();

}
