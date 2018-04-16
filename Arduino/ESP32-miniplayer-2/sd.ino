#include <SPI.h>
#include <SD.h>

#define MAX_BANK 2      //16
#define MAX_NOTE 128    //128
#define MAX_TITLE 10

char sd_notes[MAX_BANK][MAX_NOTE][MAX_TITLE];


bool sd_setup() {
  // CS / SS  GPIO for SD module
  if (!SD.begin()) {
    LOG("SD card error");
    return false;
  }
  else {
    LOG("SD card OK");
    return true;
  }
}


void sd_scanNotes() {
  for (byte i = 0; i < MAX_BANK; i++)
    for (byte j = 0; j < MAX_NOTE; j++)
      for (byte k = 0; k < MAX_TITLE; k++)
        sd_notes[i][j][k] = 0;

  // Check Bank dirs
  char bank[4];
  bank[3] = 0;
  LOG("\nScanning...");
  for (byte i = 0; i < MAX_BANK; i++) {
    bank[0] = '0' + i / 100;
    bank[1] = '0' + (i / 10) % 10;
    bank[2] = '0' + i % 10;
    if (SD.exists("/" + String(bank))) {
      LOG("Scanning bank " + String(bank));
      File dir = SD.open("/" + String(bank));

      // Check Notes files
      while (true) {
        File entry =  dir.openNextFile();
        if (!entry) break;
        if (!entry.isDirectory()) {
          int note = 0;
          note = (entry.name()[5] - '0') * 100 + (entry.name()[6] - '0') * 10 + (entry.name()[7] - '0');
          if (note < MAX_NOTE) {
            sd_notes[i][note][0] = 1; // put a stamp even if alias is empty
            byte k = 0;
            while ( (k < MAX_TITLE) && (entry.name()[8 + k] != 0) && (entry.name()[8 + k] != '.') ) {
              sd_notes[i][note][k] = entry.name()[8 + k];
              k++;
            }
          }
          //LOG("File found: "+String(entry.name())+" size="+String(entry.size()));
        }
      }
    }
  }

  LOG("Scan done.");
}

String sd_getPathNote(byte bank, byte note) {
  char b[4];
  b[0] = '0' + bank / 100;
  b[1] = '0' + (bank / 10) % 10;
  b[2] = '0' + bank % 10;
  b[3] = 0;
  char n[4];
  n[0] = '0' + note / 100;
  n[1] = '0' + (note / 10) % 10;
  n[2] = '0' + note % 10;
  n[3] = 0;
  String path = "/" + String(b) + "/" + String(n);
  if (sd_notes[bank][note][0] > 1) path += String(sd_notes[bank][note]);
  path += ".mp3";
  //if (SD.exists(path)) return path;
  //else return "";
  return path;
}



/*void sd_localList() {
  File root;
  root = SD.open("/");
  sd_scandir(root, 0);
  }

  void sd_scandir(File dir, int numTabs) {
  while (true) {

    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      //Serial.println(String(entry.name()).toInt());
      sd_scandir(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
  }*/


#include <HTTPClient.h>
#include "UDHttp.h"

bool sd_syncd = false;
int sd_syncCount = 0;
int lastPct = 0;
int dlSize = 0;
File dlFile;

void sd_syncRemote() {
  sd_syncd = false;
  sd_syncCount = 0;

  xTaskCreate(
    sd_syncTask2, /* Task function. */
    "Sync Task", /* name of task. */
    20000, /* Stack size of task */
    NULL, /* parameter of the task */
    0, /* priority of the task */
    NULL); /* Task handle to keep track of created task */
}

