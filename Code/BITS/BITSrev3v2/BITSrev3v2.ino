 //UMD Nearspace Iridium Tracking Payload (BITS) {Balloon Iridium Tracking System)
//Written by Jonathan Molter and Luke Renegar
//Uses an Iridium 9603 SBD Modem for effective unlimited range, without the need for our own RF blackmagics*
//This software is specifically written with the bits3 board revisision in mind
// ACTIVE REV, 7/25/20

#include <XBee.h> //If using 900HP's this must be the custom cpp (or really any post gen2 XBees)
#include <IridiumSBD.h>
#include <SD.h>
#include <TinyGPS.h>

//Serial Ports (sketch requires 4 {Wellll technically 3 + DEBUG})
#define OutputSerial Serial     //USB debug
#define IridiumSerial Serial1   //Iridium 9603
#define XBeeSerial Serial2      //XBee
#define gpsserial Serial3       //GPS

//Pins
#define SLEEP_PIN_NO 5    // Iridium Sleep Pin
#define RING_PIN_NO 2    // Iridium Ring Alert Pin
#define NET_AV_PIN 3
#define chipSelect 6      // Pin for SPI (6 on new board)
#define greenLED 4        // Green Status LED
#define bat_cell1 22

// Config Settings
#define DIAGNOSTICS false // Change this to see diagnostics
#define XBEE_DEBUG true
const bool USEGPS = true; // Should be true 99% of the time
const bool sendingMessages = true; // Whether or not the device is sending messages; begins as true TODO

//Setting default datarates
#define GPS_BAUD 115200
#define SBD_BAUD 19200

//XBee Addresses
const uint32_t BitsSL = 0x417B4A3B;   //BITS (white)Specific to the XBee on Bits (the Serial Low address value)
const uint32_t GroundSL = 0x417B4A36; //GroundStations (u.fl)
const uint32_t BlueSL = 0x417B4A3A;   //Blue (Blue Tape)
const uint32_t WireSL = 0x419091AC;   //Wire (wire antenna)
const uint32_t UniSH = 0x0013A200;    //Common across any and all XBees

//XBee object / global variables
XBee xbee = XBee();
XBeeResponse response = XBeeResponse();
ZBTxStatusResponse txStatus = ZBTxStatusResponse();
ZBRxResponse rx = ZBRxResponse();
ModemStatusResponse msr = ModemStatusResponse();
const int xbeeRecBufSize = 49; //Rec must be ~15bytes larger than send
const int xbeeSendBufSize = 34;
uint8_t xbeeRecBuf[xbeeRecBufSize];
uint8_t xbeeSendBuf[xbeeSendBufSize];

//GPS data struct and object
struct GPSdata{
  float GPSLat=-1;
  float GPSLon=-1;
  unsigned long GPSTime=-1;
  long GPSAlt=-1;
  int GPSSats=-1;
};
TinyGPS gps;
GPSdata gpsInfo;

//Interval definitions
const long signalCheckInterval = 15000;
unsigned long messageTimeInterval = 60000; // In milliseconds; 300000 is 5 minutes; defines how frequently the program sends messages, now changeable                                                                      

// Hard shutdowns are a bad idea, trace NS-88
//const long shutdownTimeInterval = 14400000; // In milliseconds; 14400000 is 4 hours; defines after what period of time the program stops sending messages
const long gpsLogInterval = 1000;
const long gpsLandedInterval = 1000;

//Initializing Log Files; Must be in form XXXXXXXX.log; no more than 8 'X' characters
File gpsLogFile;
File rxLogFile;
File txLogFile;

const String gpsLogName =   "GPS.LOG";  //1 sec position/altitude updates
const String eventLogName = "EVENT.LOG";//Events Iridium/XBee (Special one thanks to Luke)
const String rxLogName =    "RX.LOG";   //Iridium Uplinks   (toBalloon)
const String txLogName =    "TX.LOG";   //Iridium Downlinks (toGround)



// Program Timing
unsigned long startTime;
unsigned long lastMillisOfMessage = 0;
unsigned long lastSignalCheck = 0;
unsigned long lastLog = 0;

