// https://github.com/earlephilhower/ESP8266Audio

#include "AudioFileSourceSD.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2SDAC.h"

AudioGeneratorMP3 *mp3;
AudioFileSourceSD *file;
AudioOutputI2SDAC *out;
String currentFile = "";
bool loopMedia = false;
bool sdOK = false;
String errorPlayer = "";

float GAIN_BASE = 0.1;

void audio_setup()
{ 
  // CS / SS  GPIO for SD module
  if (!SD.begin(0)) {
    LOG("SD card error");
  } 
  else {
    LOG("SD card OK");
    sdOK = true;
  }

  if (settings_speaker() == 1) {
    GAIN_BASE = 0.1;
  }
  else if (settings_speaker() == 2) {
    GAIN_BASE = 0.05;
  }
  
  out = new AudioOutputI2SDAC();
  //out->SetBitsPerSample(16);
  //out->SetRate(44100);
  out->SetGain(GAIN_BASE);  

  mp3 = new AudioGeneratorMP3();
}

void audio_play(String filePath) 
{
  if (mp3->isRunning()) audio_stop();
  if (filePath == "") return;
  
  file = new AudioFileSourceSD(filePath.c_str());
  if (mp3->begin(file, out)) {
    currentFile = filePath;
    errorPlayer = "";
    LOG("play: "+filePath);
  }
  else {
    errorPlayer = "not found ("+filePath+")";
    audio_stop();
  }
}

void audio_stop() 
{
  if (!mp3->isRunning()) return;
  mp3->stop();
  file->close();
  currentFile = "";
  errorPlayer = "";
  LOG("stop");
}

void audio_volume(int vol) 
{
  float v = vol * GAIN_BASE / 100.0;
  out->SetGain(v);
  LOGF("gain: %f\n", v);  
}

void audio_loop(bool doLoop) 
{
  loopMedia = doLoop;
}

bool audio_loop()
{
  if (mp3->isRunning()) {
    if (mp3->loop()) return true;
    else if (loopMedia && currentFile != "") {
      //audio_play(currentFile);
      file->seek(0,SEEK_SET);
      LOG("loop: "+currentFile);
      return true;
    }
    else audio_stop();
  }
  return false;
}

bool audio_running() {
  return mp3->isRunning();
}

