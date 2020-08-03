#include <jtXBee.h>

# define xbeeSerial Serial1

char xbeeBuffer[100];
const int bufSize = 100;
jtXBee celltracker(xbeeBuffer, bufSize);

char packetData[40] = "131717,39.6967,-78.1957,172,test";
char phonenumber[12] = "7324849689";

void setup(){
  xbeeSerial.begin(9600);
  Serial.begin(9600);

  delay(500);
  
  Serial.println("Init:");

  // Generate outputData containing packetData to be txSMS to phonenumber
  char outputData[80];
  memset(outputData, 0, 80);
  celltracker.txSMS(phonenumber, packetData, outputData);
  // Send outputData to XBee
  for(int i = 0; i < 80; i++){
    //xbeeSerial.write(outputData[i]);
    Serial.println((int)outputData[i]);
  }
  celltracker.nukeBuffer();
  
  Serial.println("EnterLoop;");
}

void loop(){

  // Check if incoming data
  if(xbeeSerial.available()){
      celltracker.nukeBuffer();
      while(xbeeSerial.available()>0){
        celltracker.process(xbeeSerial.read());
      }
  }

  // If received a text
  if(celltracker.is_sms_ready()){
      
      char rxNumber[10];
      char rxData[20];
      memset(rxNumber,0,10);
      memset(rxData,0,20);
      
      if(celltracker.rxSMS(rxNumber, rxData, 20)){
        Serial.println("GoodRec");
      }

      // Print Number
      Serial.println("rxNumber");
      for(int i = 0; i < 10; i++){
        Serial.println((char)rxNumber[i]);  
      }

      // Print Data
      Serial.println("rxData");
      for(int i = 0; i < 20; i++){
        Serial.println((char)rxData[i]);  
      }

      if(strstr(rxNumber,phonenumber)){
        Serial.println("Got message from my phone");

        if(strstr(rxData,"start")){
          Serial.println("startTexts");
        }else if(strstr(rxData,"stop")){
          Serial.println("stopTexts");
        }
        
      }
  }
   
  delay(500);
}


