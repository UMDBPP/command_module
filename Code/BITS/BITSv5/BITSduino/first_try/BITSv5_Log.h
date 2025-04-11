#ifndef _BITSv5_LOG_H
#define _BITSv5_LOG_H

// Initializing Log Files; Must be in form XXXXXXXX.log; no more than 8 'X'
// characters
File gpsLogFile;
File rxLogFile;
File txLogFile;

const String gpsLogName = "GPS.LOG"; // 1 sec position/altitude updates
const String eventLogName =
    "EVENT.LOG"; // Events Iridium/XBee (Special one thanks to Luke)
const String rxLogName = "RX.LOG"; // Iridium Uplinks   (toBalloon)
const String txLogName = "TX.LOG"; // Iridium Downlinks (toGround)

void log_init(void);

static void logprint_str(String s);
static void logprint_int(int i);
static void logprint_double(double d);
static void logprintln_str(String s);
static void logprintln_int(int i);
static void logprintln_double(double d);
static void logprintln_uint32(uint32_t d);

void LogPacket(void);

#endif
