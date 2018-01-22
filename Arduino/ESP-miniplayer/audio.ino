// https://github.com/earlephilhower/ESP8266Audio

#include "AudioFileSourceSD.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2SDAC.h"

AudioGeneratorMP3 *mp3;
AudioFileSourceSD *file;
AudioOutputI2SDAC *out;
byte currentFile = 0;
bool loopMedia = true;

void audio_setup()
{
  SD.begin(0); // CS / SS  GPIO for SD module
  
  out = new AudioOutputI2SDAC();
  //out->SetBitsPerSample(16);
  //out->SetRate(44100);
  out->SetGain(0.08);  

  mp3 = new AudioGeneratorMP3();
}

void audio_play(byte fileNumber) 
{
  if (mp3->isRunning()) audio_stop();
  if (fileNumber == 0) return;
  
  char fileSTR[3];
  sprintf(fileSTR, "%03i.mp3", fileNumber);
  file = new AudioFileSourceSD(fileSTR);
  mp3->begin(file, out);
  currentFile = fileNumber;
  LOGF ("play: %s\n", fileSTR);
}

void audio_stop() 
{
  if (!mp3->isRunning()) return;
  mp3->stop();
  file->close();
  currentFile = 0;
  LOG("stop");
}

void audio_volume(int vol) 
{
  float v = vol / 200.0;
  out->SetGain(v);  
}

void audio_loop(bool doLoop) 
{
  loopMedia = doLoop;
}

bool audio_loop()
{
  if (mp3->isRunning()) {
    if (mp3->loop()) return true;
    else if (loopMedia && currentFile > 0) {
      //audio_play(currentFile);
      file->seek(0,SEEK_SET);
      LOGF ("loop: %03i\n", currentFile);
      return true;
    }
    else audio_stop();
  }
  return false;
}
