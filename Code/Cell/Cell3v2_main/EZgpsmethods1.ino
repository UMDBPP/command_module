//EZgpsmethods for adding simple preconfigured gps magic -Jonathan

const char baudNineSix[] PROGMEM = {
  //new 9600
  0xB5,0x62,0x06,0x00,0x14,0x00,0x01,0x00,0x00,0x00,0xD0,0x08,0x00,0x00,0x80,0x25,0x00,0x00,0x07,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0xA2,0xB5
};
const char baudOneOneFive[] PROGMEM = {
  //new 115200
  0xB5,0x62,0x06,0x00,0x14,0x00,0x01,0x00,0x00,0x00,0xD0,0x08,0x00,0x00,0x00,0xC2,0x01,0x00,0x07,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0xC0,0x7E
};
//const char baudNineTwoOne[] PROGMEM = {
//  //new 921600 //This works, but is too quick for an Arduino Mega
//  0xB5,0x62,0x06,0x00,0x14,0x00,0x01,0x00,0x00,0x00,0xD0,0x08,0x00,0x00,0x00,0x10,0x0E,0x00,0x07,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x1B,0x5A  
//};

const char hexList[] PROGMEM = {
	 //RATES
  0xB5, 0x62, 0x06, 0x08 ,0x06 ,0x00 ,0xE8 ,0x03 ,0x01 ,0x00 ,0x01 ,0x00 ,0x01 ,0x39, //1Hz
  //0xB5 ,0x62 ,0x06 ,0x08 ,0x06 ,0x00 ,0xF4 ,0x01 ,0x01 ,0x00 ,0x01 ,0x00 ,0x0B ,0x77, //2Hz
  //0xB5 ,0x62 ,0x06 ,0x08 ,0x06 ,0x00 ,0xFA ,0x00 ,0x01 ,0x00 ,0x01 ,0x00 ,0x10 ,0x96, //4Hz
  //0xB5 ,0x62 ,0x06 ,0x08 ,0x06 ,0x00 ,0xC8 ,0x00 ,0x01 ,0x00 ,0x01 ,0x00 ,0xDE ,0x6A, //5Hz
  //0xB5 ,0x62 ,0x06 ,0x08 ,0x06 ,0x00 ,0x64 ,0x00 ,0x01 ,0x00 ,0x01 ,0x00 ,0x7A ,0x12, //10Hz
      
      //BAUD might be in need of work, test new
     //0xB5,0x62,0x06,0x00,0x14,0x00,0x01,0x00,0x00,0x00,0xD0,0x08,0x00,0x00,0x80,0x25,0x00,0x00,0x07,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0xA2,0xB5, //9600
     //0xB5 ,0x62 ,0x06 ,0x00 ,0x14 ,0x00 ,0x01 ,0x00 ,0x00 ,0x00 ,0xD0 ,0x08 ,0x00 ,0x00 ,0x00 ,0xC2 ,0x01 ,0x00 ,0x07 ,0x00 ,0x03 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0xC0 ,0x7E, //115200
  
  
  //NAV5 AIRBORNE >1g
  //NEW
  0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4D, 0xDB,
  //0xB5 ,0x62 ,0x06 ,0x24 ,0x24 ,0x00 ,0xFF ,0xFF ,0x08 ,0x03 ,0x00 ,0x00 ,0x00 ,0x00 ,0x10 ,0x27 ,0x00 ,0x00 ,0x05 ,0x00 ,0xFA ,0x00 ,0xFA ,0x00 ,0x64 ,0x00 ,0x2C ,0x01 ,0x00 ,0x3C ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x54 ,0x2C,
  //SAVE
  0xB5 ,0x62 ,0x06 ,0x09 ,0x0D ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0xFF ,0xFF ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x03 ,0x1D ,0xAB
};

void GPSINIT(int baud){
	switch(baud){
	  case 9600:
        gpsserial.begin(115200);
        for(int i = 0;i<sizeof(hexList);i++){
          gpsserial.write(pgm_read_byte(baudNineSix+i));
          //delay(10);
        }
        gpsserial.end();
        gpsserial.begin(9600);
      break;
//    case 115200:
//        gpsserial.begin(9600);
//        for(int i = 0;i<sizeof(hexList);i++){
//          gpsserial.write(pgm_read_byte(baudOneOneFive+i));
//          delay(10);
//        }
//        gpsserial.end();
//        gpsserial.begin(115200);
//      break;
//    case 921600:
//          for(int i = 0;i<sizeof(hexList);i++){
//            gpsserial.write(pgm_read_byte(baudNineTwoOne+i));
//            delay(10);
//          }
//      break;
    default:
    Serial.println(F("GPSINIT_ERR"));
      break;
	}
	for(int i = 0;i<sizeof(hexList);i++){
		gpsserial.write(pgm_read_byte(hexList+i));
		//delay(10);
	}
 gpsserial.write("");
}

