/*
   SETTINGS
*/
#define MP_VERSION  1.0   // Refactoring

/*
   INCLUDES
*/
#include "logs.h"
#include <WiFi.h>
#include <SD.h>

#include "KESP_STM32.h"
KESP_STM32* stm32;

#include "KESP_LEDS.h"
KESP_LEDS* leds;


/*
   SETUP
*/
void setup() {



  // Settings config
  String keys[16] = {"id", "channel", "model"};
  settings_load( keys );

  // Settings SET
  //settings_set("id", 23);
  //settings_set("channel", 15);
  //settings_set("model", 1);   // 0: proto -- 1: big -- 2: small

  // STM32
  stm32 = new KESP_STM32();

  // Wifi
      // wifi_static("192.168.0.237");
      //wifi_connect("interweb", "superspeed37");
  wifi_set_hostname("esp-" + oscC_id() + " " + oscC_ch() + " v" + String(MP_VERSION, 2) );
  wifi_connect("kxkm24");
  wifi_otaz();
  wifi_onConnect(doOnConnect);
      /*if (!wifi_wait(5000)) {
          stm32->reset();
      }*/

  // SD
  if (!sd_setup()) {
    LOG("No SD detected..");
    // stm32->reset();
  }

  // SCAN FILES
  sd_scanNotes();

  // Audio
  if (!audio_init()) {
    LOG("Audio engine failed to start..");
    stm32->reset();
  }
  audio_loop(false);

  // AUDIO TEST
  /*String mediaPath = sd_getPathNote(0, 3);
  audio_volume(10);
  audio_play(mediaPath);
  */

  // LEDS
  leds_start();
  leds = new KESP_LEDS();

}


/*
   LOOP
*/
void loop() {

  audio_run();

  // write in memory: can't be done in thread ?
  if (oscC_newChan()) {
    settings_set("channel", oscC_chan());
    oscC_newChan(false);
  }
}



/*
   on Connect
*/
void doOnConnect() {
  oscC_start();
}
