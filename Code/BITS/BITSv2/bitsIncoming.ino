// Everything related from ground to BITS commands
void uplink(){
    //---------------------------------------------ARMING SECTION -------------------------------------
    if(strstr((char*)rxBuf,"disarm"))
    {
        //pingBlink();
        arm_status = 0;
        OutputSerial.println("Payload Disarmed");
        logprintln("Payload Disarmed");
        downlinkData = true;
        //strcat(downlinkMessage2,"SAFED");
        strncat(downlinkMessage2,"SAFED",(downlinkMessageSize - strlen(downlinkMessage2) - 1));
        
    }else if(strstr((char*)rxBuf,"arm"))
    {
        //pingBlink();
        arm_status = 42;
        OutputSerial.println("Payload Armed");
        logprintln("Payload Armed");
        downlinkData = true;
        //strcat(downlinkMessage2,"ARMED");
        strncat(downlinkMessage2,"ARMED",(downlinkMessageSize - strlen(downlinkMessage2) - 1));
           
    }else if(strstr((char*)rxBuf,"drop"))
    {
        //pingBlink();
        if(arm_status == 42){
          OutputSerial.println("open");
          logprintln("DROP");
          String("open").getBytes(xbeeSendBuf,xbeeSendBufSize);
          xbeeSend(BlueSL,xbeeSendBuf);
        }else{
          logprintln("UNARMED_DROP_ATTEMPT");
          OutputSerial.println("UNARMED_DROP_ATTEMPT");  
          downlinkData = true;
          //strcat(downlinkMessage2,"NOT_ARMED");
          strncat(downlinkMessage2,"NOT_ARMED",(downlinkMessageSize - strlen(downlinkMessage2) - 1));
        }
    }
    //------------------------------------Close_Drop_Section--------------------------------------
    else if(strstr((char*)rxBuf,"test"))
    {
        OutputSerial.println("TEST_SUCCESS");
        logprintln("TEST_PASS");
        downlinkData = true;
        //strcat(downlinkMessage2,"test");
        strncat(downlinkMessage2,"test",(downlinkMessageSize - strlen(downlinkMessage2) - 1));
        
    }else if(strstr((char*)rxBuf,"setrate")) //Change SBD message frequency
    {
        if(strstr((char*)rxBuf,"fast")){      //For testing / accurate drops
          OutputSerial.println("SET_RATE_FAST");
          logprintln("SET_RATE_FAST");
          messageTimeInterval = 60000;// 1 minute
          downlinkData = true;
          //strcat(downlinkMessage2,",rF");
          strncat(downlinkMessage2,",rF",(downlinkMessageSize - strlen(downlinkMessage2) - 1));
        }
        else if(strstr((char*)rxBuf,"norm")){
          OutputSerial.println("SET_RATE_NORM");
          logprintln("SET_RATE_NORM");
          messageTimeInterval = 300000; //5 minutes
          downlinkData = true;
          //strcat(downlinkMessage2,",rN");
          strncat(downlinkMessage2,",rN",(downlinkMessageSize - strlen(downlinkMessage2) - 1));
        }else if(strstr((char*)rxBuf,"slow")){
          OutputSerial.println("SET_RATE_SLOW");
          logprintln("SET_RATE_SLOW");
          messageTimeInterval = 900000; //15 minutes
          downlinkData = true;
          //strcat(downlinkMessage2,",rS");
          strncat(downlinkMessage2,",rS",(downlinkMessageSize - strlen(downlinkMessage2) - 1));
        }
    }
    
    else if(strstr((char*)rxBuf,"xbeetest")){ //Sends test message to ground XBee
        OutputSerial.println("PingXbee");
        logprintln("PingXbee");
        String("TestCommand").getBytes(xbeeSendBuf,xbeeSendBufSize);
        xbeeSend(GroundSL,xbeeSendBuf);
    }
    else if(strstr((char*)rxBuf,"pingblue")){
        OutputSerial.println("pingMars");
        logprintln("pingMars");
        String("TestCommand").getBytes(xbeeSendBuf,xbeeSendBufSize);
        xbeeSend(BlueSL,xbeeSendBuf);
    }
    
    else if(strstr((char*)rxBuf,"BLUEPASS")){ //Blue passthrough
      OutputSerial.println("BluePass");
      logprintln("BluePass");
      strcat((char*)xbeeSendBuf,(char*)rxBuf);
      xbeeSend(BlueSL,xbeeSendBuf);
      downlinkData = true;
      //strcat(downlinkMessage2,"RecBlue");
      strncat(downlinkMessage2,",BluePass",(downlinkMessageSize - strlen(downlinkMessage2) - 1));
    }
    
    else if(strstr((char*)rxBuf,"GNDPASS")){ //Ground passthrough
      OutputSerial.println("GNDPASS");
      logprintln("GNDPASS");
      strcat((char*)xbeeSendBuf,(char*)rxBuf);
      xbeeSend(GroundSL,xbeeSendBuf);
      downlinkData = true;
      //strcat(downlinkMessage2,"recconf");
      strncat(downlinkMessage2,",GNDPass",(downlinkMessageSize - strlen(downlinkMessage2) - 1));
    }

    else if(strstr((char*)rxBuf,"WIREPASS")){ //Ground passthrough
      OutputSerial.println("WIREPASS");
      logprintln("WIREPASS");
      strcat((char*)xbeeSendBuf,(char*)rxBuf);
      xbeeSend(WireSL,xbeeSendBuf);
      downlinkData = true;
      //strcat(downlinkMessage2,"recconf");
      strncat(downlinkMessage2,",WirePass",(downlinkMessageSize - strlen(downlinkMessage2) - 1));
    }
}
