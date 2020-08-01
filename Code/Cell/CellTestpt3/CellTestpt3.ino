#include <TinyGPS.h>
#include <SoftwareSerial.h>
#include <SD.h>
#include <jtXBee.h>


static const int RXPin = 5, TXPin = 6, GPSon = 2, SDpin = 10; //SDpin = 9;
static const int GPSBaud = 9600;
File GPSlog;

//Need to use Old Bootloader to compile
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
SoftwareSerial Xbee(3, 2);
const int t1 = 1000;
const long int t3 = 300000;
unsigned long previousMillis1 = 0;
unsigned long previousMillis2 = 0;
unsigned long previousMillis3 = 0;
int x = 0;


jtXBee celltracker(Xbee);
char phonenumber[12] = "7324849689";
char packetData[50] = "test";

void setup()
{
  //pinMode(GPSon,OUTPUT);
  //pinMode(10, OUTPUT);
  //digitalWrite(GPSon,LOW);
  Xbee.begin(9600);

  GPSINIT(GPSBaud);
  delay(2000);
  Serial.begin(57600);
  delay(10);
  

  Serial.println("INIT_TEXT");
  //String initPacket = "abcd";
  //initPacket.toCharArray(packetData,100);
  celltracker.txSMS(phonenumber, packetData);
  //memset(packetData, 0, 100);
  //delay(1000);
  delay(10);
  
  if (!SD.begin(SDpin)) {
    Serial.println("SD init fail");
    return;
  }else{
    Serial.println("SD init");
  }
  GPSlog = SD.open("log.txt", FILE_WRITE);
  if (GPSlog) {
    Serial.println("log init");
    GPSlog.println("Started");
  } else {
    Serial.println("log init err");
  }
  
  
  
}



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
    
//    if ((millis() - previousMillis2) >= t2 && gps.altitude.meters()<1000 && x<5) {
//      previousMillis2 = millis(); 
//        sendXbeeInfo();
//        x++;        
//    }

    //SMS_TX

    if(((millis() - previousMillis3)>t3) && (gpsInfo.GPSAlt<1000) && (x<50)){
      outputXbee();
      previousMillis3 = millis();
      x++;
      Serial.println("SMS_TX");
    }
    
//    if ((millis() - previousMillis3) >= t3 && gps.altitude()/100.<1000 && x<101) {
//      outputXbee();
//      previousMillis3 = millis();
//      x++;
//   }
}
