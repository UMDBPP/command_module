#include <XBee.h>
#include <SoftwareSerial.h>

XBee xbee = XBee();
XBeeResponse response = XBeeResponse();
//SoftwareSerial xbeeSerial(2, 3);
#define xbeeSerial Serial2


const uint32_t BitsSL = 0x417B4A3B;
const uint32_t GroundSL = 0x417B4A36;
const uint32_t MarsSL = 0x417B4A3A;
const uint32_t UniSH = 0x0013A200;

ZBTxStatusResponse txStatus = ZBTxStatusResponse();
ZBRxResponse rx = ZBRxResponse();
ModemStatusResponse msr = ModemStatusResponse();
const int xbeeRecBufSize = 50; //Rec must be ~15bytes larger than send
const int xbeeSendBufSize = 35;
uint8_t xbeeRecBuf[xbeeRecBufSize];
uint8_t xbeeSendBuf[xbeeSendBufSize];


void setup() {
  Serial.begin(9600);
  xbeeSerial.begin(9600);
  xbee.setSerial(xbeeSerial);
  Serial.println("Startup");
}

void loop() {
  if(Serial.available()>0){
    Serial.print("Sending: ");
    Serial.readBytes((char*)xbeeSendBuf,xbeeSendBufSize);
    xbeeSend(GroundSL,xbeeSendBuf);
    //Serial.write(xbeeSendBuf,25);
    //Serial.println("");
  }
  
  xbeeRead();
}

bool xbeeSend(uint32_t TargetSL,uint8_t* payload){
  XBeeAddress64 TargetAddress = XBeeAddress64(UniSH,TargetSL);
  ZBTxRequest zbTx = ZBTxRequest(TargetAddress, payload, xbeeSendBufSize); //Assembles Packe
  xbee.send(zbTx);              //Sends packet
  memset(xbeeSendBuf, 0, xbeeSendBufSize);
  if (xbee.readPacket(500)) {                                       //Checks Reception
    if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {
      xbee.getResponse().getZBTxStatusResponse(txStatus);
      if (txStatus.getDeliveryStatus() == SUCCESS) {
        Serial.println("SuccessfulTransmit");
        return true;
      } else {
        Serial.println("TxFail");
        return false;
      }
    }
  } else if (xbee.getResponse().isError()) {
    Serial.print("Error reading packet.  Error code: ");
    Serial.println(xbee.getResponse().getErrorCode());
  }
  return false;
}

void xbeeRead(){
  xbee.readPacket(); //read buffer
    if (xbee.getResponse().isAvailable()) { //got something
      if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) { //got a TxRequestPacket
        ZBRxResponse rx = ZBRxResponse();
        xbee.getResponse().getZBRxResponse(rx);
        
        uint32_t incominglsb = rx.getRemoteAddress64().getLsb();
        Serial.print("Incoming Packet From: ");
        Serial.println(incominglsb,HEX);
        if(rx.getPacketLength()>=xbeeRecBufSize){
          Serial.print("Oversized Message: ");
          Serial.println(rx.getPacketLength());
        }
        memset(xbeeRecBuf, 0, xbeeRecBufSize); // Clears old buffer
        memcpy(xbeeRecBuf,rx.getData(),rx.getPacketLength());
        if(incominglsb == BitsSL){
          processBitsMessage();
        }
        if(incominglsb == MarsSL){
          processMarsMessage();
        }
        if(incominglsb == GroundSL){
          processGroundMessage();
        }    
      }
    } //else if (xbee.getResponse().isError()) {
      //Serial.print("error code:");
      //Serial.println(xbee.getResponse().getErrorCode());
    //}
}

void processBitsMessage(){
  Serial.println("RecFromBits");
  Serial.write(xbeeRecBuf,xbeeRecBufSize); 
  Serial.println(""); 
}
void processGroundMessage(){
  Serial.println("RecFromGND");
  Serial.write(xbeeRecBuf,xbeeRecBufSize); 
  Serial.println("");  
}
void processMarsMessage(){
  Serial.println("RecFromGND");  
}
