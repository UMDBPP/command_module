// Balloon Iridium Telemetry Systems (BITS)
// Developed for the UMD Balloon Payload Program
//
// Provides capabilities for high altitude balloon position, altitude, telemetry values, and signal relay to other payloads
// Implements a Rockblock 9603 Iridium Carrier board, 900HP XBee, and Teensy3.2.
// BITSv4 board designed and assembled in-house.
//
// 2019 - 2022 : J. Molter

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
#define chipSelect    6 // C.S. Pin for SPI
#define greenLED      4 // Green Status LED
#define batteryPin   16 // Battery Voltage Analog Pin

// Configuration Flags (true/false)
#define DIAGNOSTICS     false // Diagnostics enabled
#define USEGPS          true  // GPS enabled
#define sendingMessages true  // Iridium transmit enabled
#define SBD                   // SBD must be defined for tranmit to occur

// Default Baud Rates
#define COM_BAUD  115200
#define SBD_BAUD  19200
#define XBEE_BAUD 9600
#define GPS_BAUD  115200

// XBee Address Assignments
// The XBees use 64 bit addresses, where the first 32 bit portion (Serial High) is common across XBees of the same series
// XBee addresses can be checked by connecting the XBee to a computer and using the XCTU application
//
const uint32_t BitsSL   = 0x417B4A3B; // BITS (white) Specific to the XBee on Bits (the Serial Low address value)
const uint32_t GroundSL = 0x417B4A36; // GroundStation XBee (u.fl)
const uint32_t BlueSL   = 0x417B4A3A; // Blue (Blue Tape)
const uint32_t WireSL   = 0x419091AC; // Wire (wire antenna)
const uint32_t UniSH    = 0x0013A200; // Common across any and all XBees

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

// Timing Intervals [ms]
const long signalCheckInterval    = 15000; // Time between Iridium Signal Quality (CSQ) checks
unsigned long messageTimeInterval = 60000; // Time between Iridium Packets                                                      
const long gpsLogInterval         = 1000;  // Time between GPS packets, for more than 1 Hz, GPS settings must be modified

// Initialize Program Timers
unsigned long lastMillisOfMessage = 0;
unsigned long lastSignalCheck     = 0;
unsigned long lastLog             = 0;

// Create Log File Objects
File gpsLogFile;
File rxLogFile;
File txLogFile;

// Log File Names
const char gpsLogName[]   = "GPS.LOG";   // GPS Telemetry logged at a rate of gpsLogInterval
const char eventLogName[] = "EVENT.LOG"; // Events Iridium/XBee (Special one thanks to Luke)
const char rxLogName[]    = "RX.LOG";    // Iridium Uplinks   (toBalloon)
const char txLogName[]    = "TX.LOG";    // Iridium Downlinks (toGround)

// Iridium Global Buffers
#define maxPacketSize 128                // Total Downlink Packet Size (Includes dowlinkMessageSize)
#define downlinkMessageSize 100          // Additional Downlink Message Size
#define RX_BUF_LENGTH 49		             // Uplink Buffer Size

// Create Iridium Buffers
char downlinkMessage2[downlinkMessageSize]; //Downlink Buffer
uint8_t rxBuf[RX_BUF_LENGTH]; 			        //Uplink Buffer
bool downlinkData;

int sbd_csq; // Signal Quality

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


