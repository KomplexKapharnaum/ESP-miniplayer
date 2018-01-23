#include <Arduino.h>
#include "debug.h"

#define VERSION_I 0.2

//
// REPLACE NODE ID (comment once it has been done !)
//
// #define NODE_ID 7
// #define NODE_CH 2


void setup() {
  LOGSETUP();

  settings_setup();

  // WIFI ENGINE
  //wifiman_auto();
  //wifiman_manual("kxkm-wifi", "KOMPLEXKAPHARNAUM");
  wifiman_manual("kxkm24nano", NULL);

  // OTA
  //ota_setup();

  // AUDIO ENGINE
  audio_setup();

  // INTERFACE ENGINE
  udp_setup();

  // LED / BTN
  iface_setup();

  LOGF2("Device Ready, id:%i, channel:%i\n",settings_id(),settings_ch());

  iface_led(true);
}

void loop() {
  
  // AUDIO ENGINE
  audio_loop();
  ESP.wdtFeed();

  // INTERFACE ENGINE
  udp_loop();
  ESP.wdtFeed();

  // LED / BTN
  iface_loop();
  if (iface_btn() && !audio_running()) audio_play("/tone.mp3"); 

  // OTA
  //ota_loop();

}
