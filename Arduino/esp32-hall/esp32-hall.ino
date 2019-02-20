#include "logs.h"
#include "wifi.h"
#define REFRESH 1000
#define SNAPSIZE 10
#define DEBOUNCESIZE 4

#define MP_VERSION  0.21

bool trig = false;
int analog_value;
unsigned long last_measure = 0;

void setup() {
  
  LOGSETUP();
  LOG("Starting");
  
  wifi_set_hostname("esp-hall v" + String(MP_VERSION, 2) );
  //wifi_connect("kxkm-wifi", "KOMPLEXKAPHARNAUM");
  wifi_connect("kxkm24");
  wifi_ota();
  wifi_onConnect(doOnConnect);

  rpm_start();
}

void loop() {

  analog_value = analogRead(32);
  if (analog_value == 0) {
    if (!trig) {
      trig = true;
      rpm_inc();
    }
  }
  else trig = false;

  ArduinoOTA.handle();
  delay(1);

}

void doOnConnect() {
  
}
