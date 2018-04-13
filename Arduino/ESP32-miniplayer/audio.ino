#include "AudioFileSourceSD.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"

AudioGeneratorMP3 *mp3;
AudioFileSourceSD *file;
AudioOutputI2S *out;
String audio_currentFile = "";
bool audio_loopMedia = false;
bool audio_sdOK = false;
String audio_errorPlayer = "";

bool audio_setup()
{ 
  if (SD.exists("/")) audio_sdOK = true;
  else audio_sdOK = sd_setup();
  
  out = new AudioOutputI2S();
  //out->SetBitsPerSample(16);
  //out->SetRate(44100);
  out->SetGain( settings_get("gain") );  

  mp3 = new AudioGeneratorMP3();

  audio_volume(100);
  
  return audio_sdOK;
}

bool audio_play(String filePath) 
{
  if (mp3->isRunning()) audio_stop();
  if (filePath == "") return false;

  filePath = "/"+filePath;
  
  file = new AudioFileSourceSD(filePath.c_str());
  if (mp3->begin(file, out)) {
    audio_currentFile = filePath;
    audio_errorPlayer = "";
    LOG("play: "+filePath);
    return true;
  }
  else {
    LOG("not found: "+filePath);
    audio_errorPlayer = "not found ("+filePath+")";
    audio_stop();
    return false;
  }
}

void audio_stop() 
{
  if (!mp3->isRunning()) return;
  mp3->stop();
  file->close();
  audio_currentFile = "";
  audio_errorPlayer = "";
  LOG("stop");
}

void audio_volume(int vol) 
{
  LOGF("GAIN: %i\n", vol);
  float v = vol * settings_get("gain") / 10000.0;
  out->SetGain(v);
  LOGF("gain: %f\n", v);  
}

void audio_loop(bool doLoop) 
{
  audio_loopMedia = doLoop;
}

bool audio_run()
{
  if (mp3->isRunning()) {
    if (mp3->loop()) return true;
    else if (audio_loopMedia && audio_currentFile != "") {
      //audio_play(currentFile);
      file->seek(0,SEEK_SET);
      LOG("loop: "+audio_currentFile);
      return true;
    }
    else audio_stop();
  }
  return false;
}

bool audio_running() {
  return mp3->isRunning();
}

String audio_media(){
  return audio_currentFile;
}
