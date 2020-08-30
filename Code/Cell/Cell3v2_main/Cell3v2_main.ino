// Celltracker Code, for the medium old celltracker board v2
// Written by Shailesh and Jonathan along with the use of some public libraries
// Transmits its position over the cell network and logs position

// For use with the celltracker3.2 board

#include <SD.h>
#include <jtXBee.h>

#define gpsserial Serial1
#define xbeeSerial Serial3

#define SDpin 5

File GPSlog;

int gps_index = 0;
int gps_comma = 0;
int gps_comma_lag = 0;

char gps_buffer[20];


// Internal to be put into library
float _temp_pos = 0.0;
char _gps_type[6] = "nlock";
float _gps_time = 123456.78;
char _latitude_direction = '_';
char _longitude_direction = '_';
float _latitude = 0.0;
float _longitude = 0.0;
int _gps_quality = -1;
int _gps_sats = 0;
int _gps_alt = 1;
float _gps_hdop = -1;
int _checksum = 0;
bool _read_check = false;



// External from library
//char gps_type[6] = "nlock";
float gps_time = 123456.78;
//char latitude_direction = '_';
//char longitude_direction = '_';
float latitude = 0.0;
float longitude = 0.0;
int gps_quality = -1;
int gps_sats = 0;
int gps_alt = 1;
float gps_hdop = -1;


//snprintf(gpsBuf,100,"%09.2f,%6.5f,%6.5f,%d,%d,%0.2f ",gps_time,latitude,longitude,gps_alt,gps_sats,gps_hdop);

const unsigned long flush_interval = 10000;
const unsigned int log_interval = 200;
const unsigned int text_interval = 300000;

unsigned long next_flush = 0;
unsigned long next_log = 0;
unsigned long next_text = 0;

int x = 0;

int max_gps_alt = 0;

char xbeeBuffer[100];
const int bufSize = 100;
jtXBee celltracker(xbeeBuffer, bufSize);

char phonenumber[12] = "7324849689";

void setup()
{
  xbeeSerial.begin(9600);
  
  Serial.begin(115200);
  
  while(!Serial){}
  
  gps_init();

  Serial.println(F("INIT_TEXT"));

  // Generate outputData containing packetData to be txSMS to phonenumber
  char outputData[80];
  memset(outputData, 0, 80);
  char packetData[6] = "init";
  
  celltracker.txSMS(phonenumber, packetData, outputData);

  // Send outputData to XBee
  for(int i = 0; i < 80; i++){
    xbeeSerial.write(outputData[i]);
    Serial.println((int)outputData[i]);
  }
  celltracker.nukeBuffer();

  sd_init();

  delay(500);
	
	
  
  // Hold for lock
  unsigned int gps_hold_timer = 0;

  while((gps_sats < 4)){
    myGPS();
    if(millis() > gps_hold_timer){
      gps_hold_timer = millis() + 1000;
      outputSerial();
	    outputSD();
      Serial.println("waiting");
    }
  }
  Serial.println("sats");
  Serial.println(gps_sats);
  
  memset(outputData, 0, 80);
  char lockMSG[10] = "got_lock";
  
  celltracker.txSMS(phonenumber, lockMSG, outputData);

  for(int i = 0; i < 80; i++){
    xbeeSerial.write(outputData[i]);
  }
  celltracker.nukeBuffer();

  Serial.println("EnterLoop");
}


// The frickin simplest loop() I've seen in a while
void loop() {

    //PARSE
    myGPS();

    //LOG
    if (millis() > next_log) {
		  //Serial.println("LOGBLPUIISIDSF");
      outputSD();
      outputSerial();
	  
		  next_log = millis() + log_interval;
    }

    //SMS_TX
    if((millis() > next_text) && (gps_alt<1000) && (x<50)){
      sendText();
      x++;
      Serial.println(F("SMS_TX"));
	    next_text = millis() + text_interval;
    }

}

void sendText(){
	
	char gpsBuf[100];
	memset(gpsBuf,0,100);
	snprintf(gpsBuf,100,"%09.2f,%6.5f,%6.5f,%d,%d,%0.2f",gps_time,latitude,longitude,gps_alt,gps_sats,gps_hdop);


	// Generate outputData containing packetData to be txSMS to phonenumber
	char outputData[100];
	memset(outputData, 0, 100);
	celltracker.txSMS(phonenumber, gpsBuf, outputData);
	// Send outputData to XBee
	for(int i = 0; i < 100; i++){
		xbeeSerial.write(outputData[i]);
		Serial.println((int)outputData[i]);
	}
	celltracker.nukeBuffer();
}


void outputSD(){

	char gpsBuf[80];
	memset(gpsBuf,0,80);
	snprintf(gpsBuf,80,"%09.2f,%6.5f,%6.5f,%d,%d,%0.2f\n ",gps_time,latitude,longitude,gps_alt,gps_sats,gps_hdop);
	GPSlog.write(gpsBuf,80);
	
	if(millis() > next_flush){
		GPSlog.flush();
		next_flush = millis() + flush_interval;
	}
	
	if(gps_alt > max_gps_alt){
		max_gps_alt = gps_alt;
	}
}

void sd_init(){
	if (!SD.begin(SDpin)) {
		Serial.println(F("SD init fail"));
		return;
	}else{
		Serial.println(F("SD init"));
	}
	GPSlog = SD.open("log.txt", FILE_WRITE);
	if (GPSlog) {
		Serial.println(F("log init"));
		GPSlog.println(F("Started")); //Kinda obvious tbh
	} else {
		Serial.println(F("log init err"));
	}
}



void outputSerial(){
	char gpsBuf[100];
	memset(gpsBuf,0,100);
	snprintf(gpsBuf,100,"%09.2f,%6.5f,%6.5f,%d,%d,%0.2f",gps_time,latitude,longitude,gps_alt,gps_sats,gps_hdop);
	Serial.println(gpsBuf);
}
