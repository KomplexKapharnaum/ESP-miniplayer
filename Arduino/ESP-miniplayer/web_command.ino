
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "index.h"

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

  switch(type) {
    case WStype_DISCONNECTED:
      LOGF("[%u] Disconnected!\r\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        //LOGF("[%u] Connected from %d.%d.%d.%d url: %s\r\n", num, ip[0], ip[1], ip[2], ip[3], payload);

        // send message to client
        webSocket.sendTXT(num, "Connected");
      }
      break;
    case WStype_TEXT:
      LOGF2("[%u] get Text: %s\r\n", num, payload);

      // Looks for button press "bSFXn=1" messages where n='0'..'9'
      if ((length == 7) &&
          (memcmp((const char *)payload, "bSFX", 4) == 0) &&
          (payload[6] == '1')) {
        switch (payload[4]) {
          case '0':
            audio_play("/T0.mp3");
            break;
          case '1':
            audio_play("/BAMBAM.mp3");
            break;
        }
      }
      else {
        LOGF("Unknown message from client [%s]\r\n", payload);
      }

      // send message to client
      // webSocket.sendTXT(num, "message here");

      // send data to all connected clients
      // webSocket.broadcastTXT("message here");
      break;
    case WStype_BIN:
      LOGF2("[%u] get binary length: %u\r\n", num, length);
      hexdump(payload, length);

      // send message to client
      // webSocket.sendBIN(num, payload, length);
      break;
  }
}

void webserver_setup(void)
{
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  LOG("websocket server started");

  

  // handle "/"
  server.on("/", []() {
      server.send_P(200, "text/html", INDEX_HTML);
  });

  server.begin();
  LOG("web server started");

}

inline void webserver_loop(void) {
  webSocket.loop();
  server.handleClient();
}
