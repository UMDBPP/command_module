#include <jtXBee.h>

# define xbeeSerial Serial3

char xbeeBuffer[100];
const int bufSize = 100;
jtXBee celltracker(xbeeBuffer, bufSize);

char packetData[40] = "131717,39.6967,-78.1957,172,test";
char phonenumber[12] = "7324849689";

const int outputDataSize = 80;

unsigned long int next_rep = 0;

void setup(){
  xbeeSerial.begin(9600);
  Serial.begin(115200);

  delay(500);
  
  Serial.println("Init");

  // Generate outputData containing packetData to be txSMS to phonenumber
  
  //char outputData[outputDataSize];
  //memset(outputData, 0, outputDataSize);
  //celltracker.txSMS(phonenumber, packetData, outputData, outputDataSize);
  // Send outputData to XBee
  for(int i = 0; i < outputDataSize; i++){
    //xbeeSerial.write(outputData[i]);
    //Serial.println((int)outputData[i]);
  }
  
  
  //celltracker.nukeBuffer();
  Serial.println("EnterLoop");
}

void loop(){

  // Check if incoming data
  
  if(xbeeSerial.available()){
    Serial.println("hit_on_serial");
    
      //celltracker.nukeBuffer();
      while(xbeeSerial.available()>0){
        
        int a = xbeeSerial.read();
        celltracker.process(a);
        Serial.print("proc_char: ");
        Serial.println(a);
        
      }
  }

  // If received a text
  if(celltracker.is_sms_ready()){
    Serial.println("sms_ready");
      
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
   
  if(millis()>next_rep){
    next_rep = millis()+5000;
    Serial.println("alive");
  }
}