void gpsRun(){
  //preserve = gpsInfo;
  while (gpsserial.available()){
   if (gps.encode(gpsserial.read())){
   gpsInfo = getGPS();
     break;
   }
 }
}

GPSdata getGPS(){
  GPSdata gpsInfo;
  
    float GPSLat, GPSLon;
    int GPSSats;
    long GPSAlt;
    unsigned long date,fix_age,GPSTime, GPSSpeed,GPSCourse;
    
    gps.f_get_position(&GPSLat, &GPSLon, &fix_age);
    GPSSats = gps.satellites();
    gps.get_datetime(&date, &GPSTime, &fix_age);
    GPSAlt = gps.altitude()/100.;
    GPSSpeed = gps.f_speed_mps();
    GPSCourse = gps.course();

    gpsInfo.GPSLat = GPSLat;
    gpsInfo.GPSLon = GPSLon;
    gpsInfo.GPSTime = GPSTime/100;
    gpsInfo.GPSSats = GPSSats;
    gpsInfo.GPSAlt = GPSAlt;
    gpsInfo.GPSSpeed = GPSSpeed;
    gpsInfo.GPSCourse = GPSCourse;
  
  return gpsInfo;
}

//void outputSerial(){
//  String gpspacket;
//  if(gpsInfo.GPSSats!=-1){
//    gpspacket = String(gpsInfo.GPSTime)+","+String(gpsInfo.GPSLat,4) + "," + String(gpsInfo.GPSLon,4)+","+gpsInfo.GPSAlt;
//  }else{
//    //gpspacket = String(preserve.GPSTime/100)+","+String(preserve.GPSLat,6) + "," + String(preserve.GPSLon,6)+","+preserve.GPSAlt+","+preserve.GPSSats;
//    gpspacket = "err";
//  }
//  Serial.println(gpspacket);
//}


void outputSerial(){
  char gpsBuf[50];
  memset(gpsBuf,0,50);
  
  if(gpsInfo.GPSSats!=-1){
    //gpspacket = String(gpsInfo.GPSTime)+","+String(gpsInfo.GPSLat,4) + "," + String(gpsInfo.GPSLon,4)+","+gpsInfo.GPSAlt;
    char bypasslat[20];
    char bypasslon[10];
    dtostrf(gpsInfo.GPSLat,7,4,bypasslat);
    dtostrf(gpsInfo.GPSLon,8,4,bypasslon);
    strncat(bypasslat,bypasslon,10);
    snprintf(gpsBuf,50,"%2.6u%s,%s,%u",gpsInfo.GPSTime,bypasslat,gpsInfo.GPSAlt);
  }else{
    //gpsBuf = "err";
    snprintf(gpsBuf,50,"err");
  }

  
  Serial.println(gpsBuf);
}

void sendText(){
  char gpsBuf[50];
  memset(gpsBuf,0,50);
  
  //String gpspacket;
  if(gpsInfo.GPSSats!=-1){
    //gpspacket = String(gpsInfo.GPSTime)+","+String(gpsInfo.GPSLat,4) + "," + String(gpsInfo.GPSLon,4)+","+gpsInfo.GPSAlt;
    char bypasslat[20];
    char bypasslon[10];
    dtostrf(gpsInfo.GPSLat,7,4,bypasslat);
    dtostrf(gpsInfo.GPSLon,8,4,bypasslon);
    strncat(bypasslat,bypasslon,10);
    snprintf(gpsBuf,50,"%2.6u%s,%s,%u",gpsInfo.GPSTime,bypasslat,gpsInfo.GPSAlt);
  }else{
    //gpsBuf = "err";
    snprintf(gpsBuf,50,"err");
  }
  
  //gpspacket.toCharArray(packetData,50);
  // Generate outputData containing packetData to be txSMS to phonenumber
  char outputData[80];
  memset(outputData, 0, 80);
  celltracker.txSMS(phonenumber, gpsBuf, outputData);
  // Send outputData to XBee
  for(int i = 0; i < 80; i++){
    xbeeSerial.write(outputData[i]);
    Serial.println((int)outputData[i]);
  }
  celltracker.nukeBuffer();
}

void outputSD(){
  String gpspacket;
  if(gpsInfo.GPSSats!=-1){
    gpspacket = String(gpsInfo.GPSTime)+","+String(gpsInfo.GPSLat,4) + "," + String(gpsInfo.GPSLon,4)+","+gpsInfo.GPSAlt;
  }else{
    //gpspacket = String(preserve.GPSTime/100)+","+String(preserve.GPSLat,6) + "," + String(preserve.GPSLon,6)+","+preserve.GPSAlt+","+preserve.GPSSats;
    gpspacket = "err";
  }
  GPSlog.println(gpspacket);
  GPSlog.flush();
}
