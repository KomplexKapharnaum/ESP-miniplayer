#include <SPI.h>
#include <SD.h>

#define MAX_BANK 17      //17
#define MAX_NOTE 128    //128
#define MAX_TITLE 10

char sd_notes[MAX_BANK][MAX_NOTE][MAX_TITLE];

SemaphoreHandle_t sd_lock = xSemaphoreCreateMutex();

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
  xSemaphoreTake(sd_lock, portMAX_DELAY);
  
  // Init Alias array
  for (byte bank = 0; bank < MAX_BANK; bank++)
    for (byte note = 0; note < MAX_NOTE; note++)
      for (byte k = 0; k < MAX_TITLE; k++)
        sd_notes[bank][note][k] = 0;

  // Check Bank dirs

  LOG("\nScanning...");
  for (byte i = 0; i < MAX_BANK; i++) {
    if (SD.exists("/" + pad3(i))) {
      LOG("Scanning bank " + pad3(i));
      File dir = SD.open("/" + pad3(i));

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
  xSemaphoreGive(sd_lock);
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
  xSemaphoreTake(sd_lock, portMAX_DELAY);
  if (sd_notes[bank][note][0] > 1) path += String(sd_notes[bank][note]);
  xSemaphoreGive(sd_lock);
  path += ".mp3";
  //if (SD.exists(path)) return path;
  //else return "";
  return path;
}

int sd_noteSize(byte bank, byte note) {
  int sizeN = 0;
  String path = sd_getPathNote(bank, note);
  if (SD.exists(path)) sizeN = SD.open(path).size();
  return sizeN;  
}

void sd_noteDelete(byte bank, byte note) {
  String path = sd_getPathNote(bank, note);
  if (SD.exists(path)) {
    SD.remove(path);
    xSemaphoreTake(sd_lock, portMAX_DELAY);
    sd_notes[bank][note][0] = 0;
    xSemaphoreGive(sd_lock);
    LOG("deleted " + path);
  }
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




