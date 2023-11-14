//An overly complicated piece of code for and overly simple little groundstation box
//Maybe at some point this can have a GUI, but this is still WIP
//Jonathan

#include <XBee.h>
#include <SoftwareSerial.h>

XBee xbee = XBee();
XBeeResponse response = XBeeResponse();
SoftwareSerial xbeeSerial(2, 3);
//#define xbeeSerial Serial2 //How you would do it if you wanted to use hardware serial


const uint32_t BitsSL = 0x417B4A3B;   //BITS   (white)Specific to the XBee on Bits (the Serial Low address value)
const uint32_t GroundSL = 0x417B4A36; //GndStn (u.fl)
const uint32_t BlueSL = 0x417B4A3A;   //Choppy 2 (blue)
const uint32_t WireSL = 0x419091AC;   //Choppy 1 (wire antenna)
const uint32_t GHOULSL = 0x4210F7E4;  //GHOUL
const uint32_t UniSH = 0x0013A200;    //Common across any and all XBees

ZBTxStatusResponse txStatus = ZBTxStatusResponse(); //What lets the library check if things went through
ZBRxResponse rx = ZBRxResponse();                   //Similar to above
ModemStatusResponse msr = ModemStatusResponse();    //And more
const int xbeeRecBufSize = 50; //Rec must be ~15bytes larger than send because
const int xbeeSendBufSize = 35;//there is overhead in the transmission that gets parsed out
uint8_t xbeeRecBuf[xbeeRecBufSize];
uint8_t xbeeSendBuf[xbeeSendBufSize];

void setup() {
  Serial.begin(115200);
  xbeeSerial.begin(38400);
  delay(1000);
  xbee.setSerial(xbeeSerial); //Sets which serial the xbee object listens to
  startPrompt();
}

void loop() {
  if(Serial.available()>0){ //Allows someone to send serial commands to the box
      char pick = Serial.read();
      if(pick=='1')
      {
        Serial.println("ToBits");
        delay(100);
        while(!Serial.available()){}
        if(Serial.available()>0){
          Serial.print("Sending: ");
          Serial.readBytes((char*)xbeeSendBuf,xbeeSendBufSize); //Read bytes in over serial
          Serial.write(xbeeSendBuf,xbeeSendBufSize);
          Serial.println();
          //Display what you've attempted to send
          xbeeSend(BitsSL,xbeeSendBuf);
        }
      }else if(pick=='2')
      {
        Serial.println("ToMars"); //Really should color code these XBees instead of using payload names
        delay(100);
        while(!Serial.available()){}
        if(Serial.available()>0){
          Serial.print("Sending: ");
          Serial.readBytes((char*)xbeeSendBuf,xbeeSendBufSize); //Read bytes in over serial
          Serial.write(xbeeSendBuf,xbeeSendBufSize);
          Serial.println();
          //Display what you've attempted to send
          xbeeSend(BlueSL,xbeeSendBuf);
        }
      }else if(pick=='3')
      {
        Serial.println("ToChoppy1"); //Really should color code these XBees instead of using payload names
        delay(100);
        while(!Serial.available()){}
        if(Serial.available()>0){
          Serial.print("Sending: ");
          Serial.readBytes((char*)xbeeSendBuf,xbeeSendBufSize); //Read bytes in over serial
          Serial.write(xbeeSendBuf,xbeeSendBufSize);
          Serial.println();
          //Display what you've attempted to send
          xbeeSend(WireSL,xbeeSendBuf);
        }
      }else if(pick=='4')
      {
        Serial.println("ToChoppy2"); //Really should color code these XBees instead of using payload names
        delay(100);
        while(!Serial.available()){}
        if(Serial.available()>0){
          Serial.print("Sending: ");
          Serial.readBytes((char*)xbeeSendBuf,xbeeSendBufSize); //Read bytes in over serial
          Serial.write(xbeeSendBuf,xbeeSendBufSize);
          Serial.println();
          //Display what you've attempted to send
          xbeeSend(BlueSL,xbeeSendBuf);
        }
      }
      else if(pick=='5')
      {
        Serial.println("ToGHOUL"); //Really should color code these XBees instead of using payload names
        delay(100);
        while(!Serial.available()){}
        if(Serial.available()>0){
          Serial.print("Sending: ");
          Serial.readBytes((char*)xbeeSendBuf,xbeeSendBufSize); //Read bytes in over serial
          Serial.write(xbeeSendBuf,xbeeSendBufSize);
          Serial.println();
          //Display what you've attempted to send
          xbeeSend(GHOULSL,xbeeSendBuf);
        }
      }
      else if(pick=='r') //Clear the terminal
      {
        Serial.write(27);
        Serial.print("[2J");
        Serial.write(27);
        Serial.print("[H");
        //startPrompt();
      }else if(pick=='g') //Clear the terminal
      {
        Serial.write(27);
        Serial.print("[5S");
        Serial.write(27);
        Serial.print("[H");
        //startPrompt();
      }else if(pick=='c') //Clear the terminal
      {
        for(int i=0;i<10;i++){
          Serial.println('\n');
        }
        startPrompt();
      }
      else if(pick=='p') //Clear the terminal
      {
        startPrompt();
      }
  }
  
  
  xbeeRead(); //Checks buffer, does stuff if there are things to do
}

