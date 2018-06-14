#include <HTTPClient.h>
#include "UDHttp.h"  // Manual dl from https://github.com/nhatuan84/esp-upload-download-file-http

enum : int { 
  SYNC_NOLINK = -1,
  SYNC_WAIT = 0, 
  SYNC_RUN = 1,
  SYNC_DONE = 2
  };

int sync_state = SYNC_NOLINK;
int sync_count = 0;
char sync_host[26];
int lastStamp = 0;
int lastPct = 0;
int dlSize = 0;
File dlFile;

int bank_stamp[MAX_BANK] = {0};

String sync_statusMsg = "";
SemaphoreHandle_t sync_lock = xSemaphoreCreateMutex();

void sync_do(int stamp) {
  
  if (stamp <= lastStamp) return;   // this update is already done
  if (sync_state == SYNC_DONE) sync_state = SYNC_WAIT;   // reset state
  if (sync_state != SYNC_WAIT) return;      // not ready to sync

  lastStamp = stamp;
  sync_state = SYNC_RUN;
  sync_count = 0;
  xTaskCreate(
    sync_task, /* Task function. */
    "Sync Task", /* name of task. */
    20000, /* Stack size of task */
    NULL, /* parameter of the task */
    0, /* priority of the task */
    NULL); /* Task handle to keep track of created task */
}

void sync_task(void * parameter) {
  
  LOG("\nSyncing...");
  sync_setStatus("Syncing..");

  for (byte bank = 0; bank < MAX_BANK; bank++) {
    LOG("Syncing BANK " + String(bank));
    sync_bankCheck(bank);
  }

  sync_state = SYNC_DONE;
  LOG("Sync done: " + String(sync_count) + " files");
  sd_scanNotes();  // re-scan SD card

  sync_setStatus("");
  vTaskDelete( NULL );
}

void sync_bankCheck(byte bank) {

  sync_setStatus("Get bank"+String(bank));
  HTTPClient http;
  http.setTimeout(5000);
  
  xSemaphoreTake(sync_lock, portMAX_DELAY);
  http.begin(sync_host, 3742, "/listbank/"+String(bank));
  xSemaphoreGive(sync_lock);
  
  if (http.GET() != 200) {
    LOG("Sync: can't get files list");
    sync_setStatus("Can't get list Bank "+String(bank));
    return;
  }
  sync_setStatus("Scanning Bank "+String(bank));

  String fileList = http.getString();
  http.end();

  while (fileList.indexOf('\n') >= 0) {
    String payload = fileList.substring(0, fileList.indexOf('\n'));
    fileList = fileList.substring(fileList.indexOf('\n') + 1);

    sync_fileCheck(payload);
    //LOG("payload: "+payload);
    yield();
  }
  sync_setStatus("Done bank"+String(bank));
}

void sync_fileCheck(String payload) {

  // get bank & note
  byte bank = payload.substring(0, 3).toInt();
  byte note = payload.substring(4, 7).toInt();

  // get size from 4 next bytes + filename
  int fsize = payload.substring(8, 18).toInt();
  String file = payload.substring(19);

  //LOG(payload);
  //LOG("file "+file+" size "+String(fsize));
  
  if (fsize == 0) return; // skip empty file on remote 
  
  sync_setStatus("Checking file "+file);

  // check Size
  if (fsize == sd_noteSize(bank, note)) {
    if (fsize > 0) {
      LOG("ok " + file);
      sync_setStatus("File ok "+file);
      sync_count += 1;
    }
    return;
  }

  // Delete wrong size file
  sync_setStatus("Delete file "+file);
  sd_noteDelete(bank, note);

  // missing file: download it
  if (fsize > 0) {
    LOG("missing " + file + " (" + String(fsize) + ")");
    sync_setStatus("Missing file "+file);

    if ( !SD.exists("/"+pad3(bank)) ) {
      LOG("Creating directory /" + pad3(bank));
      SD.mkdir("/"+pad3(bank));
    }

    UDHttp udh;
    lastPct = 0;
    dlSize = 0;
    dlFile = SD.open(file, FILE_WRITE);
    uint32_t startTime = millis();
    LOG("downloading "+file);
    
    char url[100];
    xSemaphoreTake(sync_lock, portMAX_DELAY);
    ("http://"+String(sync_host)+":3742/get"+file).toCharArray(url, 100);
    xSemaphoreGive(sync_lock);
    byte retry = 0;
    int result = -1;
    while(retry<5 && result<0) {
      if (retry > 0) sync_setStatus("Retrying file "+String(file));
      else sync_setStatus("Get file "+String(file));
      result = udh.download(url, sync_writeData, sync_progress);
      retry += 1;
    }
    dlFile.close();

    if (result == 0 && dlSize == fsize && result >= 0) {
      int timed = (millis() - startTime);
      if (timed == 0) timed += 1;
      LOG("download done: " + String(dlSize / 1024) + "kB in " + String(timed / 1000) + "s -> " + String((dlSize / 1024) * 1000 / timed) + "kB/s");
      sync_count += 1;
      sync_setStatus("Done file "+String(file));
    }
    else {
      LOG("Download error");
      sync_setStatus("Abort file "+String(file));
    }
  }

}

int sync_writeData(uint8_t *buffer, int len){
  //write downloaded data to file
  dlSize += len;
  return dlFile.write(buffer, len);
}

void sync_progress(int percent){
  if (percent > lastPct) {
    //LOGF("%d\n", percent);
    lastPct = percent;
  }
  sync_setStatus(String(percent)+"%");
}

void sync_setHost(IPAddress host) {
  if (sync_state != SYNC_NOLINK) return; // already got an host
  xSemaphoreTake(sync_lock, portMAX_DELAY);
  sprintf(sync_host, "%d.%d.%d.%d", host[0], host[1], host[2], host[3]);
  xSemaphoreGive(sync_lock);
  sync_state = SYNC_WAIT;
}


int sync_size() {
  return sync_count;
}

int sync_getState() {
  return sync_state;
}

void sync_setStatus(String statusMsg) {
  xSemaphoreTake(sync_lock, portMAX_DELAY);
  sync_statusMsg = statusMsg;
  xSemaphoreGive(sync_lock);
}

String sync_getStatus() {
  xSemaphoreTake(sync_lock, portMAX_DELAY);
  String statu = sync_statusMsg;
  xSemaphoreGive(sync_lock);
  return statu;
}


