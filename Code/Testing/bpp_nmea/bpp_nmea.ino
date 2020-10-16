// 

#define gpsserial Serial1

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

unsigned long next_flush = 0;
unsigned long next_log = 0;

int x = 0;

unsigned int max_gps_alt = 0;



void setup()
{
  Serial.begin(115200);

  // Hold for Serial initialization
  while(millis()<3000){}
  
  gps_init();

  delay(500);
	
  
// Hold for lock
  unsigned int gps_hold_timer = 0;

  while((gps_sats < 4)){
    myGPS();
    if(millis() > gps_hold_timer){
      gps_hold_timer = millis() + 1000;
      outputSerial();
      Serial.println("waiting");
    }
  }
  
  Serial.println("sats");
  Serial.println(gps_sats);

  
  Serial.println("EnterLoop");
}


// The frickin simplest loop() I've seen in a while
void loop() {

    //PARSE
    myGPS();

    //LOG
    if (millis() > next_log) {
      outputSerial();
	  
		  next_log = millis() + log_interval;
    }

}
//END LOOP

// ---------------------------------------------------- Methods ------------------------------------------------------- //

void outputSerial(){
  
	char gpsBuf[gpsBufSize];
	memset(gpsBuf,0,gpsBufSize);
 
	snprintf(gpsBuf,gpsBufSize,"\n%09.2f,%6.5f,%6.5f,%d,%d,%0.2f",gps_time,latitude,longitude,gps_alt,gps_sats,gps_hdop);
	Serial.println(gpsBuf);
}
