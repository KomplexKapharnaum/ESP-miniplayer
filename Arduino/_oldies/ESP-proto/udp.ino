#include <WiFiUdp.h>

WiFiUDP udp_in;
WiFiUDP udp_out;

const int recv_port = 10000;
const int send_port = 12000;
unsigned long last_beacon = 0;

IPAddress serverIP(255, 255, 255, 0);
char incomingPacket[1472];

void udp_setup() 
{
    udp_in.begin(recv_port);
    
}

void udp_loop() 
{
  int packetSize = udp_in.parsePacket();
  if (packetSize)
  {
    //Serial.printf("Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
    int len = udp_in.read(incomingPacket, 1470);
    if (len > 0) incomingPacket[len] = 0;
    else return;
    
    LOGF("UDP packet contents: %s\n", incomingPacket);

    String packet(incomingPacket);
    if (packet.substring(0,5) == "/esp/") {
      if (packet.substring(5,8) == settings_idstr() || packet.substring(5,8) == "all" || packet.substring(5,8) == settings_chstr()) {
        if (packet.substring(9,13) == "stop") audio_stop();
        else if (packet.substring(9,13) == "play") audio_play(packet.substring(13).c_str());
        else LOGF ("command unknown: %s\n", packet.substring(9,13).c_str());
      }
      else LOGF ("ID unknown: %s\n", packet.substring(5,8).c_str());
    }
    else LOGF ("Identifier unknown: %s\n", packet.substring(0,5).c_str());
  }
}

void udp_beacon()
{
   
}


