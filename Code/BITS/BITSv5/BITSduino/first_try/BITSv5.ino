#include "BITSv5.h"

void setup_handler(void) {
  Serial.begin(115200);
  IridiumSerial.begin(SBD_BAUD);

  delay(1000);
  Serial.println("BITSv5");

  gps_init(); // Setup the GPS

  xbee.setSerial(Serial2);
  startBlinks();

  // Open Files
  log_init();
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
  logprintln_str("INIT_LOG_LOG");


  // XBee Init Message to ground stations
  String("Init").getBytes(
      xbeeSendBuf,
      xbeeSendBufSize); // Convert "Init" 2 bytes, dump into Message buffer
  xbeeSend(GroundSL, xbeeSendBuf); //(Target,Message)

  int err;

#ifdef SBD // Begin satellite modem operation
  OutputSerial.println("Starting modem...");
  err = modem.begin();
  modem.useMSSTMWorkaround(false); // SUPER NF TESTTHIS TODO(2)
  modem.adjustSendReceiveTimeout(120);
  if (err != ISBD_SUCCESS) {
    OutputSerial.print("Begin failed: error ");
    OutputSerial.println(err);
    logprint_str("Begin failed: error ");
    logprintln_int(err);
    if ((err == ISBD_NO_MODEM_DETECTED) || (err == 5))
      Serial.println("No modem detected: check wiring.");
    return;
  }

  OutputSerial.println("Modem started");
  String("ModemStarted").getBytes(xbeeSendBuf, xbeeSendBufSize);
  xbeeSend(GroundSL, xbeeSendBuf);

#ifdef ISBD_CHECK_FIRMWARE // Example: Print the firmware revision
  char version[12];
  err = modem.getFirmwareVersion(version, sizeof(version));
  if (err != ISBD_SUCCESS) {
    OutputSerial.print("FirmwareVersion failed: error ");
    OutputSerial.println(err);
    logprint_str("FirmwareVersion failed: error ");
    logprintln_str(err);
    return;
  }
  OutputSerial.print("Firmware Version is ");
  OutputSerial.println(version);
#endif

  // From 0-5, 2 or better is preferred (Still works at 0)
  err = modem.getSignalQuality(sbd_csq);
  if (err != ISBD_SUCCESS) {
    OutputSerial.print("SignalQuality failed: error ");
    OutputSerial.println(err);
    logprint_str("SignalQuality failed: error ");
    logprintln_int(err);
    sbd_csq = -1;
    return;
  }

  OutputSerial.print("From 0 to 5, signal qual is: ");
  OutputSerial.println(sbd_csq);

  String("ModemCSQ:" + String(sbd_csq)).getBytes(xbeeSendBuf, xbeeSendBufSize);
  xbeeSend(GroundSL, xbeeSendBuf);
#endif

  String("GPS_Acquisition_Phase").getBytes(xbeeSendBuf, xbeeSendBufSize);
  xbeeSend(GroundSL, xbeeSendBuf);

  // HOLD UNTIL LOCK
  String gpsPacket;
  if (USEGPS) {
    OutputSerial.println("TryingGPS");

    // GPS LOCK INIT
    while ((gpsInfo.GPSAlt <= 0) ||
           (gpsInfo.GPSAlt > 100000)) // Outside of a reasonable range
    {
      // delay(500);
      // output();
      while (gpsserial.available()) {
        if (gps.encode(gpsserial.read())) {
          gpsInfo = getGPS();
          break;
        }
      }
    }

    gpsPacket = String(gpsInfo.GPSTime) + "," + String(gpsInfo.GPSLat, 4) +
                "," + String(gpsInfo.GPSLon, 4) + "," + String(gpsInfo.GPSAlt);
    txLogFile.println(gpsPacket);
    logprintln_str("GotLock");
    gpsLockBlink();
    OutputSerial.println("gpsPacket  " + String(gpsPacket));

    // Got Lock Notification
    String("gotLock").getBytes(xbeeSendBuf, xbeeSendBufSize);
    xbeeSend(GroundSL, xbeeSendBuf);

  } else {
    OutputSerial.println("Not Using GPS");
    gpsPacket = "test";
  }

// Honestly consider canning this whole section, really not needed considering
// loop does the same thing
#ifdef SBD
  // Send the message
  char sbd_buf[49]; // TX BUFFER
  OutputSerial.print(
      "Trying to send the message.  This might take several minutes.\r\n");
  gpsPacket.toCharArray(sbd_buf, 49);
  err = modem.sendSBDText(
      sbd_buf); // Sends an initial packet AFTER gps has locked.
  if (err != ISBD_SUCCESS) {
    OutputSerial.println("sendSBDText failed: error " + String(err));
    OutputSerial.println();
    logprint_str("sendSBDText failed: error ");
    logprintln_int(err);
    if (err == ISBD_SENDRECEIVE_TIMEOUT)
      OutputSerial.println("Try again with a better view of the sky.");
    logprintln_str("Try again with a better view of the sky.");
  } else {
    OutputSerial.println("Success, sent = " + String(sbd_buf));
    logprintln_str("Success, sent = " + String(sbd_buf));
  }
#endif

  String("EnteringLoop").getBytes(xbeeSendBuf, xbeeSendBufSize);
  xbeeSend(GroundSL, xbeeSendBuf);
}

