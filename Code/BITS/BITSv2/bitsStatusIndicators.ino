//Advanced blinking
void startBlinks(){
//LED Indicators
  pinMode(4,OUTPUT);//Red
  pinMode(5,OUTPUT);//Green
  pinMode(13, OUTPUT);//BuiltIn
  digitalWrite(13, LOW);
  digitalWrite(4, LOW);
  digitalWrite(5, LOW);
  for (int i = 0; i < 10; i++)
  {
    Serial.println(i);
    delay(100);
    digitalWrite(13, HIGH);
    digitalWrite(4, HIGH);
    digitalWrite(5, HIGH);
    delay(25);
    digitalWrite(13, LOW);
    digitalWrite(4, LOW);
    digitalWrite(5, LOW);
  }
}

void pingBlink(){
  for (int i = 0; i < 20; i++)
  {
    delay(250);
    digitalWrite(13, HIGH);
    digitalWrite(5, HIGH);
    delay(250);
    digitalWrite(13, LOW);
    digitalWrite(5, LOW);
  }
}

void gpsLockBlink(){
  for (int i = 0; i < 5; i++)
  {
    delay(500);
    digitalWrite(5, HIGH);
    delay(250);
    digitalWrite(5, LOW);
  }
}

void pulseRed(){
    digitalWrite(4, HIGH);
    delay(250);
    digitalWrite(4,LOW);
}

void transmitBlink(){
  for (int i=0;i<3;i++){
    digitalWrite(5,HIGH);
    delay(100);
    digitalWrite(5,LOW);
    digitalWrite(4,HIGH);
    delay(100);
    digitalWrite(4,LOW);
  }
}
