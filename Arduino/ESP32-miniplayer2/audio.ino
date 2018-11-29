//https://github.com/earlephilhower/ESP8266Audio
//https://github.com/Gianbacchio/ESP8266_Spiram
#include "AudioFileSourceSD.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"

#include "PCM51xx.h"  //https://github.com/tommag/PCM51xx_Arduino
#include "Wire.h"

PCM51xx pcm(Wire); //Using the default I2C address 0x74

AudioGeneratorMP3 *mp3;
AudioFileSourceSD *file;
AudioOutputI2S *out;

SemaphoreHandle_t audio_lock = xSemaphoreCreateMutex();

String audio_currentFile = "";
bool audio_doLoop = false;
bool audio_sdOK = false;
bool audio_engineOK = false;
String audio_errorPlayer = "";

int gainMin = 120;
int gainMax = 60;

bool audio_init()
{   
  if (SD.exists("/")) audio_sdOK = true;
  else audio_sdOK = sd_setup();

  // Prototype
  if (settings_get("model") == 0) {
    gainMax = 20;
  }
  // Big speakers
  else if (settings_get("model") == 1) {
    gainMin = 140;
    gainMax = 60;
  }
  // Small speakers
  else if (settings_get("model") == 2) {
    gainMin = 140;
    gainMax = 75;
  }
  

  out = new AudioOutputI2S(0,AudioOutputI2S::EXTERNAL_I2S, 8, AudioOutputI2S::APLL_DISABLE);
  //out->SetBitsPerSample(16);
  //out->SetRate(44100);

  // KXKM card: using PCM51xx
  bool pcmOK = true;
  if ( settings_get("model") > 0 ) {
    out->SetPinout(25, 27, 26); //HW dependent ! BCK, LRCK, DATA
    out->SetGain( 1 );
    Wire.begin(2, 4); //HW dependent ! SDA, SCL

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
  }
  
  audio_volume(100);
  mp3 = new AudioGeneratorMP3();
  audio_engineOK = pcmOK;
  return audio_engineOK;
}

bool audio_play(String filePath)
{ 
  if (!audio_engineOK) {
    xSemaphoreTake(audio_lock, portMAX_DELAY);
    audio_errorPlayer = "engine not ready";
    xSemaphoreGive(audio_lock);
    return false;
  }
  
  if (audio_running()) audio_stop();
  if (filePath == "") return false;

  xSemaphoreTake(audio_lock, portMAX_DELAY);
  file = new AudioFileSourceSD(filePath.c_str());
  bool isStarted = mp3->begin(file, out);
  //pcm.unmute();
  xSemaphoreGive(audio_lock);
  
  if (isStarted) {
    xSemaphoreTake(audio_lock, portMAX_DELAY);
    audio_currentFile = filePath;
    audio_errorPlayer = "";
    xSemaphoreGive(audio_lock);
    LOG("play: " + filePath);
  }
  else {
    LOG("not found: " + filePath);
    xSemaphoreTake(audio_lock, portMAX_DELAY);
    audio_errorPlayer = "not found (" + filePath + ")";
    xSemaphoreGive(audio_lock);
    audio_stop();
  }
  
  return isStarted;
}

void audio_stop()
{
  if (!audio_engineOK) return;
  
  if (audio_running()) {
    xSemaphoreTake(audio_lock, portMAX_DELAY);
    //pcm.mute();
    mp3->stop();
    file->close();
    audio_currentFile = "";
    audio_errorPlayer = "";
    LOG("stop");
    xSemaphoreGive(audio_lock);
  }
}

void audio_volume(int vol)
{ 
  if (!audio_engineOK) return;
  
  LOGF("GAIN: %i\n", vol);
  xSemaphoreTake(audio_lock, portMAX_DELAY);
  if (settings_get("model") > 0) {
    vol = map(vol, 0, 100, gainMin, gainMax);
    pcm.setVolume(vol);
  }
  else {
    float v = vol * gainMax / 10000.0;
    out->SetGain(v);
    LOGF("gain: %f\n", v);
  }
  xSemaphoreGive(audio_lock);
}

void audio_loop(bool doLoop)
{
  audio_doLoop = doLoop;
}

bool audio_run()
{
  if (audio_running()) {
    xSemaphoreTake(audio_lock, portMAX_DELAY);
    if (mp3->loop()) {
      xSemaphoreGive(audio_lock);
      return true;
    }
    else if (audio_doLoop && audio_currentFile != "") {
      //audio_play(currentFile);
      file->seek(0, SEEK_SET);
      LOG("loop: " + audio_currentFile);
      xSemaphoreGive(audio_lock);
      return true;
    }
    else {
      xSemaphoreGive(audio_lock);
      audio_stop();
      return false;
    }
  }
}

bool audio_running() {
  xSemaphoreTake(audio_lock, portMAX_DELAY);
  bool r = mp3->isRunning();
  xSemaphoreGive(audio_lock);
  return r;
}

String audio_media() {
  xSemaphoreTake(audio_lock, portMAX_DELAY);
  String c = audio_currentFile;
  xSemaphoreGive(audio_lock);
  return c;
}

String audio_Error() {
  xSemaphoreTake(audio_lock, portMAX_DELAY);
  String c = audio_errorPlayer;
  xSemaphoreGive(audio_lock);
  return c;
}
