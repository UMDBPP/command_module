#ifndef jtXBee_h
#define jtXBee_h

#include "Arduino.h"

class jtXBee
{
public:

	jtXBee(char* buffer, int bufSize);
    void txSMS(char* phonenumber, char* packetData, char* outputData);
	int datalength;
	void nukeBuffer();
	bool process(int byte);
	bool is_sms_ready();
	bool rxSMS(char* phonenumber, char* data, int datasize);
	
private:

	char* _buffer;
	int _bufSize;
	int _pos = 0;
	int _packetLength = 100;
	bool process_packet();
	bool _sms_ready = false;
	
};

#endif
