#ifndef _BITSv5_GPS_H
#define _BITSv5_GPS_H

const bool USEGPS = true; // Should be true 99% of the time

// GPS data struct and object
struct GPSdata {
  float GPSLat = -1;
  float GPSLon = -1;
  unsigned long GPSTime = -1;
  long GPSAlt = -1;
  int GPSSats = -1;
} gpsInfo;

SFE_UBLOX_GNSS myGNSS;

#endif
