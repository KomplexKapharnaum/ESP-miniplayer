#include <Arduino.h>
#include "debug.h"

#define VERSION_I 0.27

//
// HARD CONFIG (comment once it has been done !)
//
// #define NODE_ID 7
// #define NODE_CH 2
#define NODE_SPEAKER 2  // 1: small / 2: big

//
// SOFT CONFIG
//
#define ENABLE_IFACE
#define ENABLE_OTA

void setup() {
  LOGSETUP();

  // LED / BTN
  #ifdef ENABLE_IFACE
    iface_setup();  
  #endif

  settings_setup();

  // WIFI ENGINE
  //wifiman_auto();
  //wifiman_manual("kxkm-wifi", "KOMPLEXKAPHARNAUM");
  wifiman_manual("kxkm24nano", NULL);

  // OTA
  #ifdef ENABLE_OTA
    ota_setup();
  #endif

  // AUDIO ENGINE
  audio_setup();

  // INTERFACE ENGINE
  udp_setup();

  LOGF2("Device Ready, id:%i, channel:%i\n",settings_id(),settings_ch());

  #ifdef ENABLE_IFACE
    iface_led(false);
  #endif
}

void loop() {
  
  // AUDIO ENGINE
  audio_loop();
  ESP.wdtFeed();

  // INTERFACE ENGINE
  udp_loop();
  ESP.wdtFeed();

  // LED / BTN
  #ifdef ENABLE_IFACE
    iface_loop();
    if (iface_btn()) {
      if (!audio_running()) audio_play("/tone.mp3"); 
      iface_led(true);
    }
    else iface_led(false);
  #endif

  // OTA
  #ifdef ENABLE_OTA
    ota_loop();
  #endif
}
