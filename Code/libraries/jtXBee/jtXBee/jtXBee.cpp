#include "jtXBee.h"
#include "Arduino.h"

void jtXBee::readSerial(){
	if(_xbeeSerial.available()>0){
		char* receivedBytes;
		int readlength = _xbeeSerial.available();
		_xbeeSerial.readBytes(receivedBytes,readlength);
		for(int i = 0; i < readlength ; i++){
			Serial.println(receivedBytes);
		}
		Serial.println("detected");
	}
	Serial.println("Pass");
}

void jtXBee::rxSMS(char* receivedBytes){

}

void jtXBee::txSMS(char* phonenumber, char* packetData){
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
    finalData[26+counter] = (255 - sum&0xFF);

    datalength = 27+counter;

    for(int i = 0;i<datalength;i++){
	    _xbeeSerial.write(finalData[i]);
	    delay(1);
  }
}