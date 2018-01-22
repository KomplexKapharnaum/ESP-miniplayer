#include <Arduino.h>
#include "debug.h"


//
// REPLACE NODE ID (comment once it has been done !)
//
//#define NODE_ID 7
#define NODE_CH 5


void setup() {
  LOGSETUP();

  settings_setup();

  // WIFI ENGINE
  //wifiman_auto();
  wifiman_manual("kxkm-wifi", "KOMPLEXKAPHARNAUM");

  // AUDIO ENGINE
  audio_setup();

  // INTERFACE ENGINE
  udp_setup();

  LOGF2("Device Ready, id:%i, channel:%i\n",settings_id(),settings_ch());
}

void loop() {
  
  // AUDIO ENGINE
  audio_loop();
  ESP.wdtFeed();

  // INTERFACE ENGINE
  udp_loop();
  ESP.wdtFeed();

}
