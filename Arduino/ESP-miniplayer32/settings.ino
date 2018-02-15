#include <EEPROM.h>
byte nodeID;
byte nodeCH;
byte nodeGAIN;

void settings_setup() {
  
  EEPROM.begin(128);
  
  // NODE ID
  #ifdef NODE_ID
    EEPROM.write(0, NODE_ID);
  #endif

  // NODE CHANNEL
  #ifdef NODE_CH
    EEPROM.write(1, NODE_CH);
  #endif

  // NODE SPEAKER
  #ifdef GAIN_HARD
    EEPROM.write(2, GAIN_HARD);
  #endif

  nodeID = EEPROM.read(0);
  nodeCH = EEPROM.read(1);
  nodeGAIN = EEPROM.read(2);
  EEPROM.end();
}

byte settings_gain() {
  return nodeGAIN;
}

void settings_gainset(byte g) {
  EEPROM.begin(128);
  EEPROM.write(2, g);
  EEPROM.end();

  nodeGAIN = g;
}

byte settings_id() {
  return nodeID;
}

char* settings_idstr() {
  char idSTR[3];
  sprintf(idSTR, "%03i", nodeID);
  return idSTR;
}

void settings_idset(byte id) {
  EEPROM.begin(128);
  EEPROM.write(0, id);
  EEPROM.end();

  nodeID = id;
}

char* settings_name() {
  char nodeName[18];
  sprintf(nodeName, "%s%03i","ESP-miniplayer-", nodeID);
  return nodeName;
}

byte settings_ch() {
  return nodeCH;
}

char* settings_chstr() {
  char chSTR[3];
  sprintf(chSTR, "c%02i", nodeCH);
  return chSTR;
}

void settings_chset(byte ch) {
  EEPROM.begin(128);
  EEPROM.write(1, ch);
  EEPROM.end();

  nodeCH = ch;
}

