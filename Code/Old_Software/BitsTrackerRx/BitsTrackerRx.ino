//UMD Nearspace Iridium Tracking Payload (BITS) {Balloon Iridium Tracking System and killer of link}
//Written by Jonathan Molter and Luke Renegar

#include <IridiumSBD.h>
#include <SD.h>
#include <TinyGPS++.h>

//Serial Ports (sketch requires 4)
#define OutputSerial Serial
#define IridiumSerial Serial1
#define XBeeSerial Serial2
#define GpsSerial Serial3

#define DIAGNOSTICS false // Change this to see diagnostics
#define SLEEP_PIN_NO 6
#define SBD_RX_BUFFER_SIZE 270 // Max size of an SBD message

//Setting default datarates
#define GPS_BAUD 9600
#define SBD_BAUD 19200

// Declare the IridiumSBD object
IridiumSBD modem(IridiumSerial, SLEEP_PIN_NO);

// The TinyGPS++ object
TinyGPSPlus gps;

//Initializing Log Files
File gpsLogFile;
File rxLogFile;
File txLogFile;

const long messageTimeInterval = 300000; // In milliseconds; 300000 is 5 minutes; defines how frequently the program sends messages TODO TODO TODO FIXME FIXME FIXME XXX XXX                                                                        
const long shutdownTimeInterval = 14400000; // In milliseconds; 14400000 is 4 hours; defines after what period of time the program stops sending messages
const String gpsLogName = "00SBDGP.LOG"; // Must be in form XXXXXXXX.log; no more than 8 'X' characters
const String eventLogName = "00SBDEV.LOG"; // Must be in form XXXXXXXX.log; no more than 8 'X' characters
const String rxLogName = "00SBDRX.LOG"; // Must be in form XXXXXXXX.log;  more than 8 'X' characters
const String txLogName = "00SBDTX.LOG"; // Must be in form XXXXXXXX.log; nnoo more than 8 'X' characters

// DO NOT CHANGE THESE ... whoops
const int chipSelect = 10; // Pinn for SPI
unsigned long startTime; // The start time of the program
unsigned long lastMillisOfMessage = 0;
bool sendingMessages = true; // Whether or not the device is sending messages; begins as true TODO
//uint8_t sbd_buf[28];
char sbd_buf[49];//TX BUFFER
uint8_t rxBuf[49];//RX BUFFER
uint8_t sbd_rx_buf[SBD_RX_BUFFER_SIZE];

uint8_t gps_hour;
uint8_t gps_min;
uint8_t gps_sec;
double gps_lat;
double gps_lng;
double gps_alt;
double gps_hdop;
int sbd_csq;

#define ISBD_SUCCESS             0
#define ISBD_ALREADY_AWAKE       1
#define ISBD_SERIAL_FAILURE      2
#define ISBD_PROTOCOL_ERROR      3
#define ISBD_CANCELLED           4
#define ISBD_NO_MODEM_DETECTED   5
#define ISBD_SBDIX_FATAL_ERROR   6
#define ISBD_SENDRECEIVE_TIMEOUT 7
#define ISBD_RX_OVERFLOW         8
#define ISBD_REENTRANT           9
#define ISBD_IS_ASLEEP           10
#define ISBD_NO_SLEEP_PIN        11

#define SBD

