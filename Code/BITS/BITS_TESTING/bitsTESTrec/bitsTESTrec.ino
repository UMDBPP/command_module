uint8_t rxBuf[49];//RX BUFFER
int arm_status;//armed when = 42

void setup() {
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println("COMMAND");
  for(int i=0;i<20;i++){ 
    while(true){
      if(Serial.available()>1){
        rxBuf[i] = Serial.read();
        break;
       }
       delay(100);
    }
    Serial.print(i);
  }
  Serial.println("");
  
  Serial.println("OUTPUT-------");
  uplink();

    for(int i = 0;i<20;i++){ 
      Serial.write(rxBuf[i]);
    }
  delay(10000);
  Serial.println("");
  Serial.println("NEWLINE-------");
  for(int i = 0;i<20;i++){ 
    rxBuf[i] = 'o';
  }
}

void uplink(){
    //char str[49] = rxBuf;
    if(strstr((char*)rxBuf,"disable")){
        //pingBlink();
        arm_status = 0;
        Serial.println("Payload Disarmed");
        //logprintln("Payload Disarmed");
    }else if(strstr((char*)rxBuf,"arm")){
        //pingBlink();
        arm_status = 42;
        Serial.println("Payload Armed");
        //logprintln("Payload Armed");    
    }else if(strstr((char*)rxBuf,"drop")){
        //pingBlink();
        if(arm_status==42){
          Serial.println("DROP");
          //logprintln("DROP");
        }else{
          //logprintln("UNARMED_DROP_ATTEMPT");
          Serial.println("UNARMED_DROP_ATTEMPT");  
        }
    }else if(strstr((char*)rxBuf,"test")){
        Serial.println("TEST_SUCCESS");
        //logprintln("TEST_PASS");
    }else if(strstr((char*)rxBuf,"setrate")){
        if(strstr((char*)rxBuf,"fast")){
          Serial.println("SET_RATE_FAST");
          //logprintln("SET_RATE_FAST");
        }
        else if(strstr((char*)rxBuf,"norm")){
          Serial.println("SET_RATE_NORM");
          //logprintln("SET_RATE_NORM");
        }
    }
}
