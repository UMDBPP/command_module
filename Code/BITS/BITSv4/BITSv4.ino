// Balloon Iridium Telemetry Systems (BITS)
// Developed for the UMD Balloon Payload Program
//
// Provides capabilities for high altitude balloon position, altitude, telemetry values, and signal relay to other payloads
// Implements a Rockblock 9603 Iridium Carrier board, 900HP XBee, and Teensy3.2.
// BITSv4 board designed and assembled in-house.
// J. Molter

// Included Libraries
#include <XBee.h>       //If using 900HP's this must be the custom cpp (or really any post gen2 XBees)
#include <IridiumSBD.h>
#include <SD.h>
#include <TinyGPS.h>

// Serial Ports
#define OutputSerial  Serial  //USB debug
#define IridiumSerial Serial1 //Iridium 9603
#define XBeeSerial    Serial2 //XBee
#define gpsserial     Serial3 //GPS

// Pin Assignments
#define SLEEP_PIN_NO  5 // Iridium Sleep Pin
#define RING_PIN_NO   2 // Iridium Ring Alert Pin
#define NET_AV_PIN    3
#define chipSelect    6 // C.S. Pin for SPI (6 on new board)
#define greenLED      4 // Green Status LED
#define bat_cell     22 // Battery Voltage Analog Pin

// Configuration Flags
#define DIAGNOSTICS     false // Diagnostics enabled (?)
#define XBEE_DEBUG      true
#define USEGPS          true  // GPS enabled (?)
#define sendingMessages true  // Iridium transmit enabled (?)

// Default Baud Rates
#define COM_BAUD  115200
#define SBD_BAUD  19200
#define XBEE_BAUD 9600
#define GPS_BAUD  115200

// XBee Address Assignments
const uint32_t BitsSL   = 0x417B4A3B; //BITS (white)Specific to the XBee on Bits (the Serial Low address value)
const uint32_t GroundSL = 0x417B4A36; //GroundStations (u.fl)
const uint32_t BlueSL   = 0x417B4A3A; //Blue (Blue Tape)
const uint32_t WireSL   = 0x419091AC; //Wire (wire antenna)
const uint32_t UniSH    = 0x0013A200; //Common across any and all XBees

// XBee Object Creation
XBee xbee = XBee();
XBeeResponse response = XBeeResponse();
ZBTxStatusResponse txStatus = ZBTxStatusResponse();
ZBRxResponse rx = ZBRxResponse();
ModemStatusResponse msr = ModemStatusResponse();

// XBee Data Buffers
const int xbeeRecBufSize  = 49;      // Received XBee Buffer Size
const int xbeeSendBufSize = 34;      // Send XBee Buffer Size, must be ~15 bytes less than the Recieve Buffer
uint8_t xbeeRecBuf[xbeeRecBufSize];
uint8_t xbeeSendBuf[xbeeSendBufSize];

// GPS Data Structure
struct GPSdata{
  float GPSLat=-1;
  float GPSLon=-1;
  unsigned long GPSTime=-1;
  long GPSAlt=-1;
  int GPSSats=-1;
};

// Create GPS object and an instance of structure
TinyGPS gps;
GPSdata gpsInfo;

// Timing Intervals
const long signalCheckInterval    = 15000; // Time between Iridium Signal Quality (CSQ) checks [ms]
unsigned long messageTimeInterval = 60000; // Time between Iridium Packets [ms]                                                                     
const long gpsLogInterval         = 1000;  // Time between GPS packets [ms]

// Program Timers
unsigned long lastMillisOfMessage = 0;
unsigned long lastSignalCheck     = 0;
unsigned long lastLog             = 0;

// Create Log File Objects
File gpsLogFile;
File rxLogFile;
File txLogFile;

// Log File Names
const char gpsLogName[]   = "GPS.LOG";  //1 sec position/altitude updates
const char eventLogName[] = "EVENT.LOG";//Events Iridium/XBee (Special one thanks to Luke)
const char rxLogName[]    = "RX.LOG";   //Iridium Uplinks   (toBalloon)
const char txLogName[]    = "TX.LOG";   //Iridium Downlinks (toGround)

// Iridium Global Buffers
#define maxPacketSize 128       // Total Downlink Packet Size (Includes dowlinkMessageSize)
#define downlinkMessageSize 100 // Additional Downlink Message Size
#define RX_BUF_LENGTH 49		    // Uplink Buffer Size

// Create Iridium Buffers
char downlinkMessage2[downlinkMessageSize]; //Downlink Buffer
uint8_t rxBuf[RX_BUF_LENGTH]; 			        //Uplink Buffer
bool downlinkData;

int sbd_csq; // Signal Qual

// Status value locking out a drop command, likely unecessary, TODO
int arm_status = 42; // Armed when = 42

// Declare the IridiumSBD object
IridiumSBD modem(IridiumSerial);

// Possible Iridium Error Codes
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

