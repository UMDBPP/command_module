//Everything related to ground to BITS commands
void uplink(){
    if(strstr((char*)rxBuf,"disarm")){
        pingBlink();
        arm_status = 0;
        OutputSerial.println("Payload Disarmed");
        logprintln("Payload Disarmed");
    }else if(strstr((char*)rxBuf,"arm")){
        pingBlink();
        arm_status = 42;
        OutputSerial.println("Payload Armed");
        logprintln("Payload Armed");    
    }else if(strstr((char*)rxBuf,"drop")){
        pingBlink();
        if(arm_status==42){
          OutputSerial.println("DROP");
          logprintln("DROP");
        }else{
          logprintln("UNARMED_DROP_ATTEMPT");
          OutputSerial.println("UNARMED_DROP_ATTEMPT");  
        }
    }else if(strstr((char*)rxBuf,"test")){
        OutputSerial.println("TEST_SUCCESS");
        logprintln("TEST_PASS");
        downlinkData = true;
        strcat(downlinkMessage2,"pass");
    }else if(strstr((char*)rxBuf,"setrate")){ //Change SBD message frequency
        if(strstr((char*)rxBuf,"fast")){      //For testing / accurate drops
          OutputSerial.println("SET_RATE_FAST");
          logprintln("SET_RATE_FAST");
          messageTimeInterval = 60000;// 1 minute
        }
        else if(strstr((char*)rxBuf,"norm")){
          OutputSerial.println("SET_RATE_NORM");
          logprintln("SET_RATE_NORM");
          messageTimeInterval = 300000; //5 minutes
        }else if(strstr((char*)rxBuf,"slow")){
          OutputSerial.println("SET_RATE_SLOW");
          logprintln("SET_RATE_SLOW");
          messageTimeInterval = 900000; //15 minutes
        }
    }
    if(strstr((char*)rxBuf,"xbeetest")){
        OutputSerial.println("PingXbee");
        logprintln("PingXbee");
        String("TestCommand").getBytes(xbeeSendBuf,xbeeSendBufSize);
        xbeeSend(GroundSL,xbeeSendBuf);
    }
    if(strstr((char*)rxBuf,"HELIOS")){
      OutputSerial.println("HELIOS");
      logprintln("HELIOS");
      strcat((char*)xbeeSendBuf,(char*)rxBuf);
      xbeeSend(HeliosSL,xbeeSendBuf);
      downlinkData = true;
      strcat(downlinkMessage2,"conf");
    }
}
