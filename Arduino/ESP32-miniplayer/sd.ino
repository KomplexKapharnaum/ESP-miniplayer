#include <SPI.h>
#include <SD.h>

#define MAX_BANK 1      //16
#define MAX_NOTE 10    //128
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

bool sd_syncd = false;

void sd_syncRemote() {
  xTaskCreate(
    sd_syncTask, /* Task function. */
    "Sync Task", /* name of task. */
    20000, /* Stack size of task */
    NULL, /* parameter of the task */
    0, /* priority of the task */
    NULL); /* Task handle to keep track of created task */
}

void sd_syncTask(void * parameter) {
  //if (sd_syncd) return;             // already syncd

  while (!osc_isLinked()) {     // no link to host
    delay(500);
  }

  sd_scanNotes();  // re-scan SD card

  LOG("\nSyncing...");

  char host[26];
  sprintf(host, "%d.%d.%d.%d", osc_iplink()[0], osc_iplink()[1], osc_iplink()[2], osc_iplink()[3]);

  int httpCode;

  HTTPClient dlclient;
  dlclient.begin(host, 3742, "/file");

  HTTPClient http;
  http.begin(host, 3742, "/info");
  for (byte bank = 0; bank < MAX_BANK; bank++)
    for (byte note = 0; note < MAX_NOTE; note++) {
      httpCode = http.POST(String(bank) + "/" + String(note));
      if (httpCode == 200) {
        String payload = http.getString();

        // controlling 2 first bytes
        if (payload.substring(0, 3).toInt() != bank || payload.substring(4, 7).toInt() != note)
          LOG("wrong answer: " + String(bank) + "/" + String(note) + " " + payload);

        else {

          // get size from 4 next bytes + filename
          int fsize = payload.substring(8, 18).toInt();
          String file = payload.substring(19);

          //LOG(payload);
          //LOG("file "+file+" size "+String(fsize));

          // wrong size
          bool wrongsize = sd_notes[bank][note][0] > 0 && fsize != SD.open( sd_getPathNote(bank, note) ).size();
          wrongsize = wrongsize || (fsize > 0 && sd_notes[bank][note][0] == 0);


          if (wrongsize) {

            // exisiting wrong size -> delete it
            if (sd_notes[bank][note][0] > 0) {
              String file = sd_getPathNote(bank, note);
              SD.remove( file );
              sd_notes[bank][note][0] = 0;
              LOG("deleted " + file);
            }

            // missing file: download it
            if (fsize > 0) {
              LOG("missing " + file + " (" + String(fsize) + ")");

              if (dlclient.POST(file) == 200) {

                int len = dlclient.getSize();
                int initlen = len;
                int counter = 0;
                if (len > 0) {

                  // Create directory
                  if (!SD.exists(file.substring(0, 4))) {
                    LOG("Creating directory " + file.substring(0, 4));
                    SD.mkdir(file.substring(0, 4));
                  }

                  // create buffer for read
                  uint8_t buff[1024 * 16] = { 0 };

                  // get tcp stream
                  WiFiClient * stream = dlclient.getStreamPtr();
                  File myFile = SD.open(file, FILE_WRITE);
                  
                  uint32_t startTime = millis();
                  
                  LOGINL("downloading ");
                  LOG(file);

                  // read all data from server
                  while (dlclient.connected() && len > 0) {

                    // get available data size
                    size_t sizeS = stream->available();
                    //LOG(sizeS);

                    if (sizeS) {
                      // read up to 1024 byte
                      int c = stream->readBytes(buff, ((sizeS > sizeof(buff)) ? sizeof(buff) : sizeS));

                      // write it to File
                      myFile.write(buff, c);
                      counter++;
                      if (counter > 10) {
                        counter = 0;
                        LOG(String(((initlen - len) * 100) / initlen) + " %");
                      }
                      if (len > 0) len -= c;
                      yield();
                    }
                  }

                  /*while (stream->available() && len > 0) {
                    byte c = stream->read();
                    //Serial.write(c);
                    if (len > 0) len -= 1;
                    }*/

                  myFile.close();
                  LOG("100%");
                  LOG("done: "+String(fsize/1024)+"kB in "+String((millis()-startTime)/1000)+"s -> "+String((fsize/1024)*1000/(millis()-startTime))+"kB/s");
                }
              }
            }
          }

          // file already ok
          else if (fsize > 0) LOG("ok " + file);
        }
      }
      yield();
    }

  //sd_localList();
  dlclient.end();
  http.end();

  sd_syncd = true;
  LOG("Sync done.");

  vTaskDelete( NULL );
}