// Begin Setup
void setup()
{
    // Give the Teensy a second to turn on
    delay(1000);
    
    // Initialize Serial Ports
    OutputSerial.begin(COM_BAUD);
    XBeeSerial.begin(XBEE_BAUD);
    IridiumSerial.begin(SBD_BAUD);
  
    // Wait for Serial ports to open, then output
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
    // String("Init").getBytes(xbeeSendBuf,xbeeSendBufSize);   // Convert "Init" 2 bytes, dump into Message buffer
    // xbeeSend(GroundSL,xbeeSendBuf);                         // (Target,Message)
    snprintf(xbeeSendBuf, xbeeSendBufSize-1 , "Made Logs ");
    xbeeSend(GroundSL,xbeeSendBuf);
    memset(xbeeSendBuf,0,xbeeSendBufSize);

    // TODO: New code to test
    // Measure the battery voltage, and output over Serial and XBee
    float initVolts = 2.8 * analogRead(batteryPin);
    OutputSerial.print("VOLTS: ");OutputSerial.println(initVolts);
    snprintf(xbeeSendBuf, xbeeSendBufSize-1 , "VOLTS: %f", initVolts);
    xbeeSend(GroundSL,xbeeSendBuf);
    memset(xbeeSendBuf,0,xbeeSendBufSize);
    
    int err;
  
  // Begin satellite modem operation
  #ifdef SBD
    
    err = modem.begin();
  
    // If modem.begin() has issues, output and log the error
    if (err != ISBD_SUCCESS)
    {
        OutputSerial.print("Begin failed: error ");
        OutputSerial.println(err);
        logprint("Begin failed: error ");
        logprintln(err);
    }
  
    // Modem Settings
    modem.useMSSTMWorkaround(false);      // TODO, determine the impact of this line
    modem.adjustSendReceiveTimeout(120);  // TODO, Is this an ok value?
    
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
  
    // TODO: Verify Removal Via Test
    // String("ModemCSQ:"+String(sbd_csq)).getBytes(xbeeSendBuf,xbeeSendBufSize);
    snprintf(xbeeSendBuf, xbeeSendBufSize-1 , "ModemCSQ: %d", sbd_csq);
    xbeeSend(GroundSL,xbeeSendBuf);

  #endif
  // End SBD ifdef
  
    // Force a hold on setup until GPS lock is acquired
    // Stable lock is achieved when altitude is within acceptable bounds
    
    // TODO: Verify Removal Via Test
    //String("GPS_Acquisition_Phase").getBytes(xbeeSendBuf,xbeeSendBufSize);
    //xbeeSend(GroundSL,xbeeSendBuf);
    xbeeSend(GroundSL,"GPS_Acquisition_Phase");
  
    // TODO: Remove instance of String
    // TODO: Verify Removal Via Test
    //String gpsPacket;
    
    if (USEGPS) {
        OutputSerial.println("TryingGPS");
    
        // Wait until valid lock is established to continue
        // TODO: Add an output for connected satellites to indicate status while waiting
        gpsWaitForLock();
    
        // TODO: Verify Removal Via Test
        // gpsPacket = String(gpsInfo.GPSTime)+","+String(gpsInfo.GPSLat,4)+","+String(gpsInfo.GPSLon,4)+","+String(gpsInfo.GPSAlt);
        // txLogFile.println(gpsPacket);
        // OutputSerial.println("gpsPacket  " +String(gpsPacket));
    
        logprintln("GPS_LOCK");
        gpsLockBlink();
        
        // Indicate GPS Lock has been acquired
        // TODO: Verify Removal Via Test
        //String("gotLock").getBytes(xbeeSendBuf,xbeeSendBufSize);
        xbeeSend(GroundSL,"GPS_LOCK");  
    } else {
        OutputSerial.println("Not Using GPS");
    }


  //Honestly consider canning this whole section, really not needed considering loop does the same thing
  // TODO, I'm ditching all of this, we just repeat in loop
  // TODO: Verify Removal Via Test
  /*
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
  */
    // TODO: Verify Removal Via Test
    // String("EnteringLoop").getBytes(xbeeSendBuf,xbeeSendBufSize);
    // xbeeSend(GroundSL,xbeeSendBuf);
    xbeeSend(GroundSL,"EnteringLoop");
}

// -------------------------------- End of Setup Beginning of Loop --------------------------------

