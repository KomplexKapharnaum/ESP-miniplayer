// https://github.com/earlephilhower/ESP8266Audio

#include "AudioFileSourceSD.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2SDAC.h"

AudioGeneratorMP3 *mp3;
AudioFileSourceSD *file;
AudioOutputI2SDAC *out;

void audio_setup()
{
  SD.begin(0);
  
  out = new AudioOutputI2SDAC();
  //out->SetBitsPerSample(16);
  //out->SetRate(44100);
  //out->SetGain(1.0);  

  mp3 = new AudioGeneratorMP3();
}

void audio_play(const char *filename) 
{
  if (mp3->isRunning()) audio_stop();
  file = new AudioFileSourceSD(filename);
  mp3->begin(file, out);
  LOGF ("play: %s\n", filename);
}

void audio_stop() 
{
  mp3->stop();
  file->close();
  LOG("stop");
}

bool audio_loop()
{
  if (mp3->isRunning()) {
    if (mp3->loop()) return true;
    else audio_stop();
  }
  return false;
}
