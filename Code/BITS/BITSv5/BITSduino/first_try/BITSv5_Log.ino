#include "BITSv5_Log.h"

uint32_t cnt = 0;
bool okayToWrite = true;

// Make the CSV file and give it a simple header
void headerCSV() {
  File f = LittleFS.open("data.csv", "w");
  f.printf("sample,millis,temp,rand\n");
  f.close();
  cnt = 0;
}

// Called when the USB stick connected to a PC and the drive opened
// Note this is from a USB IRQ so no printing to SerialUSB/etc.
void plug(uint32_t i) {
  (void) i;
  okayToWrite = false;
}

// Called when the USB is ejected or removed from a PC
// Note this is from a USB IRQ so no printing to SerialUSB/etc.
void unplug(uint32_t i) {
  (void) i;
  okayToWrite = true;
}

// Called when the PC tries to delete the single file
// Note this is from a USB IRQ so no printing to SerialUSB/etc.
void deleteCSV(uint32_t i) {
  (void) i;
  LittleFS.remove("data.csv");
  headerCSV();
}

void log_init(void) {
  LittleFS.begin();

  // Set up the USB disk share
  singleFileDrive.onDelete(deleteCSV);
  singleFileDrive.onPlug(plug);
  singleFileDrive.onUnplug(unplug);
  singleFileDrive.begin("data.csv", "Recorded data from the Raspberry Pi Pico.csv");

  // Find the last written data
  File f = LittleFS.open("data.csv", "r");
  if (!f || !f.size()) {
    cnt = 1;
    headerCSV();
  } else {
    if (f.size() > 2048) {
      f.seek(f.size() - 1024);
    }
    do {
      String s = f.readStringUntil('\n');
      sscanf(s.c_str(), "%lu,", &cnt);
    } while (f.available());
    f.close();
    cnt++;
  }
}

static void logprint_str(String s) {
  File dataFile = SD.open(eventLogName, FILE_WRITE);
  dataFile.print(s);
  dataFile.close();
}

static void logprint_int(int i) {
  File dataFile = SD.open(eventLogName, FILE_WRITE);
  dataFile.print(i);
  dataFile.close();
}

static void logprint_double(double d) {
  File dataFile = SD.open(eventLogName, FILE_WRITE);
  dataFile.print(d);
  dataFile.close();
}

static void logprintln_str(String s) {
  File dataFile = SD.open(eventLogName, FILE_WRITE);
  dataFile.println(s);
  dataFile.close();
}

static void logprintln_int(int i) {
  File dataFile = SD.open(eventLogName, FILE_WRITE);
  dataFile.println(i);
  dataFile.close();
}

static void logprintln_double(double d) {
  File dataFile = SD.open(eventLogName, FILE_WRITE);
  dataFile.println(d);
  dataFile.close();
}

static void logprintln_uint32(uint32_t d) {
  File dataFile = SD.open(eventLogName, FILE_WRITE);
  dataFile.println(d, HEX);
  dataFile.close();
}

void LogPacket() {
  if (millis() - lastLog > gpsLogInterval) {
    char gpsLogPacket2[50];

    char exactTime[9]; // A somewhat convaluted way of adding : into the integer
                       // timestamp... //TEST
    snprintf(exactTime, 7, "%d", gpsInfo.GPSTime);
    char hour[3];
    char min[3];
    char sec[3];
    strncpy(hour, &exactTime[0], 2);
    hour[2] = '\0';
    strncpy(min, &exactTime[2], 2);
    min[2] = '\0';
    strncpy(sec, &exactTime[4], 2);
    sec[2] = '\0';
    snprintf(exactTime, 9, "%s:%s:%s", hour, min, sec);

    // snprintf(gpsLogPacket2,35,"%s,%4.4f,%4.4f,%u",exactTime,gpsInfo.GPSLat,gpsInfo.GPSLon,gpsInfo.GPSAlt);
    // //Fix Timestep
    snprintf(gpsLogPacket2, 45, "%06d,%4.5f,%4.5f,%u", gpsInfo.GPSTime,
             gpsInfo.GPSLat, gpsInfo.GPSLon,
             gpsInfo.GPSAlt); // Build the packet
    gpsLogFile.println(gpsLogPacket2);

    gpsLogFile.flush(); // WIP
    lastLog = millis();
  }
}
