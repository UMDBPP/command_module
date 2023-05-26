#ifndef xbeeInterpret_h
#define xbeeInterpret_h

#include "Arduino.h"

class jtXBee
{
public:
	jtXBee(Stream& s):_xbeeSerial(s){}
	void readSerial();
	void rxSMS(char* receivedData);
    void txSMS(char* phonenumber, char* packetData);
	int datalength;

private:
	Stream& _xbeeSerial;
};

#endif