// Begin Setup
void setup()
{
  delay(1000);
  
  // Initialize Serial Ports
  OutputSerial.begin(COM_BAUD);
  XBeeSerial.begin(XBEE_BAUD);
  IridiumSerial.begin(SBD_BAUD);
  
  delay(500);
  OutputSerial.println("BITSv4");

  // Run GPS Initialization Method
  gps_init();

  // Set XBee object to XBee Serial Port
  xbee.setSerial(XBeeSerial);

  // Run Startblinks
  startBlinks();

  // Open SD card
  SD.begin(chipSelect);

  // Open SD card files, write init msg, and flush the file
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

  OutputSerial.println("MadeLogs");

  // XBee Init Message to ground stations
  // TODO: Verify Removal Via Test
  // String("Init").getBytes(xbeeSendBuf,xbeeSendBufSize);   //Convert "Init" 2 bytes, dump into Message buffer
  // xbeeSend(GroundSL,xbeeSendBuf);                         //(Target,Message)
  xbeeSend(GroundSL,"Program Init, Made Logs");                            //(Target,Message)
  
  int err;

// Begin satellite modem operation
#ifdef SBD
  
  err = modem.begin();
  
  if (err != ISBD_SUCCESS)
  {
    OutputSerial.print("Begin failed: error ");
    OutputSerial.println(err);
    logprint("Begin failed: error ");
    logprintln(err);
  }

  // Modem Settings
  modem.useMSSTMWorkaround(false); // TODO, determine the impact of this line
  modem.adjustSendReceiveTimeout(120);
  
  OutputSerial.println("Modem started");

  // TODO: Verify Removal Via Test
  // String("ModemStarted").getBytes(xbeeSendBuf,xbeeSendBufSize);
  // xbeeSend(GroundSL,xbeeSendBuf);
  xbeeSend(GroundSL,"ModemStarted");

  // TODO: Verify Removal Via Test
  /*
  #ifdef ISBD_CHECK_FIRMWARE
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
  */
  
  // Check Signal Quality of Modem. From 0-5, 2 or better is preferred (Still works at 0)
  err = modem.getSignalQuality(sbd_csq);
  if (err != ISBD_SUCCESS)
  {
    OutputSerial.print("SignalQuality failed: error ");
    OutputSerial.println(err);
    logprint("SignalQuality failed: error ");
    logprintln(err);
    sbd_csq = -1;
  }

  OutputSerial.print("From 0 to 5, signal qual is: ");
  OutputSerial.println(sbd_csq);

  // TODO: Remove Capital String occurance, compress to single line
  String("ModemCSQ:"+String(sbd_csq)).getBytes(xbeeSendBuf,xbeeSendBufSize);
  xbeeSend(GroundSL,xbeeSendBuf);

#endif
// End SBD ifdef

  // Force a hold on setup until GPS lock is acquired
  // Stable lock is achieved when altitude is within acceptable bounds
  
  // TODO: Verify Removal on Test
  //String("GPS_Acquisition_Phase").getBytes(xbeeSendBuf,xbeeSendBufSize);
  //xbeeSend(GroundSL,xbeeSendBuf);
  xbeeSend(GroundSL,"GPS_Acquisition_Phase");

  // TODO: Move code to a gpsHold method
  // TODO: Remove instance of String
  String gpsPacket;
  
  if (USEGPS) {
    OutputSerial.println("TryingGPS");

    // While GPSAlt is outside of an acceptable range, continually read until it becomes available
    // TODO: Implement some method of status during this process to indicate lock is imrpoving
    while((gpsInfo.GPSAlt<=0)||(gpsInfo.GPSAlt>100000))
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

    
  } else {
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
    char gpsLogPacket2[50];
    
    char exactTime[9];//A somewhat convaluted way of adding : into the integer timestamp... //TEST
    snprintf(exactTime,7,"%d",gpsInfo.GPSTime);
    char hour[3];char min[3];char sec[3];
    strncpy(hour, &exactTime[0], 2);hour[2] = '\0';
    strncpy(min, &exactTime[2], 2);min[2] = '\0';
    strncpy(sec, &exactTime[4], 2);sec[2] = '\0';
    snprintf(exactTime,9,"%s:%s:%s",hour,min,sec);
    
    //snprintf(gpsLogPacket2,35,"%s,%4.4f,%4.4f,%u",exactTime,gpsInfo.GPSLat,gpsInfo.GPSLon,gpsInfo.GPSAlt); //Fix Timestep
    snprintf(gpsLogPacket2,45,"%06d,%4.5f,%4.5f,%u",gpsInfo.GPSTime,gpsInfo.GPSLat,gpsInfo.GPSLon,gpsInfo.GPSAlt); //Build the packet
    gpsLogFile.println(gpsLogPacket2);
    
    gpsLogFile.flush(); //WIP
    lastLog=millis();
  }
}

//* This version actually hopes to leverage the power of proper link budget calculations to obtain XBee to ground comms
//  without the need for Iridium
