// Celltracker Code, for the medium old celltracker board v2
// Written by Shailesh and Jonathan along with the use of some public libraries
// Transmits its position over the cell network and logs position

// Runs on a literal $2 Arduino Nano clone, we really need new hardware...

#include <TinyGPS.h>
#include <SoftwareSerial.h>
#include <SD.h>
#include <jtXBee.h>

#define GPSBaud 9600

const int RXPin = 5, TXPin = 6, GPSon = 2, SDpin = 10;

File GPSlog;

//Need to use Old Bootloader to compile // WHAT DOES THIS MEAN THOUGH
// The TinyGPS++ object
TinyGPS gps;

struct GPSdata{
  float GPSLat=-1;
  float GPSLon=-1;
  unsigned long GPSTime=-1;
  unsigned long GPSSpeed=-1;
  long GPSAlt=-1;
  int GPSSats=-1;
  long GPSCourse = -1;
};
 //The library object
//unsigned long cur = 0; del
GPSdata gpsInfo; //Current obj
//GPSdata preserve;//Last obj (for getting deltas)
// The serial connection to the GPS device
SoftwareSerial gpsserial(RXPin, TXPin);
SoftwareSerial xbeeSerial(3, 2);
const int t1 = 1000;
const long int t3 = 300000;
unsigned long previousMillis1 = 0;
unsigned long previousMillis2 = 0;
unsigned long previousMillis3 = 0;
int x = 0;


char xbeeBuffer[100];
const int bufSize = 100;
jtXBee celltracker(xbeeBuffer, bufSize);

char packetData[50] = "init";
char phonenumber[12] = "7324849689";

void setup()
{
  xbeeSerial.begin(9600);

  GPSINIT(GPSBaud);
  delay(1000);
  Serial.begin(57600);
  delay(10);
  

  Serial.println(F("INIT_TEXT"));

  // Generate outputData containing packetData to be txSMS to phonenumber
  char outputData[80];
  memset(outputData, 0, 80);
  celltracker.txSMS(phonenumber, packetData, outputData);
  // Send outputData to XBee
  for(int i = 0; i < 80; i++){
    //xbeeSerial.write(outputData[i]);
    Serial.println((int)outputData[i]);
  }
  celltracker.nukeBuffer();
  
  delay(10);
  
  if (!SD.begin(SDpin)) {
    Serial.println(F("SD init fail"));
    return;
  }else{
    Serial.println(F("SD init"));
  }
  GPSlog = SD.open("log.txt", FILE_WRITE);
  if (GPSlog) {
    Serial.println(F("log init"));
    //GPSlog.println(F("Started")); Kinda obvious tbh
  } else {
    Serial.println(F("log init err"));
  }
  
}


// The frickin simplest loop() I've seen in a while
void loop() {
 while (gpsserial.available() > 0)

    //PARSE
    gpsRun();

    //LOG
    if ((millis() - previousMillis1) >= t1) {
      previousMillis1 = millis(); 
      outputSD();
      outputSerial();
    }
    

    //SMS_TX
    if(((millis() - previousMillis3)>t3) && (gpsInfo.GPSAlt<1000) && (x<50)){
      sendText();
      previousMillis3 = millis();
      x++;
      Serial.println(F("SMS_TX"));
    }

}
