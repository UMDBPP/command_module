//UMD Nearspace Iridium Tracking Payload (BITS) {Balloon Iridium Tracking (or is it Telemetry?) System and killer of link)
//Written by Jonathan Molter and Luke Renegar
//Uses an Iridium 9603 SBD Modem for effective unlimited range, without the need for our own RF blackmagics

#include <XBee.h> //If using 900HP's this must be the custom cpp (or really any post gen2 XBees)
#include <IridiumSBD.h>
#include <SD.h>
#include <TinyGPS.h>

//Serial Ports (sketch requires 4 {Wellll technically 3 + USB})
#define OutputSerial Serial
#define IridiumSerial Serial1
#define XBeeSerial Serial2
#define GpsSerial Serial3

#define DIAGNOSTICS false // Change this to see diagnostics
const bool USEGPS = true;
#define SLEEP_PIN_NO 6
#define SBD_RX_BUFFER_SIZE 270 // Max size of an SBD message

//Setting default datarates
#define GPS_BAUD 9600
#define SBD_BAUD 19200

// Declare the IridiumSBD object
IridiumSBD modem(IridiumSerial, SLEEP_PIN_NO);

//Xbee Stuff
const uint32_t TardisSL = 0x417B4A3B;
const uint32_t MarsSL = 0x417B4A3B;
const uint32_t GroundSL = 0x417B4A3B;
const uint32_t UniSH = 0x0013A200;//Dont Touch this one
XBee xbee = XBee();
XBeeResponse response = XBeeResponse();
XBeeAddress64 TardisAddress = XBeeAddress64(UniSH, TardisSL);//SH,SL
XBeeAddress64 MarsAddress = XBeeAddress64(UniSH, MarsSL);
XBeeAddress64 GroundAddress = XBeeAddress64(UniSH, GroundSL);
ZBTxStatusResponse txStatus = ZBTxStatusResponse();
ZBRxResponse rx = ZBRxResponse();
uint8_t xbeeBuf[250];//Could be bigger

struct GPSdata{
  float GPSLat=0;
  float GPSLon=0;
  unsigned long GPSTime=0;
  unsigned int GPSAlt=0;
  int GPSSats=-1;
};
TinyGPS gps;
GPSdata gpsInfo;

//Initializing Log Files
File gpsLogFile;
File rxLogFile;
File txLogFile;

const long signalCheckInterval = 15000;
long messageTimeInterval = 60000; // In milliseconds; 300000 is 5 minutes; defines how frequently the program sends messages, now changeable                                                                      
const long shutdownTimeInterval = 14400000; // In milliseconds; 14400000 is 4 hours; defines after what period of time the program stops sending messages

// LogFiles, Must be in form XXXXXXXX.log; no more than 8 'X' characters
const String gpsLogName =   "GPS.LOG";  //1 sec position/altitude updates
const String eventLogName = "EVENT.LOG";//Events Iridium/XBee
const String rxLogName =    "RX.LOG";   //Iridium Uplinks   (toBalloon)
const String txLogName =    "TX.LOG";   //Iridium Downlinks (toGround)

// DO NOT CHANGE THESE ... whoops (sorry Luke)
const int chipSelect = 10; // Pin for SPI

unsigned long startTime; // The start time of the program
unsigned long lastMillisOfMessage = 0;
unsigned long lastSignalCheck = 0;
unsigned long lastLog = 0;

bool sendingMessages = true; // Whether or not the device is sending messages; begins as true TODO

char sbd_buf[49];//TX BUFFER
uint8_t rxBuf[49];//RX BUFFER
uint8_t sbd_rx_buf[SBD_RX_BUFFER_SIZE];
String downlinkMessage;
bool downlinkData;

int arm_status;//armed when = 42

uint8_t gps_hour;
uint8_t gps_min;
uint8_t gps_sec;
double gps_lat;
double gps_lng;
unsigned int gps_alt;
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
  GpsSerial.begin(9600);
  GPSINIT(); //To get the GPS into Airborne mode WIP TODO
  IridiumSerial.begin(SBD_BAUD);
  XBeeSerial.begin(9600);
  xbee.setSerial(XBeeSerial);
  startBlinks();
  
//Open Files
  SD.begin(chipSelect);
  gpsLogFile = SD.open(gpsLogName, FILE_WRITE);
  rxLogFile = SD.open(rxLogName, FILE_WRITE);
  txLogFile = SD.open(txLogName, FILE_WRITE);
  delay(10);
  gpsLogFile.println("INIT_GPS_LOG");
  rxLogFile.println("INIT_RX_LOG");
  txLogFile.println("INIT_TX_LOG");
  gpsLogFile.flush();
  rxLogFile.flush();
  txLogFile.flush();
  
  logprintln("INIT_LOG_LOG");

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
    OutputSerial.println(version);
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

#endif

String gpsPacket;
char improvedGPSPacket[49];

