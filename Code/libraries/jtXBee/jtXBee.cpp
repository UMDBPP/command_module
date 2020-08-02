#include "jtXBee.h"
#include "Arduino.h"

// sym 

jtXBee::jtXBee(char* buffer, int bufSize){
	_buffer = buffer;
	_bufSize = bufSize;
	_ready = false;
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
	
	if(_pos >= (3+_packetLength)){
		_pos = 0;
		process_packet();
	}
}

bool jtXBee::isAvailable(){
	return _ready;
}

bool jtXBee::process_packet(){
	// Dump for Test
	for(int i = 0; i < _bufSize; i++){
		Serial.println((int)_buffer[i]);
	}
	
	// Calculate correct checksum
	int sum = 0;
	for(int i = 3; i < _packetLength+3; i++){
		sum += (int)_buffer[i];
	}
	
	// Check the checksum
	if((255 - (sum&0xFF)) == int(_buffer[_packetLength+3])){
		return false;
	}
	
	// Switch through different packet types
	switch((int)_buffer[3]){
		// 9F / SMS_RX
		case 159 : 
			rxSMS();
			break;
		
		default : 
			return false;
	}
}

void jtXBee::nukeBuffer(){
	memset(_buffer,0,_bufSize);
}

void jtXBee::markRead(){
	_ready = false;
}

void jtXBee::rxSMS(){
	Serial.println("got a text");
	_ready = true;
}

void jtXBee::txSMS(char* phonenumber, char* packetData, char* outputData){
    int outputDataINT[100];

    //int startdelimINT = 126;
    //int frameTypeINT = 31;
    //int frameIdINT = 1;
    //int optionsINT = 0;

    //int length = 3;
    int sum = 32;//1+31 (10 + 1F)
    int finalData[100];
    memset(finalData, 0, 100);

    int counter = 0;

    //length += 20;
	int length = 23;

    //Number minus string
    int numberINT[22];
    counter = 0;
    for(int i=0;i<20;i++){
        if(counter<strlen(phonenumber)){
            numberINT[counter] = (phonenumber[i]);
            sum+=int(phonenumber[i]);
        }else{
            numberINT[counter] = 0;
        }
        counter++;
    }

    length+=strlen(packetData);

    int packetDataLength = strlen(packetData);

    counter = 0;
    for(int i = 0;i<packetDataLength;i++){
        outputDataINT[i] = int(packetData[i]);
        sum+=packetData[i];
    }

    //int checkINT = sum;
    //cout << "checkINT: " << checkINT << endl;

    //int decvalINT = 255 - checkINT&0xFF;
	//int decvalINT = 255 - sum&0xFF;
    //cout << "decvalINT: " << decvalINT << endl;


    //int packetLengthINT = (length);

    counter = 0;
    for(int i:numberINT){
        finalData[6+counter] = i;
        counter++;
    }

    counter = 0;
    for(int iter = 0;iter<packetDataLength;iter++){
        finalData[26+counter] = (outputDataINT[iter]);
        counter++;
    }

    //int
    finalData[0] = 126;//startdelimINT;
    finalData[1] = 0;
    finalData[2] = length;
    finalData[3] = 31;//frameTypeINT;
    finalData[4] = 1;//frameIdINT;
    finalData[5] = 0;//optionsINT;
    finalData[26+counter] = (255 - (sum&0xFF));

    datalength = 27+counter;
	
	
	for(int i = 0; i < 99; i++){
		outputData[i] = (char)finalData[i];
	}
	
	/**
    for(int i = 0;i<datalength;i++){
	    _xbeeSerial.write(finalData[i]);
	    delay(1);
    }
	*/
}