void loop_handler(void) {

  ISBDCallback(); // During the Iridium retry protocol anything that still needs
                  // to happen needs to go in here

  xbeeRead();
  LogPacket();

  // Check Signal Quality
  // if (((millis() - lastSignalCheck) > signalCheckInterval) && (millis() <
  // shutdownTimeInterval)) {
  if (((millis() - lastSignalCheck) > signalCheckInterval)) {
    int new_csq = 0;
    int csq_err = modem.getSignalQuality(new_csq);
    if (csq_err == 0) // if executed during an Iridium session, this will yield
                      // an ISBD_REENTRANT; keep previous value
    {
      sbd_csq = new_csq;
    } else if (csq_err == ISBD_REENTRANT) {
      // KeepCalm, carry on
    } else {
      logprint_str("CSQ error code = ");
      logprintln_int(csq_err);
    }
    lastSignalCheck = millis();
  }

  // Transmit Via Iridium
  //  If not currently transmitting, and at time, and transmitting messages ON,
  //  transmit
  // if ((sbd_csq > 0 && (millis() - lastMillisOfMessage) > messageTimeInterval)
  // && (millis() < shutdownTimeInterval) && sendingMessages) {
  if ((sbd_csq > 0 && (millis() - lastMillisOfMessage) > messageTimeInterval) &&
      sendingMessages) {

    size_t rx_buf_size = RX_BUF_LENGTH; // RECENT CHANGE via TODO(1)

    // Assesmble the packet to be sent to the ground
    char Packet2[maxPacketSize];
    snprintf(Packet2, maxPacketSize, "%06d,%4.4f,%4.4f,%u", gpsInfo.GPSTime,
             gpsInfo.GPSLat, gpsInfo.GPSLon,
             gpsInfo.GPSAlt); // Build the packet

    // If there is stuff to downlink than do
    if (downlinkData) {
      logprint_str("DownAttempt: ");
      logprintln_str(downlinkMessage2);
      strncat(
          Packet2, downlinkMessage2,
          (maxPacketSize - strlen(Packet2) -
           1)); // Add data from downlinkMessage2 into Packet2, check for size
      downlinkData = false;
      memset(downlinkMessage2, 0, downlinkMessageSize); // Clear downlink
                                                        // message
    }

    logprint_str(Packet2);
    logprintln_str("Loop Sending");
    OutputSerial.println("Loop Sending");
    txLogFile.println(Packet2);
    txLogFile.flush();

    // SEND/RECEIVE DATA, return status
    uint8_t sbd_err = modem.sendReceiveSBDText(
        Packet2, rxBuf, rx_buf_size); // Message, RecieveBuffer, SizeOfBuffer

    logprint_str("SBD send receive completed with return code: ");
    logprintln_int(sbd_err); // 0 is good
    OutputSerial.println("Send Error:  " + String(sbd_err));

    rxLogFile.print(String(gpsInfo.GPSTime)); // Log time of RX
    rxLogFile.print("\t");

    // Process Received Messages
    uplink(); // Run uplink message handling (bitsIncoming)

    for (int k = 0; k < rx_buf_size; k++) // Prints RX characters to SD file
    {
      rxLogFile.write(
          rxBuf[k]); // Trying write instead of print, shoud fix garbage data
      rxBuf[k] = 0;
      delay(1);
    }
    rxLogFile.println();
    rxLogFile.flush();

    lastMillisOfMessage = millis();
  }
}
