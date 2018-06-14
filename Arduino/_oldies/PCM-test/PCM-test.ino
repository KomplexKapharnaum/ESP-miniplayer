#include <Arduino.h>
#include <AudioOutputI2S.h>
#include <AudioFileSourcePROGMEM.h>
#include <AudioGeneratorFLAC.h>
#include "driver/i2s.h"

#include "sample.h"

AudioOutputI2S *out;
AudioFileSourcePROGMEM *file;
AudioGeneratorFLAC *flac;

unsigned long refresh = 0;


void setup()
{
  Serial.begin(115200);
  Serial.println("Starting up...\n");
  
  file = new AudioFileSourcePROGMEM( sample_flac, sizeof(sample_flac) );

  pcm_setup();
  out = new AudioOutputI2S();
  if (out->SetPinout(25, 27, 26)) Serial.println("Pinout changed");
  else Serial.println("Error while changing Pinout");

  out->SetRate(44100*4);
  out->SetBitsPerSample(16);
  out->SetChannels(2);
  
  flac = new AudioGeneratorFLAC();
  flac->begin(file, out);
}

void loop()
{
  //pcm_get(0x76);
  if ((millis()-refresh) > 500) {
    pcm_get(0x76);
    refresh = millis();
  }
  
  if (flac->isRunning()) {
    if (!flac->loop()) flac->stop();
  } else {
    Serial.printf("FLAC done\n");
    file = new AudioFileSourcePROGMEM( sample_flac, sizeof(sample_flac) );
    flac = new AudioGeneratorFLAC();
    flac->begin(file, out);
  }
}

