#include <SoftwareSerial.h>
SoftwareSerial xbeeSerial(8,9);


void setup() {
  Serial.begin(9600);
  xbeeSerial.begin(9600);
  Serial.print(0xA2);
  delay(1000);
}

void loop() {
  uint8_t packet[] = {'7','E', '0','0' ,'0','F', '0','0', '0','1', '0','0', '7','D' ,'3','3' ,'A','2' ,'0','0', '4','1', '7','B' ,'4','A' ,'3','B' ,'0','0', '7','4' ,'6','5', '7','3' ,'7','4' ,'4','8'};
  uint8_t packet2[] = {'7','E','0','0','0','F','0','0','0','1','0','0','7','D','3','3','A','2','0','0','4','1','7','B','4','A','3','B','0','0','7','4','6','5','7','3','7','4','4','8'};
  uint8_t packet3 = 0x7E000F0001007D33A200417B4A3B007465737448;
  uint8_t packet4[] = {0x7e, 0x20, 0x0f, 0x20, 0x01, 0x20, 0x7d, 0x33, 0xA2,0x20 ,0x41, 0x7b, 0x4a, 0x3b, 0x20, 0x74, 0x65, 0x73, 0x74, 0x48};
  for(int i =0;i<sizeof(packet4);i++){
        Serial.write(packet4[i]);
        delay(10);
  }
  //Serial.write(packet4);
  //Serial.println("PING");
  delay(1000);
}