// Iridium Global Buffers
//#define SBD_RX_BUFFER_SIZE 100 // Max size of an SBD message TODO(1) DELETE
#define maxPacketSize 128		//Standard(GPS/TIME, etc.) + Message Data
#define downlinkMessageSize 100 //Message Data
#define RX_BUF_LENGTH 49		//Uplink Buffer Size

uint8_t rxBuf[RX_BUF_LENGTH]; 			   //Uplink Buffer
//uint8_t sbd_rx_buf[SBD_RX_BUFFER_SIZE];  //TODO(1) DELETE ?(what is this even doing...)
char downlinkMessage2[downlinkMessageSize];//Downlink Buffer
bool downlinkData;

int sbd_csq; //Signal Qual

// The best global
int arm_status = 42;//armed when = 42

// Declare the IridiumSBD object
IridiumSBD modem(IridiumSerial);

//Possible Iridium Error Codes
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
  Serial.begin(115200);
  Serial2.begin(115200);
  IridiumSerial.begin(SBD_BAUD);
  
  delay(1000);
  Serial.println("BITSrev3v2");
  
  gps_init(); //Setup the GPS
  
  
  xbee.setSerial(Serial2);
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
  Serial.println("MadeLogs");
  logprintln("INIT_LOG_LOG");

  //memset(sbd_rx_buf, 0, SBD_RX_BUFFER_SIZE); //TODO(1) DELETE ?

  //XBee Init Message to ground stations
  String("Init").getBytes(xbeeSendBuf,xbeeSendBufSize);   //Convert "Init" 2 bytes, dump into Message buffer
  xbeeSend(GroundSL,xbeeSendBuf);                         //(Target,Message)
  
  int err;

#ifdef SBD  // Begin satellite modem operation
  OutputSerial.println("Starting modem...");
  err = modem.begin();
  modem.useMSSTMWorkaround(false);//SUPER NF TESTTHIS TODO(2)
  modem.adjustSendReceiveTimeout(120);
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
  String("ModemStarted").getBytes(xbeeSendBuf,xbeeSendBufSize);
  xbeeSend(GroundSL,xbeeSendBuf);

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
    sbd_csq = -1;
    return;
  }

  OutputSerial.print("From 0 to 5, signal qual is: ");
  OutputSerial.println(sbd_csq);

  String("ModemCSQ:"+String(sbd_csq)).getBytes(xbeeSendBuf,xbeeSendBufSize);
  xbeeSend(GroundSL,xbeeSendBuf);
#endif

  String("GPS_Acquisition_Phase").getBytes(xbeeSendBuf,xbeeSendBufSize);
  xbeeSend(GroundSL,xbeeSendBuf);

// HOLD UNTIL LOCK
String gpsPacket;
if(USEGPS){
  OutputSerial.println("TryingGPS");

  //GPS LOCK INIT
    while((gpsInfo.GPSAlt<=0)||(gpsInfo.GPSAlt>100000)) //Outside of a reasonable range
    {
      //delay(500);
      //output();
      while (gpsserial.available()){
          if (gps.encode(gpsserial.read())){
          gpsInfo = getGPS();
        break;
        }
      }
    }
    
    gpsPacket = String(gpsInfo.GPSTime)+","+String(gpsInfo.GPSLat,4)+","+String(gpsInfo.GPSLon,4)+","+String(gpsInfo.GPSAlt);
    txLogFile.println(gpsPacket);
    logprintln("GotLock");
    gpsLockBlink();
    OutputSerial.println("gpsPacket  " +String(gpsPacket));

    //Got Lock Notification
    String("gotLock").getBytes(xbeeSendBuf,xbeeSendBufSize);
    xbeeSend(GroundSL,xbeeSendBuf); 

    
}else{
  OutputSerial.println("Not Using GPS");
  gpsPacket = "test";
}


//Honestly consider canning this whole section, really not needed considering loop does the same thing
#ifdef SBD
// Send the message
  char sbd_buf[49];//TX BUFFER
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

        String("EnteringLoop").getBytes(xbeeSendBuf,xbeeSendBufSize);
        xbeeSend(GroundSL,xbeeSendBuf);
}

