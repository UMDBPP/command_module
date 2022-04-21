//Advanced blinking

void startBlinks(){
    pinMode(greenLED,OUTPUT);//Green
    for (int i = 0; i < 10; i++)
    {
        Serial.println(i);
        delay(500);
        digitalWrite(greenLED, HIGH);
        delay(250);
        digitalWrite(greenLED, LOW);
    }
}

void pingBlink(){
    for (int i = 0; i < 20; i++)
    {
        delay(250);
        digitalWrite(greenLED, HIGH);
        delay(250);
        digitalWrite(greenLED, LOW);
    }
}

void gpsLockBlink(){
    for (int i = 0; i < 5; i++)
    {
        delay(1000);
        digitalWrite(greenLED, HIGH);
        delay(500);
        digitalWrite(greenLED, LOW);
    }
}

void transmitBlink(){
    for (int i=0;i<3;i++){
        digitalWrite(greenLED,HIGH);
        delay(300);
        digitalWrite(greenLED,LOW);
    }
}
