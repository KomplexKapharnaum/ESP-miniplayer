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
#define MP_VERSION  0.84  // Threaded audio + Mutexing everything
#define MP_VERSION  0.86  // Reset if PCM fails to init
#define MP_VERSION  0.87  // Channel change fix
#define MP_VERSION  0.88  // Switch ON power line on start / Reset//Stop do switch OFF
#define MP_VERSION  0.89  // Monitor AP mac + Wifi strength
#define MP_VERSION  0.90  // Icecast streaming

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
  //settings_set("id", 137);
  //settings_set("channel", 1);
  //settings_set("model", 0);   // 0: proto -- 1: big -- 2: small

  // STM32
  if ( settings_get("model") > 0 ) stm32_start();
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
    LOG("No SD detected..");
    //stm32_reset();
  }

  // SCAN FILES
  sd_scanNotes();

  // Audio
  if (!audio_setup()) {
    LOG("Audio engine failed to start..");
    stm32_reset();
  }
  audio_loop(false);
  
}

/*
   LOOP
*/
void loop() {
  audio_run();

  // write in memory: can't be done in thread ?
  if (osc_newChan()) {
    settings_set("channel", osc_chan());
    osc_newChan(false);
  }
}


/*
   on Connect
*/
void doOnConnect() {
  osc_start();
}