bool xbeeSend(uint32_t TargetSL,uint8_t* payload){
  XBeeAddress64 TargetAddress = XBeeAddress64(UniSH,TargetSL);      //The full address, probably could be done more efficiently, oh well
  ZBTxRequest zbTx = ZBTxRequest(TargetAddress, payload, xbeeSendBufSize); //Assembles Packet
  xbee.send(zbTx);                                                  //Sends packet
  memset(xbeeSendBuf, 0, xbeeSendBufSize);                          //Nukes buffer
  if (xbee.readPacket(10)) {                                       //Checks Reception
    if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {   //If rec
      xbee.getResponse().getZBTxStatusResponse(txStatus);
      if (txStatus.getDeliveryStatus() == SUCCESS) {                //If positive transmit response
        Serial.println("SuccessfulTransmit");
        //startPrompt();
        return true;
      } else {
        Serial.println("TxFail");
        //startPrompt();
        return false;
      } 
    }
  } else if (xbee.getResponse().isError()) { //Stil have yet to see this trigger, might be broken...
    Serial.print("Error reading packet.  Error code: ");
    Serial.println(xbee.getResponse().getErrorCode());
  } else {
    Serial.println("Send Failure, check that remote XBee is powered on");  
  }
  return false;
}

void xbeeRead(){
  xbee.readPacket(); //read serial buffer
    if (xbee.getResponse().isAvailable()) { //got something
      if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) { //got a TxRequestPacket
        xbee.getResponse().getZBRxResponse(rx);
        
        uint32_t incominglsb = rx.getRemoteAddress64().getLsb(); //The SL of the sender
        Serial.print("Incoming Packet From: ");
        Serial.println(incominglsb,HEX);
        if(rx.getPacketLength()>=xbeeRecBufSize){                //Probably means something is done broke
          Serial.print("Oversized Message: ");
          Serial.println(rx.getPacketLength());
        }
        memset(xbeeRecBuf, 0, xbeeRecBufSize); // Nukes old buffer
        memcpy(xbeeRecBuf,rx.getData(),rx.getPacketLength());
        if(incominglsb == BitsSL){ //Seperate methods to handle messages from different senders
          processBitsMessage();    //prevents one payload from having the chance to be mistaken as another
        }
        if(incominglsb == BlueSL){ //Config for Mars
          processMarsMessage();
        }
        if(incominglsb == WireSL){ //Config for Choppy
          processChoppyMessage();
        }    
        if(incominglsb == GHOULSL){ //Config for Choppy
          processGHOULMessage();
        }    
      }
    }
}
void startPrompt(){
  Serial.println("XBee Ground Station Box:");
  Serial.println("Enter Message Target (1 BITS, 2 Mars, 3 Choppy1, 4 Choppy2, 5 GHOUL)");
}

void processBitsMessage(){ //Just print things to the monitor
  Serial.println("RecFromBits");
  Serial.write(xbeeRecBuf,xbeeRecBufSize);
  Serial.println();
  //startPrompt();
}

void processMarsMessage(){ //Just print things to the monitor
  Serial.println("RecFromMars");
  Serial.write(xbeeRecBuf,xbeeRecBufSize);
  Serial.println();
  //startPrompt();
}

void processChoppyMessage(){ //Just print things to the monitor
  Serial.println("RecFromChoppy");
  Serial.write(xbeeRecBuf,xbeeRecBufSize);
  Serial.println();
  //startPrompt();
}

void processGHOULMessage(){ //Just print things to the monitor
  Serial.println("RecFromGHOUL");
  Serial.write(xbeeRecBuf,xbeeRecBufSize);
  Serial.println();
  //startPrompt();
}
/**
void processGroundMessage(){ //But THIS IS THE GROUND
  Serial.print("RecFromGND: ");
  Serial.write(xbeeRecBuf,xbeeRecBufSize);
  
  if(strstr((char*)xbeeRecBuf,"test")){ //Checks if "test" is within buffer
      Serial.println("");
      Serial.println("ackTest");
      String("PacketAck").getBytes(xbeeSendBuf,xbeeSendBufSize);
      xbeeSend(GroundSL,xbeeSendBuf);
  }
  if(strstr((char*)xbeeRecBuf,"TG")){ //Checks if "test" is within buffer
      Serial.println("");
      Serial.println("ToGround");
      String("ToGNDAck").getBytes(xbeeSendBuf,xbeeSendBufSize);
      xbeeSend(GroundSL,xbeeSendBuf);
  }  
}*/
