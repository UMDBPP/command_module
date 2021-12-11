//Using the hardware from cellv3rev1 to put an XBee on a payload in overkill fashion
// Code by Jonathan
// XBee library by Andrew Rapp, modified for gen3 XBees by Jonathan

#include <XBee.h>

XBee xbee = XBee();
XBeeResponse response = XBeeResponse();
#define xbeeSerial Serial1

#define triggerPin 3

const uint32_t BitsSL = 0x417B4A3B;   //BITS   (white)Specific to the XBee on Bits (the Serial Low address value)
const uint32_t GroundSL = 0x417B4A36; //GndStn (u.fl)
const uint32_t BlueSL = 0x417B4A3A;   //Choppy 2 (blue)
const uint32_t WireSL = 0x419091AC;   //Choppy 1 (wire antenna)
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
  delay(1000);
  Serial.println("INIT");
  Serial1.begin(9600);
  delay(1000);
  xbee.setSerial(xbeeSerial); //Sets which serial the xbee object listens to

  String("xbeeTrigger_ON").getBytes(xbeeSendBuf,xbeeSendBufSize);
  xbeeSend(GroundSL,xbeeSendBuf);

  pinMode(triggerPin,OUTPUT);
  digitalWrite(triggerPin,HIGH);
}

void loop() {
  xbeeRead(); //Checks buffer, does stuff if there are things to do
  delay(10);
}

bool xbeeSend(uint32_t TargetSL,uint8_t* payload){
  XBeeAddress64 TargetAddress = XBeeAddress64(UniSH,TargetSL);      //The full address, probably could be done more efficiently, oh well
  ZBTxRequest zbTx = ZBTxRequest(TargetAddress, payload, xbeeSendBufSize); //Assembles Packet
  xbee.send(zbTx);                                                  //Sends packet
  memset(xbeeSendBuf, 0, xbeeSendBufSize);                          //Nukes buffer
  if (xbee.readPacket(500)) {                                       //Checks Reception
    if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {   //If rec
      xbee.getResponse().getZBTxStatusResponse(txStatus);
      if (txStatus.getDeliveryStatus() == SUCCESS) {                //If positive transmit response
        Serial.println("SuccessfulTransmit");
        return true;
      } else {
        Serial.println("TxFail");
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
        if(incominglsb == BlueSL){ //Blue
          processBlueMessage();
        }
        if(incominglsb == GroundSL){ //Ground Station
          processGroundMessage();
        }    
      }
    }
}

void processBitsMessage(){ //Just print things to the monitor
  Serial.println("RecFromBits");
  Serial.write(xbeeRecBuf,xbeeRecBufSize);

  if(strstr((char*)xbeeRecBuf,"gndtest")){ //Checks if "test" is within buffer
      Serial.println("");
      Serial.println("ackTest");
      String("PacketAck").getBytes(xbeeSendBuf,xbeeSendBufSize);
      xbeeSend(GroundSL,xbeeSendBuf);
  }
  if(strstr((char*)xbeeRecBuf,"TG")){ 
      Serial.println("");
      Serial.println("ToGround");
      String("ToGNDAck").getBytes(xbeeSendBuf,xbeeSendBufSize);
      xbeeSend(BitsSL,xbeeSendBuf);
  }
  if(strstr((char*)xbeeRecBuf,"terminate")){ 
      Serial.println("");
      Serial.println("Terminate");
      String("ToGNDAckTerm").getBytes(xbeeSendBuf,xbeeSendBufSize);
      xbeeSend(BitsSL,xbeeSendBuf);
      terminate();
  }
}

void processBlueMessage(){ //Just print things to the monitor
  Serial.println("RecFromBlue");
  Serial.write(xbeeRecBuf,xbeeRecBufSize);
}

void processGroundMessage(){
  Serial.print("RecFromGND: ");
  Serial.write(xbeeRecBuf,xbeeRecBufSize);
  
  if(strstr((char*)xbeeRecBuf,"gndtest")){
      Serial.println("");
      Serial.println("ackTest");
      String("PacketAck").getBytes(xbeeSendBuf,xbeeSendBufSize);
      xbeeSend(GroundSL,xbeeSendBuf);
  }
  if(strstr((char*)xbeeRecBuf,"TG")){ 
      Serial.println("");
      Serial.println("ToGround");
      String("ToGNDAck").getBytes(xbeeSendBuf,xbeeSendBufSize);
      xbeeSend(GroundSL,xbeeSendBuf);
  }
  if(strstr((char*)xbeeRecBuf,"terminate")){ 
      Serial.println("");
      Serial.println("Terminate");
      String("ToGNDAckTerm").getBytes(xbeeSendBuf,xbeeSendBufSize);
      xbeeSend(GroundSL,xbeeSendBuf);
      terminate();
  }
}

// Cutdown command
void terminate(){
  Serial.println("TERMINATING");
  // Trigger sets triggerPin LOW
  digitalWrite(triggerPin,LOW);

  // Send confirmation to ground XBee
  String("Terminated").getBytes(xbeeSendBuf,xbeeSendBufSize);
  xbeeSend(GroundSL,xbeeSendBuf);

  // Send confirmation via BITS
  String("ToGNDAckTerm").getBytes(xbeeSendBuf,xbeeSendBufSize);
  xbeeSend(BitsSL,xbeeSendBuf);
}
