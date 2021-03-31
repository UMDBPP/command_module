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
File event_log;

int gps_index = 0;
int gps_comma = 0;
int gps_comma_lag = 0;

char gps_buffer[20];
const int outputDataSize = 100;
const int gpsBufSize = 50;


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
float gps_time = 123456.78;
float latitude = 0.0;
float longitude = 0.0;
int gps_quality = -1;
int gps_sats = 0;
int gps_alt = 1;
float gps_hdop = -1;


const unsigned int flush_interval = 10000;
const unsigned int log_interval = 1000;
const unsigned int text_interval = 300000;

unsigned long next_flush = 0;
unsigned long next_log = 0;
unsigned long next_text = 0;

int x = 0;

unsigned int max_gps_alt = 0;

char xbeeBuffer[100];
const int xbeeBufSize = 100;
jtXBee celltracker(xbeeBuffer, xbeeBufSize);

char phonenumber[12] = "7324849689";

void setup()
{
  xbeeSerial.begin(9600);
  
  Serial.begin(115200);

  // Hold for Serial initialization
  while(millis()<3000){}
  
  gps_init();

  sd_init();

  delay(500);


// Generate outputData containing packetData to be txSMS to phonenumber
  Serial.println(F("INIT_TEXT"));
  
  char packetData[6] = "init"; //payload 
  char outputData[outputDataSize];         //buffer
  memset(outputData, 0, outputDataSize);   //Nukebuffer
  
  if(!celltracker.txSMS(phonenumber, packetData, outputData, outputDataSize)){ //fill buffer with data
    Serial.println("packet_gen_fail");
  } else {
    Serial.println("init_msg_gen");
  }

  // Send outputData to XBee over xbeeSerial
  for(int i = 0; i < outputDataSize; i++){
    //xbeeSerial.write(outputData[i]);
    //Serial.println((int)outputData[i]);
  }
  
	
  
// Hold for lock
  unsigned int gps_hold_timer = 0;

//  while((gps_sats < 4)){
//    myGPS();
//    if(millis() > gps_hold_timer){
//      gps_hold_timer = millis() + 1000;
//      outputSerial();
//	    outputSD();
//      Serial.println("waiting");
//    }
//  }
  
  Serial.println("sats");
  Serial.println(gps_sats);
  
  memset(outputData, 0, outputDataSize);
  char lockMSG[10] = "got_lock";

  GPSlog.println(F("got_lock"));
  
  if(!celltracker.txSMS(phonenumber, lockMSG, outputData, outputDataSize)){
    Serial.println("packet_gen_fail");
  }

  for(int i = 0; i < outputDataSize; i++){
    //xbeeSerial.write(outputData[i]);
  }
  
  celltracker.nukeBuffer();

  Serial.println("EnterLoop");
  GPSlog.println(F("EnterLoop"));
}


// The frickin simplest loop() I've seen in a while
void loop() {

    //PARSE
    myGPS();

    handle_cell();

    //LOG
    if (millis() > next_log) {
      outputSD();
      outputSerial();
	  
		  next_log = millis() + log_interval;
    }

    //SMS_TX
    if((millis() > next_text) && (gps_alt<1000) && (x<50)){
      sendText();
      x++;
      Serial.println(F("SMS_TX"));
      GPSlog.println(F("SMS_TX"));
	    next_text = millis() + text_interval;
    }

}
//END LOOP

// ---------------------------------------------------- Methods ------------------------------------------------------- //

void sendText(){
	
	char gpsBuf[gpsBufSize];
	memset(gpsBuf,0,gpsBufSize);
	int string_size = snprintf(gpsBuf,gpsBufSize,"%\n09.2f,%6.5f,%6.5f,%d,%d,%0.2f",gps_time,latitude,longitude,gps_alt,gps_sats,gps_hdop);

	// Generate outputData containing packetData to be txSMS to phonenumber
	char outputData[outputDataSize];
	memset(outputData, 0, outputDataSize);
 
	if(celltracker.txSMS(phonenumber, gpsBuf, outputData, outputDataSize)){
	  
    for(int i = 0; i < outputDataSize; i++){ 
      xbeeSerial.write(outputData[i]);
      Serial.println((int)outputData[i]);
    }
    
	}else{
	  Serial.println("packet_gen_fail");
	}
 
}



void outputSD(){

	char gpsBuf[gpsBufSize];
	memset(gpsBuf,0,gpsBufSize);
  
	int string_size = snprintf(gpsBuf,gpsBufSize,"\n%09.2f,%6.5f,%6.5f,%d,%d,%0.2f",gps_time,latitude,longitude,gps_alt,gps_sats,gps_hdop);

  if(string_size < gpsBufSize){
	  GPSlog.write(gpsBuf, string_size+1);
  }else{
    event_log.println("gpsBuf_ovflw");  
  }
  
	if(millis() > next_flush){
		GPSlog.flush();
		next_flush = millis() + flush_interval;
	}
	
	if(gps_alt > max_gps_alt){
		max_gps_alt = gps_alt;
	}
}

void outputSerial(){
  
	char gpsBuf[gpsBufSize];
	memset(gpsBuf,0,gpsBufSize);
 
	snprintf(gpsBuf,gpsBufSize,"\n%09.2f,%6.5f,%6.5f,%d,%d,%0.2f",gps_time,latitude,longitude,gps_alt,gps_sats,gps_hdop);
	Serial.println(gpsBuf);
}


void sd_init(){
  
  if (!SD.begin(SDpin)) {return;}
  
  GPSlog = SD.open("gps.txt", FILE_WRITE);
  event_log = SD.open("event.txt", FILE_WRITE);
  
  if (GPSlog) {
    GPSlog.println(F("init")); //Kinda obvious tbh
  } else {return;}

  if (event_log) {
    event_log.println(F("init")); //Kinda obvious tbh
  } else {return;}
  
  Serial.println(F("sd init success"));
}

void handle_cell(){
  
  // Check if incoming data
  if(xbeeSerial.available()){
      celltracker.nukeBuffer();
      while(xbeeSerial.available()>0){
        celltracker.process(xbeeSerial.read());
      }
  }

  // If received a text
  if(celltracker.is_sms_ready()){
      
      char rxNumber[10];
      char rxData[20];
      memset(rxNumber,0,10);
      memset(rxData,0,20);
      
      if(celltracker.rxSMS(rxNumber, rxData, 20)){
        Serial.println("GoodRec");
      }

      // Print Number
      Serial.println("rxNumber");
      for(int i = 0; i < 10; i++){
        Serial.println((char)rxNumber[i]);  
      }

      // Print Data
      Serial.println("rxData");
      for(int i = 0; i < 20; i++){
        Serial.println((char)rxData[i]);  
      }

      if(strstr(rxNumber,phonenumber)){
        Serial.println("Got message from my phone");

        if(strstr(rxData,"start")){
          Serial.println("startTexts");
        }else if(strstr(rxData,"stop")){
          Serial.println("stopTexts");
        }
        
      }
  }  
}

