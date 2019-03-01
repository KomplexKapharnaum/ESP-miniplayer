/*
   SETTINGS
*/
#define MP_VERSION  1.0   // Refactoring



/*
   INCLUDES
*/
#include <WiFi.h>
#include <SD.h>

#include "KESP_SETTINGS.h"
KESP_SETTINGS* settings;

#include "KESP_STM32.h"
KESP_STM32* stm32;

#include "KESP_LEDS.h"
KESP_LEDS* leds;

#include "PCM51xx.h"
#include "Wire.h"
PCM51xx pcm(Wire);

#include "KESP_AUDIO.h"
KESP_AUDIO* audio;


/*
   SETUP
*/
void setup() {

  // LOG system ( depends on #define DEBUG )
  LOGSETUP();

  // Settings config
  String keys[16] = {"id", "channel", "model"};
  settings = new KESP_SETTINGS(keys);

  // Settings SET
  //settings->set("id", 23);
  //settings->set("channel", 15);
  //settings->set("model", 1);   // 0: proto -- 1: big -- 2: small

  // STM32
  stm32 = new KESP_STM32();
  Serial.println("yo");

  // WIFI init
  String hostname = "esp-" + osc_id() + "-" + osc_ch() + "-v" + String(MP_VERSION, 2);
  wifi_set_hostname(hostname, true);
  wifi_onConnect(osc_start);
  //wifi_static("192.168.0.237");

  // WIFI go
  //wifi_connect("interweb", "superspeed37");
  wifi_connect("kxkm24");


  // AUDIO
  bool pcmOK = true;
  Wire.begin(2, 4);
  if (pcm.begin(PCM51xx::SAMPLE_RATE_44_1K, PCM51xx::BITS_PER_SAMPLE_16))
      LOG("PCM51xx initialized successfully.");
  else
  {
    LOG("Failed to initialize PCM51xx.");
    uint8_t powerState = pcm.getPowerState();
    if (powerState == PCM51xx::POWER_STATE_I2C_FAILURE)
    {
      LOGINL("No answer on I2C bus at address ");
      LOG(pcm.getI2CAddr());
    }
    else
    {
      LOGINL("Power state : ");
      LOG(pcm.getPowerState());
      LOG("Check that the sample rate / bit depth combination is supported.");
    }
    pcmOK = false;
  }
  audio = new KESP_AUDIO(pcmOK);
  // if(!audio->isEngineOK()) {
  //   LOG("Audio engine failed to start.. RESET !");
  //   stm32->reset();
  // }

  // MIDI
  audio->midiNoteScan();

  // AUDIO TEST
  /*String mediaPath = sd_getPathNote(0, 3);
  audio_volume(10);
  audio_play(mediaPath);
  */

  // LEDS
  leds = new KESP_LEDS();

}


/*
   LOOP
*/
void loop() {

  audio->run();

  // write in memory: can't be done in thread ?
  if (osc_newChan()) {
    settings->set("channel", osc_chan());
    osc_newChan(false);
  }

}
