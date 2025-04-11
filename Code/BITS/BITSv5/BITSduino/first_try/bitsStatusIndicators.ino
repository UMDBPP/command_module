// Advanced blinking
void startBlinks() {
  // LED Indicators
  // pinMode(redLED,OUTPUT);//Red
  pinMode(LED_PIN, OUTPUT); // Green
  digitalWrite(4, LOW);
  digitalWrite(5, LOW);
  for (int i = 0; i < 10; i++) {
    Serial.println(i);
    delay(100);
    // digitalWrite(redLED, HIGH);
    digitalWrite(LED_PIN, HIGH);
    delay(25);
    // digitalWrite(redLED, LOW);
    digitalWrite(LED_PIN, LOW);
  }
}

void pingBlink() {
  for (int i = 0; i < 20; i++) {
    delay(250);
    digitalWrite(LED_PIN, HIGH);
    delay(250);
    digitalWrite(LED_PIN, LOW);
  }
}

void gpsLockBlink() {
  for (int i = 0; i < 5; i++) {
    delay(500);
    digitalWrite(LED_PIN, HIGH);
    delay(250);
    digitalWrite(LED_PIN, LOW);
  }
}

void transmitBlink() {
  for (int i = 0; i < 3; i++) {
    // digitalWrite(redLED,HIGH);
    // delay(100);
    // digitalWrite(redLED,LOW);
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
  }
}
