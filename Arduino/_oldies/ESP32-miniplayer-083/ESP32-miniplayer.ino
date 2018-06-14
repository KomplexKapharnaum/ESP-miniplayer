/*
   SETTINGS
*/
#define MP_VERSION  0.72  // Sync on demand
#define MP_VERSION  0.73  // Sync unFreeze
#define MP_VERSION  0.75  // Volume fix
#define MP_VERSION  0.76  // stm32 support
#define MP_VERSION  0.77  // Sync error report
#define MP_VERSION  0.78  // Minor Fixes
#define MP_VERSION  0.79  // Gain based on Model
#define MP_VERSION  0.80  // Shutdown
#define MP_VERSION  0.81  // Gain levels
#define MP_VERSION  0.82  // Sync log
#define MP_VERSION  0.83  // Sync ignore empty

/*
   INCLUDES
*/
#include "debug.h"
#include <WiFi.h>
#include <SD.h>
#include "KXKM_STM32_energy_API.h"

/*
   SETUP
*/
void setup() {

  // Settings config
  String keys[16] = {"id", "channel", "model"};
  settings_load( keys );

  // Settings SET
  //settings_set("id", 27);
  //settings_set("channel", 1);
  //settings_set("model", 1);

  // STM32
  if ( settings_get("model") > 0 ) stm32_setup();
  else LOGSETUP();

  // Wifi
  // wifi_static("192.168.0.237");
  //wifi_connect("interweb", "superspeed37");
  //wifi_connect("kxkm-wifi", "KOMPLEXKAPHARNAUM");
  wifi_connect("kxkm24nano");
  wifi_ota( "esp-" + osc_id() + " " + osc_ch() + " v" + String(MP_VERSION, 2) );
  wifi_onConnect(doOnConnect);
  //wifi_wait(5000, true);

  // SD
  if (!sd_setup()) {
    //LOG("No SD detected.. restarting");
    //delay(500);
    //ESP.restart();
  }

  // SCAN FILES
  sd_scanNotes();

  /*for (byte i=0; i<16; i++)
    for (byte j=0; j<128; j++) {
      String file = sd_getPathNote(i, j);
      if (file != "") {
        LOGINL(file+" ");
        LOG(SD.open(file).size());
      }
    }*/

  // Audio
  audio_setup();
  audio_loop(true);
}

/*
   LOOP
*/
void loop() {

  wifi_loop();

  osc_loop();

  audio_run();

  stm32_loop();
}


/*
   on Connect
*/
void doOnConnect() {
  osc_setup();
}