void loop()
{  
  ISBDCallback(); //During the Iridium retry protocol anything that still needs to happen needs to go in here

  xbeeRead();
  LogPacket();
  
//Check Signal Quality
  //if (((millis() - lastSignalCheck) > signalCheckInterval) && (millis() < shutdownTimeInterval)) {
  if (((millis() - lastSignalCheck) > signalCheckInterval)) {
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
  // If not currently transmitting, and at time, and transmitting messages ON, transmit
  //if ((sbd_csq > 0 && (millis() - lastMillisOfMessage) > messageTimeInterval) && (millis() < shutdownTimeInterval) && sendingMessages) {
  if ((sbd_csq > 0 && (millis() - lastMillisOfMessage) > messageTimeInterval) && sendingMessages) {

    size_t rx_buf_size = RX_BUF_LENGTH; //RECENT CHANGE via TODO(1)

  //Assesmble the packet to be sent to the ground
    char Packet2[maxPacketSize];
    snprintf(Packet2,maxPacketSize,"%06d,%4.4f,%4.4f,%u",gpsInfo.GPSTime,gpsInfo.GPSLat,gpsInfo.GPSLon,gpsInfo.GPSAlt); //Build the packet

  //If there is stuff to downlink than do
    if(downlinkData){ 
      logprint("DownAttempt: ");
      logprintln(downlinkMessage2);
      strncat(Packet2,downlinkMessage2,(maxPacketSize - strlen(Packet2) - 1));  //Add data from downlinkMessage2 into Packet2, check for size
      downlinkData = false;
      memset(downlinkMessage2, 0, downlinkMessageSize);                         //Clear downlink message
    }    
    
    logprint(Packet2);logprintln("Loop Sending");
    OutputSerial.println("Loop Sending");
    txLogFile.println(Packet2);
    txLogFile.flush();

  //SEND/RECEIVE DATA, return status
    uint8_t sbd_err = modem.sendReceiveSBDText(Packet2,rxBuf,rx_buf_size);  //Message, RecieveBuffer, SizeOfBuffer

    logprint("SBD send receive completed with return code: ");
    logprintln(sbd_err); // 0 is good
    OutputSerial.println("Send Error:  " +String(sbd_err));

    rxLogFile.print(String(gpsInfo.GPSTime)); //Log time of RX
    rxLogFile.print("\t");
    
  //Process Received Messages
    uplink(); //Run uplink message handling (bitsIncoming)
 
    for (int k = 0; k < rx_buf_size; k++)//Prints RX characters to SD file
    {
      rxLogFile.write(rxBuf[k]); //Trying write instead of print, shoud fix garbage data
      rxBuf[k] = 0;
      delay(1);
    }
    rxLogFile.println();
    rxLogFile.flush();

    lastMillisOfMessage = millis();
  }
}//End of Loop


bool ISBDCallback() //Functions that occurs during packet transmit sequence delay
{
  //Parse the GPS without delay
  while (gpsserial.available()){
    if (gps.encode(gpsserial.read())){
      gpsInfo = getGPS();
      break;
    }
  }

  //Check the Xbee
  xbeeRead();
  
  LogPacket();
  
  return true;
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

void LogPacket(){
  if(millis()-lastLog>gpsLogInterval){
    char gpsLogPacket2[35];
    
    char exactTime[9];//A somewhat convaluted way of adding : into the integer timestamp... //TEST
    snprintf(exactTime,7,"%d",gpsInfo.GPSTime);
    char hour[3];char min[3];char sec[3];
    strncpy(hour, &exactTime[0], 2);hour[2] = '\0';
    strncpy(min, &exactTime[2], 2);min[2] = '\0';
    strncpy(sec, &exactTime[4], 2);sec[2] = '\0';
    snprintf(exactTime,9,"%s:%s:%s",hour,min,sec);
    
    snprintf(gpsLogPacket2,35,"%s,%4.4f,%4.4f,%u",exactTime,gpsInfo.GPSLat,gpsInfo.GPSLon,gpsInfo.GPSAlt); //Fix Timestep
    gpsLogFile.println(gpsLogPacket2);
    
    gpsLogFile.flush(); //WIP
    lastLog=millis();
  }
}

//* This version actually hopes to leverage the power of proper link budget calculations to obtain XBee to ground comms
//  without the need for Iridium
