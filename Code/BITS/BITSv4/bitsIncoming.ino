// Supporting uplink command for BITS

// Everything related from ground to BITS commands

void uplink(){
    //------------------------------------ARMING SECTION------------------------------------
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

    //------------------------------------TX_Rates--------------------------------------
    else if(strstr((char*)rxBuf,"setrate")) //Change SBD message frequency
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
        else if(strstr((char*)rxBuf,"land")){
          OutputSerial.println("SET_RATE_LANDED");
          logprintln("SET_RATE_SLOW");
          messageTimeInterval = 3600000; //1 hour
          downlinkData = true;
          //strcat(downlinkMessage2,",rS");
          strncat(downlinkMessage2,",rL",(downlinkMessageSize - strlen(downlinkMessage2) - 1));
        }
    }

    //------------------------------------Reset_GPS--------------------------------------
    else if(strstr((char*)rxBuf,"gps_reset")){ //Sends test message to ground XBee
        OutputSerial.println("gps_reset");
        logprintln("gps_reset");
        String("gps_reset").getBytes(xbeeSendBuf,xbeeSendBufSize);
        xbeeSend(GroundSL,xbeeSendBuf);
        gps_reset();
    }

    //------------------------------------Check_RX_Processing--------------------------------    
    else if(strstr((char*)rxBuf,"test"))
    {
        OutputSerial.println("TEST_SUCCESS");
        logprintln("TEST_PASS");
        downlinkData = true;
        //strcat(downlinkMessage2,"test");
        strncat(downlinkMessage2,"test",(downlinkMessageSize - strlen(downlinkMessage2) - 1));
        
    }
    
    //------------------------------------Check_GND--------------------------------------
    else if(strstr((char*)rxBuf,"gnd_test")){ //Sends test message to ground XBee
        OutputSerial.println("gnd_check");
        logprintln("gnd_check");
        String("gnd_check").getBytes(xbeeSendBuf,xbeeSendBufSize);
        xbeeSend(GroundSL,xbeeSendBuf);
    }

    //------------------------------------Check_BLUE--------------------------------------
    else if(strstr((char*)rxBuf,"blue_test")){
        OutputSerial.println("blue_check");
        logprintln("blue_check");
        String("blue_check").getBytes(xbeeSendBuf,xbeeSendBufSize);
        xbeeSend(BlueSL,xbeeSendBuf);
    }

    //------------------------------------Check_Wire--------------------------------------
    else if(strstr((char*)rxBuf,"blue_test")){
        OutputSerial.println("wire_check");
        logprintln("wire_check");
        String("wire_check").getBytes(xbeeSendBuf,xbeeSendBufSize);
        xbeeSend(BlueSL,xbeeSendBuf);
    }

    //------------------------------------PASSTHROUGH_SECTION--------------------------------------
    
    //Blue passthrough
    else if(strstr((char*)rxBuf,"BLUEPASS")){ 
      OutputSerial.println("BluePass");
      strcat((char*)xbeeSendBuf,(char*)rxBuf); // assemble passthrough packet
      char conf[15] = ",GND";
      
      if(xbeeSend(BlueSL,xbeeSendBuf))  //try to passthrough with xbee
      {      
        logprintln("BluePass");               // if success, log it
        strcat(conf,"PASS");
      }
      else if(xbeeSend(BlueSL,xbeeSendBuf)) //try again if failed
      {
        logprintln("BluePass"); 
        strcat(conf,"PASS");
      }
      else
      {
        logprintln("BluePassFail"); // if failed twice, log the fail // TODO write a retry system
        strcat(conf,"FAIL");
      }
      
      downlinkData = true;
      strncat(downlinkMessage2,",BluePass",(downlinkMessageSize - strlen(downlinkMessage2) - 1));
    }

    //Ground passthrough
    else if(strstr((char*)rxBuf,"GNDPASS")){ 
      OutputSerial.println("GNDPASS");
      strcat((char*)xbeeSendBuf,(char*)rxBuf);
      char conf[15] = ",GND";

      if(xbeeSend(GroundSL,xbeeSendBuf))  //try to passthrough with xbee
      {      
        logprintln("GNDPASS");               // if success, log it
        strcat(conf,"PASS");
      }
      else if(xbeeSend(GroundSL,xbeeSendBuf)) //try again if failed
      {
        logprintln("GNDPASS");
        strcat(conf,"PASS"); 
      }
      else
      {
        logprintln("GNDPASSFail"); // if failed twice, log the fail // TODO write a retry system
        strcat(conf,"FAIL");
      }
      
      downlinkData = true;
      //strcat(downlinkMessage2,"recconf");
      strncat(downlinkMessage2,conf,(downlinkMessageSize - strlen(downlinkMessage2) - 1));
    }

    //Wire passthrough
    else if(strstr((char*)rxBuf,"WIREPASS")){ 
      OutputSerial.println("WIREPASS");
      logprintln("WIREPASS");
      strcat((char*)xbeeSendBuf,(char*)rxBuf);
      char conf[15] = ",Wire";

      if(xbeeSend(WireSL,xbeeSendBuf))  //try to passthrough with xbee
      {      
        logprintln("WIREPASS");               // if success, log it
        strcat(conf,"PASS");
      }
      else if(xbeeSend(WireSL,xbeeSendBuf)) //try again if failed
      {
        logprintln("WIREPASS"); 
        strcat(conf,"PASS");
      }
      else if(xbeeSend(WireSL,xbeeSendBuf)) //try again if failed
      {
        logprintln("WIREPASS"); 
        strcat(conf,"PASS");
      }
      else if(xbeeSend(WireSL,xbeeSendBuf)) //try again if failed
      {
        logprintln("WIREPASS"); 
        strcat(conf,"PASS");
      }
      else
      {
        logprintln("WIREPASS_FAIL"); // if failed twice, log the fail // TODO write a retry system
        strcat(conf,"FAIL");
      }
      
      downlinkData = true;
      strncat(downlinkMessage2,conf,(downlinkMessageSize - strlen(downlinkMessage2) - 1));
    }

    //GHOUL Passthrough
    else if(strstr((char*)rxBuf,"GHOULPASS")){ 
      OutputSerial.println("GHOULPASS");
      logprintln("GHOULPASS");
      strcat((char*)xbeeSendBuf,(char*)rxBuf);
      char conf[15] = ",GHOUL";

      if(xbeeSend(GHOULSL,xbeeSendBuf))  //try to passthrough with xbee
      {      
        logprintln("GHOULPASS");               // if success, log it
        strcat(conf,"PASS");
      }
      else if(xbeeSend(GHOULSL,xbeeSendBuf)) //try again if failed
      {
        logprintln("GHOULPASS"); 
        strcat(conf,"PASS");
      }
      else if(xbeeSend(GHOULSL,xbeeSendBuf)) //try again if failed
      {
        logprintln("GHOULPASS"); 
        strcat(conf,"PASS");
      }
      else if(xbeeSend(GHOULSL,xbeeSendBuf)) //try again if failed
      {
        logprintln("GHOULPASS"); 
        strcat(conf,"PASS");
      }
      else
      {
        logprintln("GHOULPASS_FAIL"); // if failed twice, log the fail // TODO write a retry system
        strcat(conf,"FAIL");
      }
      
      downlinkData = true;
      strncat(downlinkMessage2,conf,(downlinkMessageSize - strlen(downlinkMessage2) - 1));
    }
}
