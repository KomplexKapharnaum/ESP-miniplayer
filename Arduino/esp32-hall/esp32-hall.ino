#include "logs.h"
#include "wifi.h"
#define REFRESH 1000
#define SNAPSIZE 10
#define DEBOUNCESIZE 4

#define MP_VERSION  0.1

bool trig = false;
int analog_value;

void setup() {
  
  LOGSETUP();
  LOG("Starting");
  
  wifi_set_hostname("esp-reed v" + String(MP_VERSION, 2) );
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

  
  delay(10);

    

  /*delay(REFRESH);
  detachInterrupt(4);

  // get lowest value (offset)
  byte nextIndex = snapIndex+1;
  if (nextIndex >= SNAPSIZE) nextIndex = 0;
  unsigned long offset = snapshots[nextIndex];

  // average time for a rotation
  average = 0;
  for (int i=0; i<SNAPSIZE; i++)
    average += (snapshots[i]-offset);
  average = average / SNAPSIZE;

  if (average == 0) rpm = 0;
  else rpm = 60*1000/average;

  LOGINL("RPM=");
  LOG(rpm);

  //Restart the interrupt processing
  attachInterrupt(4, rpm_fun, FALLING);

  LOG(rpm);*/

}

void doOnConnect() {
  
}
