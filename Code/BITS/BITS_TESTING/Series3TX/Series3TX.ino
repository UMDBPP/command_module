#include <XBee.h>
#include <SoftwareSerial.h>

XBee xbee = XBee();
SoftwareSerial xbeeSerial(2, 3);

uint8_t payload[] = { 't', 'e' , 's', 't'};

// SH + SL Address of receiving XBee
XBeeAddress64 addr64 = XBeeAddress64(0x0013A200, 0x417B4A3B);
ZBTxRequest zbTx = ZBTxRequest(addr64, payload, sizeof(payload));
ZBTxStatusResponse txStatus = ZBTxStatusResponse();
const uint32_t BITSSL = 0x417B4A3B;
ZBRxResponse rx = ZBRxResponse();
ModemStatusResponse msr = ModemStatusResponse();
uint8_t xbeeBuf[250];


void setup() {
  Serial.begin(9600);
  xbeeSerial.begin(9600);
  xbee.setSerial(xbeeSerial);
  Serial.println("Startup");
}

void loop() {

  xbee.send(zbTx);

  // after sending a tx request, we expect a status response
  // wait up to half second for the status response
  if (xbee.readPacket(500)) {
    // got a response!
    Serial.println("THINGS!");

    // should be a znet tx status
    if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {
      xbee.getResponse().getZBTxStatusResponse(txStatus);

      // get the delivery status, the fifth byte
      if (txStatus.getDeliveryStatus() == SUCCESS) {
        Serial.println("EvenMore");
        // success.  time to celebrate
      } else {
        Serial.println("NOPE");
      }
    }
  } else if (xbee.getResponse().isError()) {
    Serial.print("Error reading packet.  Error code: ");
    Serial.println(xbee.getResponse().getErrorCode());
  } else {
    // local XBee did not provide a timely TX Status Response -- should not happen
  }
  delay(5000);

  BITSMAIN();
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
        Serial.println(incominglsb,HEX);
        int incomingLength = rx.getPacketLength();
        memset(xbeeBuf, 0, 250); // Clears old buffer
        memcpy(xbeeBuf,rx.getData(),rx.getPacketLength());
        if(incominglsb == BITSSL){
          processBITSMessage();
        }
        /**
        if(incominglsb == TardisSL){
          processTardisMessage();
        }else if(incominglsb == MarsSL){
          processMarsMessage();
        }else if(incominglsb == GroundSL){
          processGroundMessage();
        }
        */
      }
    } else if (xbee.getResponse().isError()) {
      Serial.print("error code:");
      Serial.println(xbee.getResponse().getErrorCode());
    }         
}
void processBITSMessage(){
  Serial.println("BITS ACK");  
}
