#include "jtXBee.h"
#include "Arduino.h"

// sym 

jtXBee::jtXBee(char* buffer, int bufSize){
	_buffer = buffer;
	_bufSize = bufSize;
	_sms_ready = false;
}

bool jtXBee::process(int byte){
	// Keep track of position / reset on 7E byte
	if(byte == 126){
		_pos = 0;
		nukeBuffer();
	}else if(_pos > _bufSize){
		_pos = 0;
		return 0;
	}else{
		_pos++;
	}
	
	// Store byte to buffer
	_buffer[_pos] = (char)byte;
	
	// Get Length
	if(_pos == 1) {
		_packetLength = byte*256;
	}else if(_pos == 2){
		_packetLength += byte;
	}
	
	Serial.println(byte);
	if(_pos >= (3+_packetLength)){
		_pos = 0;
		Serial.println("proc");
		process_packet();
	}
}

bool jtXBee::is_sms_ready(){
	return _sms_ready;
}

bool jtXBee::process_packet(){
	// Dump for Test
	for(int i = 0; i < _bufSize; i++){
		//Serial.println((int)_buffer[i]);
	}
	
	// Calculate correct checksum
	int sum = 0;
	for(int i = 3; i < _packetLength+3; i++){
		sum += (int)_buffer[i];
	}
	
	// Check the checksum
	if((255 - (sum&0xFF)) != int(_buffer[_packetLength+3])){
		return false;
	}
	
	// Switch through different packet types
	Serial.println("3rdBuf");
	Serial.println(_buffer[3]);
	switch((int)_buffer[3]){
		// 9F / SMS_RX
		case 159 : 
			_sms_ready = true;
			Serial.println("pckt_159");
			break;
		
		default : 
			return false;
	}
	
	return true;
}

// Nuke the buffer
void jtXBee::nukeBuffer(){
	memset(_buffer,0,_bufSize);
}

bool jtXBee::rxSMS(char* phonenumber, char* data, int datasize){
	
	// Get the number
	for(int i = 6; i <= 15; i++){
		phonenumber[i-6] = _buffer[i];
	}
	
	// Get the data
	for(int i = 24; i <= _packetLength+2; i++){
		if((i-24) > datasize){
			return false;
		}
		data[i-24] = _buffer[i];
	}
	
	_sms_ready = false;
	
	return true;
}

void jtXBee::txSMS(char* phonenumber, char* packetData, char* outputData){
    

    int sum = 32;//1+31 (10 + 1F)
    
	int datalength = 23+strlen(packetData);

	// Data
    for(int i = 0;i<strlen(packetData);i++){
		outputData[26+i] = (packetData[i]);
        sum+=packetData[i];
    }

	// phone#
	for(int i=0; i<strlen(phonenumber); i++){
		outputData[i+6] = phonenumber[i];
		sum+=phonenumber[i];
	}
	
	
	outputData[0] = 126;//startdelimINT;
    outputData[1] = 0;
    outputData[2] = datalength;
    outputData[3] = 31;//frameTypeINT;
    outputData[4] = 1;//frameIdINT;
    outputData[5] = 0;//optionsINT;
    outputData[datalength+3] = (char)(255 - (sum&0xFF));
}