if(USEGPS){
  OutputSerial.println("TryingGPS");
//GPS LOCK INIT
    while((gpsInfo.GPSAlt<=0)||(gpsInfo.GPSAlt>100000))
    {
      delay(1);
      //output();
      while (GpsSerial.available()){
        if (gps.encode(GpsSerial.read())){
          gpsInfo = getGPS();
          break;
        }
      }    
    }
    gpsPacket = String(gpsInfo.GPSTime)+","+String(gpsInfo.GPSLat,4)+","+String(gpsInfo.GPSLon,4)+","+String(gpsInfo.GPSAlt);
//WIP WORK    
    char Lon[8];
    char Lat[8];
    dtostrf(gpsInfo.GPSLon,8,4,Lon);
    dtostrf(gpsInfo.GPSLat,8,4,Lat);
    strcat(improvedGPSPacket,(char*)gpsInfo.GPSTime);
    strcat(improvedGPSPacket,Lon);
    strcat(improvedGPSPacket,Lat);
    strcat(improvedGPSPacket,(char*)gpsInfo.GPSAlt);
    
    
    txLogFile.println(improvedGPSPacket);
    OutputSerial.println("gpsPacket  " +String(improvedGPSPacket));
}else{
  OutputSerial.println("Not Using GPS");
  gpsPacket = "test";
}

#ifdef SBD
// Send the message
  OutputSerial.print("Trying to send the message.  This might take several minutes.\r\n");
  char init_message[49];
  gpsPacket.toCharArray(init_message,49);
  err = modem.sendSBDText(init_message);//Sends an initial packet AFTER gps has locked.
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
    OutputSerial.println("Success, sent = "+String(init_message));
    logprintln("Success, sent = "+String(init_message));
  }
#endif
}

void loop()
{
//Parse the GPS
  while (GpsSerial.available()){
    if (gps.encode(GpsSerial.read())){
      gpsInfo = getGPS();
      break;
    }
  } 
 
//Check the Xbee
  xbeeRead(xbee);
  if(millis()-lastLog>1000){
    String gpsLogPacket;
    gpsLogPacket = String(gpsInfo.GPSTime)+","+String(gpsInfo.GPSLat,4)+","+String(gpsInfo.GPSLon,4)+","+String(gpsInfo.GPSAlt);
    gpsLogFile.println(gpsLogPacket);
    gpsLogFile.flush();
    lastLog=millis();
  }
//Check Signal Quality
  if (((millis() - lastSignalCheck) > signalCheckInterval) && (millis() < shutdownTimeInterval)) {
    int new_csq = 0;
    int csq_err = modem.getSignalQuality(new_csq);
    if(csq_err == 0) // if executed during an Iridium session, this will yield an ISBD_REENTRANT; keep previous value
    {
      sbd_csq = new_csq;
    }
    else if(csq_err == ISBD_REENTRANT){
      //KeepCalm, carry on  
    }else{
      logprint("CSQ error code = ");
      logprintln(csq_err);
    }
    lastSignalCheck = millis();
  }

//Transmit Via Iridium
  if ((sbd_csq > 0 && (millis() - lastMillisOfMessage) > messageTimeInterval) && (millis() < shutdownTimeInterval) && sendingMessages) {

    String logString = "";logString += String(gpsInfo.GPSTime);logString += "\t";

    uint32_t gps_time = gpsInfo.GPSTime;
    //uint32_t gps_time = gps.time.value();
    size_t rx_buf_size = sizeof(sbd_rx_buf); //TODO
    size_t bufferSize = sizeof(rxBuf);

    //String timeString = String(gps_hour)+":"+String(gps_min)+":"+String(gps_sec);
    String Packet = String(gpsInfo.GPSTime) + ","+String(gpsInfo.GPSLat,4)+","+String(gpsInfo.GPSLon,4)+","+String(gpsInfo.GPSAlt);
    if(downlinkData){
      Packet = Packet + "," + downlinkMessage;
      downlinkData = false;
      downlinkMessage = "";
    }    
    logprint(Packet);logprintln("Loop Sending");
    OutputSerial.println("Loop Sending");
    txLogFile.println(Packet);
    txLogFile.flush();
    
    Packet.toCharArray(sbd_buf,49);
    //uint8_t sbd_err = modem.sendReceiveSBDText(sbd_buf,rxBuf,bufferSize); //SEND/RECEIVE
    
    uint8_t sbd_err = modem.sendReceiveSBDBinary((uint8_t*)atoi(sbd_buf),bufferSize,rxBuf,bufferSize); //TESTING SEND/RECEIVE

    logprint("SBD send receive completed with return code: ");
    logprintln(sbd_err);
    OutputSerial.println("Send Error:  " +String(sbd_err));

    rxLogFile.print(String(gpsInfo.GPSTime));
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
  //smartDelay(250); //Not the best way to do this, but can stay for now....Actually crap, but FIIINE <WIPGPS>
  //Ok, I really hate this method but I can't make it go away in time for flight
  //edit: killed it
}//End of Loop



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

static void logprintln32(uint32_t d)
{
  File dataFile = SD.open(eventLogName, FILE_WRITE);
  dataFile.println(d,HEX);
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
