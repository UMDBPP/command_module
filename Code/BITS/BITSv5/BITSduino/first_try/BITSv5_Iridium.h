#ifndef _BITSv5_IRIDIUM_H
#define _BITSv5_IRIDIUM_H

// Possible Iridium Error Codes
#define ISBD_SUCCESS 0
#define ISBD_ALREADY_AWAKE 1
#define ISBD_SERIAL_FAILURE 2
#define ISBD_PROTOCOL_ERROR 3
#define ISBD_CANCELLED 4
#define ISBD_NO_MODEM_DETECTED 5
#define ISBD_SBDIX_FATAL_ERROR 6
#define ISBD_SENDRECEIVE_TIMEOUT 7
#define ISBD_RX_OVERFLOW 8
#define ISBD_REENTRANT 9
#define ISBD_IS_ASLEEP 10
#define ISBD_NO_SLEEP_PIN 11
#define SBD

// Setting default datarates
#define SBD_BAUD 19200

#define maxPacketSize 128       // Standard(GPS/TIME, etc.) + Message Data
#define downlinkMessageSize 100 // Message Data
#define RX_BUF_LENGTH 49        // Uplink Buffer Size

#define IridiumSerial Serial1 // Iridium 9603

// Declare the IridiumSBD object
IridiumSBD modem(IridiumSerial);

int sbd_csq; // Signal Qual

// Iridium Global Buffers
uint8_t rxBuf[RX_BUF_LENGTH];               // Uplink Buffer
char downlinkMessage2[downlinkMessageSize]; // Downlink Buffer
bool downlinkData;

#endif
