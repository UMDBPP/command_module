//Advanced blinking
void startBlinks(){
//LED Indicators
  pinMode(redLED,OUTPUT);//Red
  pinMode(greenLED,OUTPUT);//Green
  digitalWrite(4, LOW);
  digitalWrite(5, LOW);
  for (int i = 0; i < 10; i++)
  {
    Serial.println(i);
    delay(100);
    digitalWrite(redLED, HIGH);
    digitalWrite(greenLED, HIGH);
    delay(25);
    digitalWrite(redLED, LOW);
    digitalWrite(greenLED, LOW);
  }
}

void pingBlink(){
  for (int i = 0; i < 20; i++)
  {
    delay(250);
    digitalWrite(redLED, HIGH);
    delay(250);
    digitalWrite(redLED, LOW);
  }
}

void gpsLockBlink(){
  for (int i = 0; i < 5; i++)
  {
    delay(500);
    digitalWrite(greenLED, HIGH);
    delay(250);
    digitalWrite(greenLED, LOW);
  }
}

void pulseRed(){
    digitalWrite(redLED, HIGH);
    delay(250);
    digitalWrite(redLED,LOW);
}

void transmitBlink(){
  for (int i=0;i<3;i++){
    digitalWrite(redLED,HIGH);
    delay(100);
    digitalWrite(redLED,LOW);
    digitalWrite(greenLED,HIGH);
    delay(100);
    digitalWrite(greenLED,LOW);
  }
}
