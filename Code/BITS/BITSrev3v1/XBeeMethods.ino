//This file is for storing XBee methods used for communication via the XBee

//Transmit
bool xbeeSend(uint32_t TargetSL,uint8_t* payload){
  XBeeAddress64 TargetAddress = XBeeAddress64(UniSH,TargetSL);
  ZBTxRequest zbTx = ZBTxRequest(TargetAddress, payload, xbeeSendBufSize); //Assembles Packet
  xbee.send(zbTx);              			//Sends packet
  memset(xbeeSendBuf, 0, xbeeSendBufSize);  //Clear tx_buf
  if (xbee.readPacket(500)) {               //Checks Reception for 500ms
    if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {
      xbee.getResponse().getZBTxStatusResponse(txStatus);
      if (txStatus.getDeliveryStatus() == SUCCESS) { 
        Serial.println("GoodTx");//Received ACK
        return true;
      } else {
        Serial.println("TxFail");//Missing ACK
        return false;
      }
    }
  } else if (xbee.getResponse().isError()) {
    Serial.print("PckErr");
    Serial.println(xbee.getResponse().getErrorCode());
  }
  return false;
}

//Process Incoming
void xbeeRead(){
  xbee.readPacket(); //read buffer
    if (xbee.getResponse().isAvailable()) { //got something
      if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) { //got a TxRequestPacket
        xbee.getResponse().getZBRxResponse(rx);
        uint32_t incominglsb = rx.getRemoteAddress64().getLsb(); //ID the message sender
        logprint("IncPckFrom ");
        logprintln32(incominglsb);
        if(rx.getPacketLength()>=xbeeRecBufSize){
          logprint("Oversized Message: ");
          logprintln(rx.getPacketLength());
        }
        memset(xbeeRecBuf, 0, xbeeRecBufSize); // Clears old buffer
        memcpy(xbeeRecBuf,rx.getData(),rx.getPacketLength());//Safely fills buffer
		
		//This separates out the actual methods used to interpret XBee data, preventing payloads from commanding themselves
		//As a sidenote, a payload commanding itself would invlove some fun AI, and probably also disaster
        if(incominglsb == BlueSL){
          processBlueMessage();
        }
        else if(incominglsb == GroundSL){
          processGroundMessage();
        }
        else if(incominglsb == WireSL){
          processWireMessage();
        }    
      }
    } 
}

// Begin the section with Individual process messages, this will probably become a new tab to avoid confusion
// Still the same tab, because what is good 2am software without bad organization

//Message From Ground
void processGroundMessage(){
  
  OutputSerial.println("RecGround");//DEBUG
  
  //Ping Pong Test
  if(strstr((char*)xbeeRecBuf,"ping")){ //Received "ping", attempt to "pong"
      OutputSerial.println("groundPingPong");
      logprintln("groundPingPong");
      String("pong").getBytes(xbeeSendBuf,xbeeSendBufSize);
      xbeeSend(GroundSL,xbeeSendBuf);
  }
  
  //To Ground Data Test
  if(strstr((char*)xbeeRecBuf,"TG")){ //If contains TG tag, commence
	  OutputSerial.println("TG");
      logprintln("TGgnd");
      String("TGgndReq").getBytes(xbeeSendBuf,xbeeSendBufSize);
      xbeeSend(GroundSL,xbeeSendBuf); //Let Ground XBee know request to TX via Iridium has been filed
      downlinkData = true;
      //strcat(downlinkMessage2,(char*)xbeeRecBuf); //NEVER EVER USE THIS
	  
	  //Append the message to Iridium's packet in a safe manner. Excess bits will be deleted
      strncat(downlinkMessage2,(char*)xbeeRecBuf,downlinkMessageSize - strlen(downlinkMessage2) - 1);
      logprint("GNDDownAppend");
      logprintln(downlinkMessage2);
  }
}

void processBlueMessage(){
  OutputSerial.println("RecBlue");
  logprintln("RecBlue");
  if(strstr((char*)xbeeRecBuf,"TG")){
      OutputSerial.println("TG_Blue");
      logprintln("TG_Blue");
      downlinkData = true;
      //strcat(downlinkMessage2,(char*)xbeeRecBuf);//DELETE on TEST
      strncat(downlinkMessage2,(char*)xbeeRecBuf,downlinkMessageSize - strlen(downlinkMessage2) - 1);
      logprint("BlueDownAppend");
      logprintln(downlinkMessage2);
      //concat(   target      ,     source      ,downlink_buffer_size - current_downlink_packet_size) //Prevents payloads from overflowing the downlink and nuking BITS
  }
  if(strstr((char*)xbeeRecBuf,"ping")){
      OutputSerial.println("BluePingPong");
      logprintln("BluePingPong");
      String("pong").getBytes(xbeeSendBuf,xbeeSendBufSize);
      xbeeSend(BlueSL,xbeeSendBuf);
  }
}

void processWireMessage(){
  OutputSerial.println("RecWire");
  logprintln("RecWire");
  if(strstr((char*)xbeeRecBuf,"TG")){
      OutputSerial.println("TG_Wire");
      logprintln("TG_Wire");
      downlinkData = true;
      //strcat(downlinkMessage2,(char*)xbeeRecBuf);//DELETE on TEST
      strncat(downlinkMessage2,(char*)xbeeRecBuf,downlinkMessageSize - strlen(downlinkMessage2) - 1);
      logprint("WireDownAppend");
      logprintln(downlinkMessage2);
      //concat(   target      ,     source      ,downlink_buffer_size - current_downlink_packet_size) //Prevents payloads from overflowing the downlink and nuking BITS
  }
  if(strstr((char*)xbeeRecBuf,"ping")){
      OutputSerial.println("WirePingPong");
      logprintln("WirePingPong");
      String("pong").getBytes(xbeeSendBuf,xbeeSendBufSize);
      xbeeSend(BlueSL,xbeeSendBuf);
  }
}
