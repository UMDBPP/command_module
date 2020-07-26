#include <jtXBee.h>

# define xbeeSerial Serial1

jtXBee celltracker(xbeeSerial);

char xbeeBuffer[100];

void setup(){
  xbeeSerial.begin(9600);
  Serial.begin(9600);
  
  char packetData[160] = "x?";//"131717,39.6967,-78.1957,172,test";
  char phonenumber[12] = "7324849689";
  //celltracker.txSMS(phonenumber, packetData);
}

void loop(){
//  if(xbeeSerial.available()){
//    memset(xbeeBuffer,0,100);
//    int i = 0;
//    while(xbeeSerial.available()>0){      
//        xbeeBuffer[i] = xbeeSerial.read();
//        i++;
//    }
//
//    for(i = 0;i<100;i++){
//      Serial.println(int(xbeeBuffer[i]));
//    }
//  }
  delay(500);
}


