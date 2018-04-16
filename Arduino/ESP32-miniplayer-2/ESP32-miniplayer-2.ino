/*
 * SETTINGS
 */
#define MP_VERSION  0.7

/*
 * INCLUDES
 */
#include "debug.h"
#include <WiFi.h>
#include <SD.h>

/*
 * SETUP
 */
void setup() {

  // Log
  LOGSETUP();  

  // Settings config
  String keys[16] = {"id", "channel","gain"};
  settings_load( keys );

  // Settings SET
  settings_set("id", 25);
  settings_set("channel", 11);
  settings_set("gain", 20);

  // Wifi
  // wifi_static("192.168.0.237");
  //wifi_connect("interweb", "superspeed37");
  wifi_connect("kxkm-wifi", "KOMPLEXKAPHARNAUM");
  wifi_ota( "esp-"+String(settings_get("id"))+" c"+String(settings_get("channel"))+" v"+String(MP_VERSION,2) );
  wifi_onConnect(doOnConnect);
  //wifi_wait(5000, true);

  // SD
  if (!sd_setup()) {
    LOG("No SD detected.. restarting");
    delay(500);
    ESP.restart();
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

  
  sd_syncRemote();
}

/*
 * LOOP
 */
void loop() {
  
  wifi_loop();
  
  osc_loop();
  
  audio_run();
}


/*
 * on Connect
 */
void doOnConnect() {
   osc_setup();
}

