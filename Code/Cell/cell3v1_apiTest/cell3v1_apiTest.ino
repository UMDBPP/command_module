#include <jtXBee.h>

# define xbeeSerial Serial1

char xbeeBuffer[100];
const int bufSize = 100;
jtXBee celltracker(xbeeBuffer, bufSize);


void setup(){
  xbeeSerial.begin(9600);
  Serial.begin(9600);
  
  char packetData[100] = "x?";//"131717,39.6967,-78.1957,172,test";
  char phonenumber[12] = "7324849689";
  char outputData[100];
  memset(outputData, 0, 100);
  celltracker.txSMS(phonenumber, packetData, outputData);

  delay(1000);
  
  Serial.println("Init:");
  
  for(int i = 0; i < 100; i++){
    //xbeeSerial.write(outputData[i]);
  }

  celltracker.nukeBuffer();
  for(int i = 0; i < bufSize; i++){
    //Serial.println((int)xbeeBuffer[i]);
  }

  Serial.println("EnterLoop;");
}

void loop(){
  if(xbeeSerial.available()){
    celltracker.nukeBuffer();
    //int i = 0;
    while(xbeeSerial.available()>0){      
        //xbeeBuffer[i] = xbeeSerial.read();
        celltracker.process(xbeeSerial.read());
        //i++;
      //Serial.println((int)xbeeSerial.read());
    }

    
  }
  delay(500);
}


