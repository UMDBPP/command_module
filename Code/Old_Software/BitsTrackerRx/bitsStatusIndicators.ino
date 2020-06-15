void startBlinks(){
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  for (int i = 0; i < 10; i++)
  {
    Serial.println(i);
    delay(950);
    digitalWrite(13, HIGH);
    delay(50);
    digitalWrite(13, LOW);
  }
}

void pingBlink(){
  for (int i = 0; i < 20; i++)
  {
    delay(250);
    digitalWrite(13, HIGH);
    delay(250);
    digitalWrite(13, LOW);
  }
}
