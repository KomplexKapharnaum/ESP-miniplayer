//https://github.com/earlephilhower/ESP8266Audio
#include "AudioFileSourceSD.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"

#include "PCM51xx.h"  //https://github.com/tommag/PCM51xx_Arduino
#include "Wire.h"

PCM51xx pcm(Wire); //Using the default I2C address 0x74

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

  out = new AudioOutputI2S(0,AudioOutputI2S::EXTERNAL_I2S, AudioOutputI2S::APLL_ENABLE);
  //out->SetBitsPerSample(16);
  //out->SetRate(44100);

  // KXKM card: using PCM51xx
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
    }
  }
  
  audio_volume(100);
  mp3 = new AudioGeneratorMP3();
  return audio_sdOK;
}

bool audio_play(String filePath)
{
  if (mp3->isRunning()) audio_stop();
  if (filePath == "") return false;

  file = new AudioFileSourceSD(filePath.c_str());
  if (mp3->begin(file, out)) {
    audio_currentFile = filePath;
    audio_errorPlayer = "";
    LOG("play: " + filePath);
    return true;
  }
  else {
    LOG("not found: " + filePath);
    audio_errorPlayer = "not found (" + filePath + ")";
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
  if (settings_get("model") > 0) {
    int v = vol*255*settings_get("gain")/10000;
    v = 255-constrain(v, 0, 255);
    pcm.setVolume(v);
  }
  else {
    float v = vol * settings_get("gain") / 10000.0;
    out->SetGain(v);
    LOGF("gain: %f\n", v);
  }
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
      file->seek(0, SEEK_SET);
      LOG("loop: " + audio_currentFile);
      return true;
    }
    else audio_stop();
  }
  return false;
}

bool audio_running() {
  return mp3->isRunning();
}

String audio_media() {
  return audio_currentFile;
}
