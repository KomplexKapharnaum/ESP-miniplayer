// https://github.com/earlephilhower/ESP8266Audio

#include "AudioFileSourceSD.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2SDAC.h"

AudioGeneratorMP3 *mp3;
AudioFileSourceSD *file;
AudioOutputI2SDAC *out;

void audio_setup()
{
  SD.begin(0); // CS / SS  GPIO for SD module
  
  out = new AudioOutputI2SDAC();
  //out->SetBitsPerSample(16);
  //out->SetRate(44100);
  out->SetGain(0.08);  

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
  if (!mp3->isRunning()) return;
  mp3->stop();
  file->close();
  LOG("stop");
}

void audio_volume(int vol) 
{
  float v = vol / 200.0;
  out->SetGain(v);  
}

bool audio_loop()
{
  if (mp3->isRunning()) {
    if (mp3->loop()) return true;
    else audio_stop();
  }
  return false;
}
