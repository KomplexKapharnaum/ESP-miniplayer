/*
 * SETTINGS
 */
#define MP_VERSION  0.72  // Sync on demand

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
  String keys[16] = {"id", "channel", "gain", "model"};
  settings_load( keys );

  // Settings SET
  //settings_set("id", 13);
  //settings_set("channel", 2);
  //settings_set("gain", 60);
  //settings_set("model", 0);

  // Wifi
  // wifi_static("192.168.0.237");
  //wifi_connect("interweb", "superspeed37");
  //wifi_connect("kxkm-wifi", "KOMPLEXKAPHARNAUM");
  wifi_connect("kxkm24nano");
  wifi_ota( "esp-"+osc_id()+" "+osc_ch()+" v"+String(MP_VERSION,2) );
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