void setup()
{
  Serial.begin(9600);
  GpsSerial.begin(GPS_BAUD);
  GPSINIT(); //To get the GPS into Airborne mode
  IridiumSerial.begin(SBD_BAUD);
  XBeeSerial.begin(9600);
  startBlinks();
  
//Open Files
  SD.begin(chipSelect);
  gpsLogFile = SD.open(gpsLogName, FILE_WRITE);
  rxLogFile = SD.open(rxLogName, FILE_WRITE);
  txLogFile = SD.open(txLogName, FILE_WRITE);

  gpsLogFile.println("INIT_GPS_LOG");
  rxLogFile.println("INIT_RX_LOG");
  txLogFile.println("INIT_TX_LOG");
  logprint("INIT_LOG_LOG");

  for (int k = 0; k < SBD_RX_BUFFER_SIZE; k++) //Probably useless TODO
  {
    sbd_rx_buf[k] = 0;
  }
  int err;

#ifdef SBD  // Begin satellite modem operation
  OutputSerial.println("Starting modem...");
  err = modem.begin();
  if (err != ISBD_SUCCESS)
  {
    OutputSerial.print("Begin failed: error ");
    OutputSerial.println(err);
    logprint("Begin failed: error ");
    logprintln(err);
    if ((err == ISBD_NO_MODEM_DETECTED) || (err == 5))
      Serial.println("No modem detected: check wiring.");
    return;
  }
  OutputSerial.println("Modem started");

  #ifdef ISBD_CHECK_FIRMWARE // Example: Print the firmware revision
    char version[12];
    err = modem.getFirmwareVersion(version, sizeof(version));
    if (err != ISBD_SUCCESS)
    {
      OutputSerial.print("FirmwareVersion failed: error ");
      OutputSerial.println(err);
      logprint("FirmwareVersion failed: error ");
      logprintln(err);
      return;
    }
    OutputSerial.print("Firmware Version is ");
    OutputSerial.print(version);
    OutputSerial.println(".");
  #endif

  // From 0-5, 2 or better is preferred (Still works at 0)
  err = modem.getSignalQuality(sbd_csq);
  if (err != ISBD_SUCCESS)
  {
    OutputSerial.print("SignalQuality failed: error ");
    OutputSerial.println(err);
    logprint("SignalQuality failed: error ");
    logprintln(err);
    sbd_csq = 0;
    return;
  }

  OutputSerial.print("On a scale of 0 to 5, signal quality is currently ");
  OutputSerial.print(sbd_csq);

//GPS LOCK INIT
    //OutputSerial.println("ReportLAT"+String(gps_lat,6));
    //logprintln("ReportLAT"+String(gps_lat,6));
    while(gps_lat<=0)
    {
      smartDelay(1000);
      //OutputSerial.println("ReportLAT1 "+String(gps_lat,6));
      //logprintln("ReportLAT1 "+String(gps_lat,6));
    }
    String gpsPacket = String(gps_hour)+":"+String(gps_min)+":"+String(gps_sec)+","+String(gps_lat,5)+","+String(gps_lng,5)+","+String(gps_alt,1);
    txLogFile.println(gpsPacket);
    OutputSerial.println("gpsPacket  " +String(gpsPacket));
// Send the message
  OutputSerial.print("Trying to send the message.  This might take several minutes.\r\n");
  gpsPacket.toCharArray(sbd_buf,49);
  err = modem.sendSBDText(sbd_buf);//Sends an initial packet AFTER gps has locked.
  if (err != ISBD_SUCCESS)
  {
    OutputSerial.println("sendSBDText failed: error "+String(err));
    OutputSerial.println();
    logprint("sendSBDText failed: error ");
    logprintln(err);
    if (err == ISBD_SENDRECEIVE_TIMEOUT)
      OutputSerial.println("Try again with a better view of the sky.");
      logprintln("Try again with a better view of the sky.");
  }else{
    OutputSerial.println("Success, sent = "+String(sbd_buf));
    logprintln("Success, sent = "+String(sbd_buf));
  }
#endif
}

