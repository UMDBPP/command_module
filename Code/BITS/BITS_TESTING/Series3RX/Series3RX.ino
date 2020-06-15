#include <XBee.h>
//#include <SoftwareSerial.h>

XBee xbee = XBee();
XBeeResponse response = XBeeResponse();
// create reusable response objects for responses we expect to handle 
ZBRxResponse rx = ZBRxResponse();
ModemStatusResponse msr = ModemStatusResponse();

//uint8_t ssRX = 9;
//uint8_t ssTX = 8;
//SoftwareSerial nss(ssRX, ssTX);
uint8_t xbeeBuf[250];
const uint32_t GroundSL = 0x417B4A36;
const uint32_t TardisSL = 0x417B4A3B;
const uint32_t MarsSL = 0x417B4A3B;



void setup() {  
  // start serial
  Serial.begin(9600);
  Serial2.begin(9600);
  xbee.setSerial(Serial2);
  Serial.println("Starting up!");
}

void loop() {

         BITSMAIN();
      
        //Serial.println("Got an rx packet!");
        //Serial.print("rxgetOption: ");
        //Serial.println(rx.getOption());    
        /**Broken
        if (rx.getOption() == ZB_PACKET_ACKNOWLEDGED) {
            // the sender got an ACK
            Serial.println("packet acknowledged");
        } else {
          Serial.println("packet not acknowledged");
        }
        */
       
       /** 
        Serial.print("checksum is ");
        Serial.println(rx.getChecksum(), HEX);

        Serial.print("packet length is ");
        Serial.println(rx.getPacketLength(), DEC);
        
         for (int i = 0; i < rx.getDataLength(); i++) {
          Serial.print("payload [");
          Serial.print(i, DEC);
          Serial.print("] is ");
          Serial.println(rx.getData()[i], HEX);
        }
        
       for (int i = 0; i < xbee.getResponse().getFrameDataLength(); i++) {
        Serial.print("frame data [");
        Serial.print(i, DEC);
        Serial.print("] is ");
        Serial.println(xbee.getResponse().getFrameData()[i], HEX);
       }
       */
      
}
 void BITSMAIN(){
    xbee.readPacket();
    if (xbee.getResponse().isAvailable()) {                  // got something          
      if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) { // got a zb rx packet
        xbee.getResponse().getZBRxResponse(rx);              // now fill our zb rx class

        uint32_t incominglsb = rx.getRemoteAddress64().getLsb();
        //logprint("ReceivedXbeePacketFrom");
        //logprintln32(incominglsb);
        Serial.print("ReceivedXbeePacketFrom: ");
        Serial.print(incominglsb,HEX);
        int incomingLength = rx.getPacketLength();
        memset(xbeeBuf, 0, 250); // Clears old buffer
        memcpy(xbeeBuf,rx.getData(),rx.getPacketLength());
        if(incominglsb == TardisSL){
          processTardisMessage();
        }else if(incominglsb == MarsSL){
          processMarsMessage();
        }else if(incominglsb == GroundSL){
          processGroundMessage();
        }
      }
    } else if (xbee.getResponse().isError()) {
      Serial.print("error code:");
      Serial.println(xbee.getResponse().getErrorCode());
    }         
}
void processGroundMessage(){
  Serial.print("Parse message: ");
  if(strstr((char*)xbeeBuf,"test")){
        Serial.println("GroundIsTesting");
  }
}
void processTardisMessage(){}
void processMarsMessage(){}
