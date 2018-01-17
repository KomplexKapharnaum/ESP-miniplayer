#include <EEPROM.h>
byte nodeID;
byte nodeCH;

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

  nodeID = EEPROM.read(0);
  nodeCH = EEPROM.read(1);
  EEPROM.end();
}

byte settings_id() {
  return nodeID;
}

char* settings_idstr() {
  char idSTR[3];
  sprintf(idSTR, "%03i", nodeID);
  return idSTR;
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
  sprintf(chSTR, "c%02i", nodeID);
  return chSTR;
}