void loop()
{
  ISBDCallback();

  int new_csq = 0;
  int csq_err = modem.getSignalQuality(new_csq);
  if(csq_err == 0) // if executed during an Iridium session, this will yield an ISBD_REENTRANT; keep previous value
  {
    sbd_csq = new_csq;
  }
  else
  {
    Serial.print("CSQ error code = ");
    Serial.println(csq_err);
  }

  if ((sbd_csq > 0 && (millis() - lastMillisOfMessage) > messageTimeInterval) && (millis() < shutdownTimeInterval) && sendingMessages) {

    String logString = "";logString += gps_hour;logString += ":";logString += gps_min;logString += ":";logString += gps_sec;logString += "\t";

    uint32_t gps_time = gps.time.value();
    size_t rx_buf_size = sizeof(sbd_rx_buf); //TODO
    //uint8_t rxBuf[49];//MADE GLOBAL
    size_t bufferSize = sizeof(rxBuf);

    String timeString = String(gps_hour)+":"+String(gps_min)+":"+String(gps_sec);
    String gpsPacket = timeString + ","+String(gps_lat,5)+","+String(gps_lng,5)+","+String(gps_alt,1); //TEST
    //String gpsPacket = String(gps_hour)+":"+String(gps_min)+":"+String(gps_sec)+","+String(gps_lat,5)+","+String(gps_lng,5)+","+String(gps_alt,1);
    
    logprint(gpsPacket);logprintln("Loop Sending");
    OutputSerial.println("Loop Sending");
    txLogFile.println(gpsPacket);
    txLogFile.flush();
    
    gpsPacket.toCharArray(sbd_buf,49);

    uint8_t sbd_err = modem.sendReceiveSBDText(sbd_buf,rxBuf,bufferSize); //SEND/RECEIVE
    
    // update time //This can be shortened TODO
    //REPLACED WITH TIME STRING, HOLD TILL TEST
    //logString = "";logString += gps_hour;logString += ":";logString += gps_min;logString += ":";logString += gps_sec;logString += "\t";

    logprint("SBD send receive completed with return code: ");
    logprintln(sbd_err);
    OutputSerial.println("Send Error:  " +String(sbd_err));

    rxLogFile.print(timeString);
    rxLogFile.print("\t");
    rxLogFile.print(rx_buf_size);
    rxLogFile.print("\t");

//Uplink
    uplink();
 
    for (int k = 0; k < bufferSize; k++)//Prints RX characters to SD file
    {
      rxLogFile.print(rxBuf[k]);
      rxBuf[k] = 0;
      delay(1);
    }
    rxLogFile.println();
    rxLogFile.flush();

    lastMillisOfMessage = millis();
  }
  smartDelay(250); //Not the best way to do this, but can stay for now....Actually crap, but FIIINE
  //Ok, I really hate this method but I can't make it go away in time for flight
}

bool ISBDCallback()
{

  if (gps.location.isUpdated())
  {
    gps_hour = gps.time.hour();
    gps_min = gps.time.minute();
    gps_sec = gps.time.second();
    gps_lat = gps.location.lat();
    gps_lng = gps.location.lng();
    gps_alt = gps.altitude.meters();
    gps_hdop = gps.hdop.value();

    String logString = "";
    logString += gps_hour;
    logString += ":";
    logString += gps_min;
    logString += ":";
    logString += gps_sec;
    Serial.print(logString);
    Serial.print(",");
    Serial.print(gps_lat, 6);
    Serial.print(",");
    Serial.print(gps_lng, 6);
    Serial.print(",");
    Serial.print(gps_alt, 2);
    Serial.print(",");
    Serial.print(gps_hdop);
    Serial.print(",");
    Serial.print(sbd_csq);
    Serial.println();
    if (gpsLogFile) {
      gpsLogFile.print(logString);
      gpsLogFile.print(",");
      gpsLogFile.print(gps_lat, 6);
      gpsLogFile.print(",");
      gpsLogFile.print(gps_lng, 6);
      gpsLogFile.print(",");
      gpsLogFile.print(gps_alt, 2);
      gpsLogFile.print(",");
      gpsLogFile.print(gps_hdop);
      gpsLogFile.print(",");
      gpsLogFile.print(sbd_csq);
      gpsLogFile.println();
      gpsLogFile.flush();
    }
  }
  else
  {
    smartDelay(50);
    //Serial.println("No Update.");
  }
  return true; // ISBD operation should continue
}

static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    while (GpsSerial.available())
    {
      gps.encode(GpsSerial.read());
    }
  } while (millis() - start < ms);
}



static void logprint(String s)
{
  File dataFile = SD.open(eventLogName, FILE_WRITE);
  dataFile.print(s);
  dataFile.close();
}

static void logprint(int i)
{
  File dataFile = SD.open(eventLogName, FILE_WRITE);
  dataFile.print(i);
  dataFile.close();
}


static void logprint(double d)
{
  File dataFile = SD.open(eventLogName, FILE_WRITE);
  dataFile.print(d);
  dataFile.close();
}

static void logprintln(String s)
{
  File dataFile = SD.open(eventLogName, FILE_WRITE);
  dataFile.println(s);
  dataFile.close();
}

static void logprintln(int i)
{
  File dataFile = SD.open(eventLogName, FILE_WRITE);
  dataFile.println(i);
  dataFile.close();
}

static void logprintln(double d)
{
  File dataFile = SD.open(eventLogName, FILE_WRITE);
  dataFile.println(d);
  dataFile.close();
}

#if DIAGNOSTICS
void ISBDConsoleCallback(IridiumSBD *device, char c)
{
  OutputSerial.write(c);
}

void ISBDDiagsCallback(IridiumSBD *device, char c)
{
  OutputSerial.write(c);
}
#endif
