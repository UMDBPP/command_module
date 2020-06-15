void setup() {
  char sbd_buf[49];
  uint8_t go[49];
  unsigned int GPSTime = 182412;
  float GPSLat = 39.6967;
  float GPSLon = -78.1958;
  unsigned int GPSAlt = 168;
  String Packet = String(GPSTime) + ","+String(GPSLat,4)+","+String(GPSLon,4)+","+String(GPSAlt);
  Packet.toCharArray(sbd_buf,49);
  Serial.begin(9600);
  Serial.println(sbd_buf);
  Serial.println("ONE");
  Serial.write((char*)((uint8_t*)atoi(sbd_buf)));
  go = ((uint8_t*)atoi(sbd_buf));
  printf("This is a %go",go);
  Serial.println("TWO");
}

void loop() {
delay(10000);
}