void loop()
{  
    // During the Iridium retry protocol anything that still needs to happen needs to be in ISBDCallback()
    ISBDCallback();
  
    xbeeRead();
    
    LogPacket();
    
    // Check Signal Quality if its been more than signalCheckInterval since the last check
    if (((millis() - lastSignalCheck) > signalCheckInterval)) {
        int new_csq = 0;
        int csq_err = modem.getSignalQuality(new_csq);
        if(csq_err == 0) // If executed during an Iridium session, this will yield an ISBD_REENTRANT; keep previous value
        {
          sbd_csq = new_csq;
        }
        else if(csq_err == ISBD_REENTRANT){
          // KeepCalm, carry on  
        }else{
          logprint("CSQ error code = ");
          logprintln(csq_err);
        }
        lastSignalCheck = millis();
    }
  
    // Transmit SBD Packet Via Iridium
    // If Signal Quality is not an error, and messageTimeInterval time has elapsed, and sendingMessages flag is true:    
    if ((sbd_csq > 0 && (millis() - lastMillisOfMessage) > messageTimeInterval) && sendingMessages) {
        size_t rx_buf_size = RX_BUF_LENGTH;

        // TODO: New code to test
        // Measure the battery voltage, and output over Serial and XBee
        float initVolts = 2.8 * analogRead(batteryPin);
          
        // Assesmble the packet to be sent to the ground
        // gpsInfo is maintained in global, so no need to parse values
        char Packet2[maxPacketSize];
        snprintf(Packet2,maxPacketSize,"%06d,%4.4f,%4.4f,%u,%3.1f",gpsInfo.GPSTime,gpsInfo.GPSLat,gpsInfo.GPSLon,gpsInfo.GPSAlt,initVolts); //Build the packet
    
        // If there is XBee data to downlink, then add that data to the SBD Packet
        if(downlinkData){ 
            logprint("XbeeDown: ");
            logprintln(downlinkMessage2);
            
            // Add data from downlinkMessage2 (XBee) and put it into Packet2 (SBD Packet)
            // strncat cuts truncates data if it would cause maxPacketSize to be exceeded
            strncat(Packet2,downlinkMessage2,(maxPacketSize - strlen(Packet2) - 1));
            downlinkData = false;
            // Nuke the downlinkMessage2 buffer
            memset(downlinkMessage2, 0, downlinkMessageSize);
          }    
      
      logprint(Packet2);
      logprintln("Loop Sending");
      OutputSerial.println("Loop Sending");
      
      // Write the SBD TX packet to the txFile, then flush the file
      txLogFile.println(Packet2);
      txLogFile.flush();
  
      // SBD Modem Send
      uint8_t sbd_err = modem.sendReceiveSBDText(Packet2,rxBuf,rx_buf_size);  //Message, RecieveBuffer, SizeOfBuffer
  
      logprint("SBD tx/rx completed. Return Code: ");
      logprintln(sbd_err); // 0 is good
      
      OutputSerial.println("Send Error:  " +String(sbd_err));

      // TODO: Verify Removal Via Test
      // rxLogFile.print(String(gpsInfo.GPSTime)); //Log time of RX
      // rxLogFile.print("\t");
      rxLogFile.println(gpsInfo.GPSTime);
      
      // Process Uplink Messages Recieved during the SBDText
      sbdUplink();

      // Dump the raw rxBuf to the rxLogFile
      for (int k = 0; k < rx_buf_size; k++)
      {
          rxLogFile.write(rxBuf[k]); // Trying write instead of print, shoud fix garbage data
          rxBuf[k] = 0;
          delay(1);
      }
      rxLogFile.println();
      rxLogFile.flush();
  
      lastMillisOfMessage = millis();
    }
}
// ---------------------------------------- End of Loop ----------------------------------------

// The following continues to occur when attempting SBD transmissions
bool ISBDCallback() {
  
  // Parse the GPS without delay
  while (gpsserial.available()){
    if (gps.encode(gpsserial.read())){
      gpsInfo = getGPS();
      break;
    }
  }

  // Check the Xbee
  xbeeRead();

  // Log GPS
  LogPacket();
  
  return true;
}

// Logprint collection of methods. Probably excessive, but functional.
// Logfile is purposely kept closed most of the time to avoid corruption.

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

// TODO: Maybe just remove DIAGNOSTICS
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

// Logging GPS packets
void LogPacket(){

  // If it has been at least gpsLogInterval ms since last packet, do:
  if(millis()-lastLog>gpsLogInterval){
    // Create buffers
    char gpsLogPacket2[50];
    //char exactTime[9];//A somewhat convaluted way of adding : into the integer timestamp... //TEST

    // Handle time, and push into reduced character format (HHMMSS)
    // TODO: Verify Removal Via Test
    //snprintf(exactTime,7,"%d",gpsInfo.GPSTime);
    //char hour[3];char min[3];char sec[3];
    //strncpy(hour, &exactTime[0], 2);hour[2] = '\0';
    //strncpy(min, &exactTime[2], 2);min[2] = '\0';
    //strncpy(sec, &exactTime[4], 2);sec[2] = '\0';
    //snprintf(exactTime,9,"%s:%s:%s",hour,min,sec);

    // Build the GPS packet
    // TODO: Verify Removal Via Test
    //snprintf(gpsLogPacket2,35,"%s,%4.4f,%4.4f,%u",exactTime,gpsInfo.GPSLat,gpsInfo.GPSLon,gpsInfo.GPSAlt); //Fix Timestep
    snprintf(gpsLogPacket2,45,"%06d,%4.5f,%4.5f,%u",gpsInfo.GPSTime,gpsInfo.GPSLat,gpsInfo.GPSLon,gpsInfo.GPSAlt); //Build the packet

    // Write packet to file, and flush the file
    gpsLogFile.println(gpsLogPacket2);
    gpsLogFile.flush();
    
    lastLog=millis();
  }
}

