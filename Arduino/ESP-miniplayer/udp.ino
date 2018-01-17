#include <WiFiManager.h> 
#include <WiFiUdp.h>

WiFiUDP udp_in;
WiFiUDP udp_out;

const int recv_port = 10000;
const int send_port = 12000;
unsigned long last_beacon = 0;

IPAddress serverIP;
char incomingPacket[1472];
String currentData;

void udp_setup() 
{
    udp_in.begin(recv_port);
  
    IPAddress myIP = WiFi.localIP();
    IPAddress mask = WiFi.subnetMask();

    serverIP[0] = myIP[0] | !mask[0];
    serverIP[1] = myIP[1] | !mask[1];
    serverIP[2] = myIP[2] | !mask[2];
    serverIP[3] = myIP[3] | !mask[3];

    LOGF4("Broadcast %i.%i.%i.%i\n", serverIP[0], serverIP[1], serverIP[2], serverIP[3]);
}

void udp_loop() 
{
  int packetSize = udp_in.parsePacket();
  if (packetSize)
  {
    //Serial.printf("Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
    int len = udp_in.read(incomingPacket, 1470);
    if (len >= 0) incomingPacket[len] = 0;
    else return;
    
    LOGF("UDP packet contents: %s\n", incomingPacket);

    currentData = String(incomingPacket);
    String data = udp_next();
    
    // Check /esp
    data = udp_next();
    if ( data != "esp") {
      LOG("Invalid packet");
      return udp_clear();
    }
    serverIP = udp_in.remoteIP();

    // Check identity
    data = udp_next();
    if (data != settings_idstr() && data != "all" && data != settings_chstr()) {
      LOG("Not for me ...");
      return udp_clear();
    }

    // Command
    data = udp_next();
    if (data == "stop") audio_stop();
    else if (data == "play") audio_play(udp_next().c_str());
    else if (data == "setchannel") settings_chset(udp_next().toInt());
    else if (data == "setid") settings_idset(udp_next().toInt());
    else LOGF ("Command unknown: %s\n", data.c_str());

    return udp_clear();   
  }

  if ((millis() - last_beacon) > 1000) udp_beacon();
}

void udp_beacon()
{
  udp_out.beginPacket(serverIP, send_port);
  String replyPacekt ("/iam/esp/");
  replyPacekt += settings_id();
  replyPacekt += "/";
  replyPacekt += settings_ch();
  udp_out.write(replyPacekt.c_str());
  udp_out.endPacket();
  last_beacon = millis();
}

String udp_next() {
  String dataCopy(currentData.c_str());
  for (int i = 0; i<dataCopy.length(); i++) {
    if (dataCopy.substring(i, i+1) == "/") {
      currentData = dataCopy.substring(i+1);
      return dataCopy.substring(0, i);
      break;
    }
  }
  udp_clear();
  return dataCopy;
}

void udp_clear() {
  currentData = "";
}


