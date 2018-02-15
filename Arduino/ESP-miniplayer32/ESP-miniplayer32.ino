#include <Arduino.h>
#include "debug.h"

#define VERSION_I 0.30

//
// HARD CONFIG (comment once it has been done !)
//
#define NODE_ID 7
#define NODE_CH 7
#define GAIN_HARD 20  // volume FULL = 20%

//
// SOFT CONFIG
//
//#define ENABLE_BTNLED
#define ENABLE_OTA

void setup() {
  LOGSETUP();

  // LED / BTN
  #ifdef ENABLE_IFACE
    iface_setup();  
  #endif

  settings_setup();

  // WIFI ENGINE
  wifiman_manual("interweb", "superspeed37");

  // OTA
  #ifdef ENABLE_OTA
    ota_setup();
  #endif

  // AUDIO ENGINE
  audio_setup();

  // INTERFACE ENGINE
  udp_setup();

  LOGF2("Device Ready, id:%i, channel:%i\n",settings_id(),settings_ch());

  #ifdef ENABLE_BTNLED
    iface_led(false);
  #endif
}

void loop() {
  
  // AUDIO ENGINE
  audio_loop();
  yield();

  // INTERFACE ENGINE
  udp_loop();
  yield();

  // LED / BTN
  #ifdef ENABLE_BTNLED
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
