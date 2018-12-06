#include <WiFiUdp.h>
#include <OSCMessage.h> //https://github.com/stahlnow/OSCLib-for-ESP8266

#define RPM_BUFFER_SIZE 10
#define RPM_BUFFER_SAMPLINGTIME 500
#define RPM_MAGNETCOUNT 3

IPAddress serverIP;
int send_port = 3753;

int periodBuffer[RPM_BUFFER_SIZE];
int bufferIndex = 0;
int rpmcount;
int last_rpm;

SemaphoreHandle_t rpm_lock;

void rpm_start() {
  IPAddress myIP = WiFi.localIP();
  IPAddress mask = WiFi.subnetMask();

  serverIP[0] = myIP[0] | (~mask[0]);
  serverIP[1] = myIP[1] | (~mask[1]);
  serverIP[2] = myIP[2] | (~mask[2]);
  serverIP[3] = myIP[3] | (~mask[3]);

  LOGINL("OSC: Broadcasting on ");
  LOG(serverIP);

  rpm_lock = xSemaphoreCreateMutex();
  xTaskCreatePinnedToCore( rpm_task, "rpm_task", 1000, NULL, 0, NULL, 1);
}

void rpm_task( void * parameter ) {

  unsigned long now;
  unsigned long average = 0;
  WiFiUDP udp_out;

  while (true) {
    now = millis();

    // read current frame
    xSemaphoreTake(rpm_lock, portMAX_DELAY);
    periodBuffer[bufferIndex] = rpmcount;
    rpmcount = 0;
    xSemaphoreGive(rpm_lock);
    bufferIndex += 1;
    if (bufferIndex >= RPM_BUFFER_SIZE) bufferIndex = 0;

    // calculate average RPM
    average = 0;
    for (int i = 0; i < RPM_BUFFER_SIZE; i++) {
      //LOGINL(periodBuffer[i]);
      //LOGINL(" / ");
      average += periodBuffer[i];
    }
    average = average * 60 * 1000 / (RPM_BUFFER_SIZE * RPM_BUFFER_SAMPLINGTIME * RPM_MAGNETCOUNT);  // average RPM

    if (average != last_rpm) {
      last_rpm = average;

      // SEND OSC (2 times)
      for (int k=0; k<2; k++) {
        udp_out.beginPacket(serverIP, send_port);
        OSCMessage msg("/gegenrpm");
        msg.add(last_rpm);
        msg.send(udp_out);
        udp_out.endPacket();
      }
    }
    LOG(average);

    vTaskDelay( (RPM_BUFFER_SAMPLINGTIME - millis() + now) / portTICK_PERIOD_MS );
  }
  vTaskDelete(NULL);
}

void rpm_inc() {
  xSemaphoreTake(rpm_lock, portMAX_DELAY);
  rpmcount += 1;
  xSemaphoreGive(rpm_lock);
  LOG("ping");
}


//
// Unpack /esp/manual/who/how/what
//
String oscC_next(String& currentData) {
  String dataCopy(currentData.c_str());
  for (int i = 0; i < dataCopy.length(); i++) {
    if (dataCopy.substring(i, i + 1) == "/") {
      currentData = dataCopy.substring(i + 1);
      return dataCopy.substring(0, i);
      break;
    }
  }
  currentData = "";
  return dataCopy;
}
