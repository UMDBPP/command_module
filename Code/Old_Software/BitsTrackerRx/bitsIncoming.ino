void uplink(){
    //char str[49] = rxBuf;
    if(strstr((char*)rxBuf,"release")){
        pingBlink();
        OutputSerial.println("RELEASE THE KRAKEN");
        logprintln("RELEASE COMMAND DETECTED");
    }
    if(strstr((char*)rxBuf,"test")){
        OutputSerial.println("TEST_SUCCESS");
        logprintln("TEST_PASS");
    }
}
