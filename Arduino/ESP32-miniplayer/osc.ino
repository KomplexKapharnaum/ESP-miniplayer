#include <WiFiUdp.h>
#include <OSCMessage.h> //https://github.com/stahlnow/OSCLib-for-ESP8266

WiFiUDP udp_in;
WiFiUDP udp_out;

const int recv_port = 10000;
const int send_port = 12000;
unsigned long last_beacon = 0;

IPAddress serverIP;
char incomingPacket[1472];
String currentData;

IPAddress linkIP;
bool linkStatus = false;
String lastPacket = "";

void osc_setup()
{
  udp_in.begin(recv_port);

  IPAddress myIP = WiFi.localIP();
  IPAddress mask = WiFi.subnetMask();

  serverIP[0] = myIP[0] | (~mask[0]);
  serverIP[1] = myIP[1] | (~mask[1]);
  serverIP[2] = myIP[2] | (~mask[2]);
  serverIP[3] = myIP[3] | (~mask[3]);

  LOGINL("OSC: Broadcasting on ");
  LOG(serverIP);
}

void osc_loop()
{
  int packetSize = udp_in.parsePacket();
  if (packetSize)
  {
    //Serial.printf("Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
    int len = udp_in.read(incomingPacket, 1470);
    if (len >= 0) incomingPacket[len] = 0;
    else return;

    //LOGF("UDP: packet received: %s\n", incomingPacket);

    currentData = String(incomingPacket);
    String data = osc_next();

    // Check /esp
    data = osc_next();
    if ( data != "esp") {
      LOGF("Invalid packet: %s\n", incomingPacket);
      return osc_clear();
    }

    // Check MSG-COUNTER or /manual
    data = osc_next();
    if (data != "manual") {
      if ( data == lastPacket) {
        //LOG("Already played");
        return osc_clear();
      }
      lastPacket = data;
    }

    // Set Server IP
    linkIP = udp_in.remoteIP();
    if (!linkStatus) {
      linkStatus = true;
    }
    
    // Check identity
    data = osc_next();
    if (data != osc_id() && data != "all" && data != osc_ch()) {
      LOG("Not for me ... ");
      return osc_clear();
    }

    // Command
    data = osc_next();
    if (data == "hello") ;
    else if (data == "sync") {
      sync_setHost(linkIP);
      sync_do(osc_next().toInt());
    }
    else if (data == "stop") audio_stop();
    else if (data == "play") {
      String mediaPath = sd_getPathNote(osc_next().toInt(), osc_next().toInt());
      audio_volume(osc_next().toInt());
      audio_play(mediaPath);
    }
    else if (data == "playtest") {
      String mediaPath = "test.mp3";
      audio_volume(100);
      audio_play(mediaPath);
    }
    else if (data == "volume") audio_volume(osc_next().toInt());
    else if (data == "setchannel") settings_set("channel", osc_next().toInt());
    else if (data == "setid") settings_set("id", osc_next().toInt());
    else if (data == "loop") audio_loop((bool) osc_next().toInt());
    else if (data == "reset") stm32_reset();
    else if (data == "shutdown") stm32_shutdown();
    else LOGF ("Command unknown: %s\n", data.c_str());

    return osc_clear();
  }

  osc_beacon();
}

//
// Send info beacon as heartbeat (status and autodiscovery)
//
void osc_beacon()
{
  if ((millis() - last_beacon) < 800) return;

  udp_out.beginPacket(serverIP, send_port);

  // OSC over UDP
  OSCMessage msg("/iam/esp");
  //msg.add((uint32_t)ESP.getEfuseMac());
  msg.add(settings_get("id"));
  msg.add(MP_VERSION);
  msg.add((int)recv_port);
  msg.add(settings_get("channel"));
  msg.add(linkStatus);
  msg.add(audio_sdOK);
  msg.add(sync_size());
  if (audio_currentFile != "") msg.add(audio_currentFile.c_str());
  else msg.add("stop");
  msg.add(audio_errorPlayer.c_str());
  msg.add(stm32_batteryLevel());
  msg.add(sync_errorMsg().c_str());
  msg.send(udp_out);

  udp_out.endPacket();
  last_beacon = millis();

  //LOG("beacon");
}

//
// Unpack /esp/manual/who/how/what
//
String osc_next() {
  String dataCopy(currentData.c_str());
  for (int i = 0; i < dataCopy.length(); i++) {
    if (dataCopy.substring(i, i + 1) == "/") {
      currentData = dataCopy.substring(i + 1);
      return dataCopy.substring(0, i);
      break;
    }
  }
  osc_clear();
  return dataCopy;
}

//
// Clear data received buffer
//
void osc_clear() {
  currentData = "";
}

String osc_id() {
  return String(settings_get("id"));
}

String osc_ch() {
  byte ch = settings_get("channel");
  String ans = "c";
  if (ch < 10) ans += "0";
  ans += String(ch);
  return ans;
}

bool osc_isLinked() {
  return linkStatus;
}

IPAddress osc_iplink() {
  return linkIP;
}