void sd_syncTask2(void * parameter) {
  while (!osc_isLinked()) {     // no link to host
    delay(500);
  }

  sd_scanNotes();  // re-scan SD card
  LOG("\nSyncing...");

  char host[26];
  sprintf(host, "%d.%d.%d.%d", osc_iplink()[0], osc_iplink()[1], osc_iplink()[2], osc_iplink()[3]);

  for (byte bank = 0; bank < MAX_BANK; bank++) {
    LOG("Syncing BANK " + String(bank));
    sd_bankCheck(bank);
  }

  sd_syncd = true;
  LOG("Sync done: " + String(sd_syncCount) + " files");
  sd_scanNotes();  // re-scan SD card

  vTaskDelete( NULL );
}

void sd_bankCheck(byte bank) {
  char host[26];
  sprintf(host, "%d.%d.%d.%d", osc_iplink()[0], osc_iplink()[1], osc_iplink()[2], osc_iplink()[3]);
  HTTPClient http;
  http.begin(host, 3742, "/listbank/"+String(bank));

  if (http.GET() != 200) {
    LOG("Sync: can't get files list");
    vTaskDelete( NULL );
    return;
  }

  String fileList = http.getString();
  http.end();

  while (fileList.indexOf('\n') >= 0) {
    String payload = fileList.substring(0, fileList.indexOf('\n'));
    fileList = fileList.substring(fileList.indexOf('\n') + 1);

    sd_fileCheck(payload);
    //LOG("payload: "+payload);
    yield();
  }
}

void sd_fileCheck(String payload) {

  // get bank & note
  byte bank = payload.substring(0, 3).toInt();
  byte note = payload.substring(4, 7).toInt();

  // get size from 4 next bytes + filename
  int fsize = payload.substring(8, 18).toInt();
  String file = payload.substring(19);

  //LOG(payload);
  //LOG("file "+file+" size "+String(fsize));

  // wrong size
  bool wrongsize = sd_notes[bank][note][0] > 0 && fsize != SD.open( sd_getPathNote(bank, note) ).size();
  wrongsize = wrongsize || (fsize > 0 && sd_notes[bank][note][0] == 0);

  // size is ok
  if (!wrongsize) {
    if (fsize > 0) {
      LOG("ok " + file);
      sd_syncCount += 1;
    }
    return;
  }

  // exisiting wrong size -> delete it
  if (sd_notes[bank][note][0] > 0) {
    String file = sd_getPathNote(bank, note);
    SD.remove( file );
    sd_notes[bank][note][0] = 0;
    LOG("deleted " + file);
  }

  // Create directory
  if (!SD.exists(file.substring(0, 4))) {
    LOG("Creating directory " + file.substring(0, 4));
    SD.mkdir(file.substring(0, 4));
  }

  // missing file: download it
  if (fsize > 0) {
    LOG("missing " + file + " (" + String(fsize) + ")");

    char host[26];
    sprintf(host, "%d.%d.%d.%d", osc_iplink()[0], osc_iplink()[1], osc_iplink()[2], osc_iplink()[3]);

    UDHttp udh;
    lastPct = 0;
    dlSize = 0;
    dlFile = SD.open(file, FILE_WRITE);
    uint32_t startTime = millis();
    LOG("downloading "+file);
    
    char url[100];
    ("http://"+String(host)+":3742/get"+file).toCharArray(url, 100);
    int result = udh.download(url, sd_downdata, sd_downprogress);  
    dlFile.close();

    if (result == 0 && dlSize == fsize) {
      int timed = (millis() - startTime);
      if (timed == 0) timed += 1;
      LOG("download done: " + String(dlSize / 1024) + "kB in " + String(timed / 1000) + "s -> " + String((dlSize / 1024) * 1000 / timed) + "kB/s");
      sd_syncCount += 1;
    }
    else LOG("Download error");
  }

}

int sd_downdata(uint8_t *buffer, int len){
  //write downloaded data to file
  dlSize += len;
  return dlFile.write(buffer, len);
}

void sd_downprogress(int percent){
  if (percent > lastPct) {
    //LOGF("%d\n", percent);
    lastPct = percent;
  }
}

int sd_syncNbr() {
  return sd_syncCount;
}

