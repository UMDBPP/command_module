#include <jtXBee.h>

# define xbeeSerial Serial1

jtXBee celltracker(xbeeSerial);

void setup(){
  xbeeSerial.begin(9600);
  
  
  char packetData[160] = "x?";//"131717,39.6967,-78.1957,172,test";
  char phonenumber[12] = "7324849689";
  //celltracker.txSMS(phonenumber, packetData);
}

void loop(){
  celltracker.readSerial();
  delay(500);
}


