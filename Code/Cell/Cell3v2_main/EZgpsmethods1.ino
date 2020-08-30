//----------------------------------------------------------------Configuration Hex------------------------------------------------------------------------------------
// This section contains the preconfigured arrays of hex to change GPS configurations. Could be partially calculated in advance, but no need

//-----------------------------------------Disable Unused Messages---(UBX-CFG-MSG)----------------------------------
void gps_nuke_messages(){

  const char nuke_msg[] PROGMEM = {
    0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x3F, //RMC
    0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2A, //GLL
    0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x31, //GSA
    0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x38, //GSV
    0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x46 //VTG
  };

  gps_write(nuke_msg,sizeof(nuke_msg));
}

void gps_set_GGA(){
  
  const char gga_msg[] PROGMEM = {
    0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x28
  };

  gps_write(gga_msg,sizeof(gga_msg));
}

//-------------------------------------------UpdateHz----- (UBX-CFG-RATE)-------------------------------------------

void set_gps_one_hertz(){
  const char one_hertz[] PROGMEM = {0xB5, 0x62, 0x06, 0x08 ,0x06 ,0x00 ,0xE8 ,0x03 ,0x01 ,0x00 ,0x01 ,0x00 ,0x01 ,0x39};

  gps_write(one_hertz,sizeof(one_hertz));
}

void set_gps_five_hertz(){
  const char five_hertz[] PROGMEM = {0xB5 ,0x62 ,0x06 ,0x08 ,0x06 ,0x00 ,0xC8 ,0x00 ,0x01 ,0x00 ,0x01 ,0x00 ,0xDE ,0x6A};

  gps_write(five_hertz,sizeof(five_hertz));
}

void set_gps_ten_hertz(){
  const char ten_hertz[] PROGMEM = {0xB5 ,0x62 ,0x06 ,0x08 ,0x06 ,0x00 ,0x64 ,0x00 ,0x01 ,0x00 ,0x01 ,0x00 ,0x7A ,0x12};

  gps_write(ten_hertz,sizeof(ten_hertz));
}

//-----------------------------------------------Baud----(UBX-CFG-PRT)---------------------------------------------

void gps_set_baud_default(){ //9600

  const char defaultBaud[] PROGMEM = {
    0xB5,0x62,0x06,0x00,0x14,0x00,0x01,0x00,0x00,0x00,0xD0,0x08,0x00,0x00,
    0x80,0x25,0x00,0x00,0x07,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0xA2,0xB5
  };
  gps_write(defaultBaud,sizeof(defaultBaud));
}

void gps_set_baud_fast(){ //115200
  
  const char quickBaud[] PROGMEM = {
    0xB5,0x62,0x06,0x00,0x14,0x00,0x01,0x00 ,0x00 ,0x00,0xD0,0x08,0x00,0x00,
    0x00,0xC2,0x01,0x00,0x07,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0xC0,0x7E
  };
  gps_write(quickBaud,sizeof(quickBaud));
}

void gps_set_baud_ultra(){
  
  const char ultraBaud[] PROGMEM = { //921600
    0xB5,0x62,0x06,0x00,0x14,0x00,0x01,0x00,0x00,0x00,0xD0,0x08,0x00,0x00,
    0x00,0x10,0x0E,0x00,0x07,0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x1B,0x5A
  };
  gps_write(ultraBaud,sizeof(ultraBaud));
}

//-------------------------------------------------Nav Mode---- (UBX-CFG-NAV5)----------------------------------------

bool gps_set_navmode_one_g(){
  
  const char navmode_one_g[] PROGMEM = {
    0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 
    0x64, 0x00, 0x2C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0xDC
  };

  const char navmode_poll[] PROGMEM = {0xB5, 0x62, 0x06, 0x24, 0x00, 0x00, 0x2A, 0x84};
  
  gps_write(navmode_one_g,sizeof(navmode_one_g));

  delay(1000);
  
  unsigned int stop_time = millis() + 5000;
  gps_write(navmode_poll,sizeof(navmode_poll));

  //---------------Parse UBX return ---------------
  bool rec_ubx_first = false;
  bool rec_ubx = false;
  int ubx_index = 2;

  //Sit in a loop until either a UBX packet is parsed or until a timeout is hit
  while(stop_time > millis()){
      if(gpsserial.available()){
        
        int parsed_char = gpsserial.read();
        
        if(rec_ubx){
          if(parsed_char == pgm_read_byte(navmode_one_g+ubx_index)){
            ubx_index++;
            if(ubx_index > 43){
              return true;
            }
          }else{
            return false;
          }
        }
        
        if(parsed_char == 181){
          //Serial.println("starting_cap");
          rec_ubx_first = true;
        }else if(parsed_char == 98 && rec_ubx_first == true){
          rec_ubx = true;
        }else{
          rec_ubx_first = false;  
        }

      }
  }
  return false; // Return after timeout
}

//-------Save------- (UBX-CFG-CFG)
void gps_save_config(){
  const char save_gps_conf[] PROGMEM = {
    0xB5,0x62,0x06,0x09,0x0D,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x1D,0xAB
  };

  gps_write(save_gps_conf,sizeof(save_gps_conf));
}

// ------------------------------------------ Initialize ------------------------------------------
void gps_init(){  

  //init GPS at proper baud
  gps_baud_init();
  
  // nuke uneeded NMEA msgs
  gps_nuke_messages();
  
  // Make sure the GxGGA msg is active
  gps_set_GGA();

  if(!gps_set_navmode_one_g()){
    Serial.println("navmode_conf_fail");
  }else{
    Serial.println("itworked");
  }

  set_gps_one_hertz();

  gps_save_config();
}

void gps_baud_init(){
    gpsserial.begin(9600);
	delay(50);
    gps_set_baud_fast();
    gpsserial.end();
    gpsserial.begin(115200);
	delay(100);
	//gps_set_baud_ultra();
	//gpsserial.end();
	//gpsserial.begin(921600);
	
    delay(100);
}

// Write the GPS hex code to the GPS port
void gps_write(const char* config_string, int char_length){
    //Serial.println(char_length);
    for(int i = 0; i<char_length;i++){
      gpsserial.write(pgm_read_byte(config_string+i));
    }
}
// ------------------------------------------------------------------------------------------------ RUN GPS--------------------------------------------------

// $GNGGA,062422.80,3859.72361,N,07655.84891,W,2,06,1.59,22.1,M,-34.7,M,,0000*47
// $GNGGA,061808.00,,,,,0,04,78.68,,,,,,0000*7A


void myGPS(){
  
  
  
  if(gpsserial.available()){
    while (gpsserial.available()){
      char parsed_byte = gpsserial.read();
        
      //Start of string
      if(parsed_byte == '$'){
        
        // Print Last MSG
        //char gpsBuf[100];
        //memset(gpsBuf,0,100);
        
        gps_comma_lag = 0;
        gps_index = -1;
        gps_comma = 0;
        _checksum = 0;
        _read_check = false;
		
      }
      else if(parsed_byte == '*'){
         //Serial.print("Check: ");
         //Serial.println(_checksum);
         _read_check = true;
		 gps_index = -1;
      }
      else if(parsed_byte == 13){
         //Serial.println("");
         char * pEnd;
		 if(_checksum == strtol(gps_buffer,&pEnd,16)){
			//Serial.println("Match");
			
			gps_time = _gps_time;
			latitude = _latitude;
			longitude = _longitude;
			gps_quality = _gps_quality;
			gps_sats = _gps_sats;
			gps_alt = _gps_alt;
			gps_hdop = _gps_hdop;
			
		 }
         _read_check = false;
      }
      else if(parsed_byte == ','){ //If Comma
        gps_comma++;
        gps_index = -1;
        _checksum = _checksum ^ parsed_byte;
      }
      else                         //Process Data
      {
        if(!_read_check){
          _checksum = _checksum ^ parsed_byte;  
        }
        
        switch (gps_comma)
        {
          
          case 0: // Build GNGGA
            break;
          
          case 1: // Build Time: 123456.20
            if(gps_comma_lag < gps_comma){ //Lock TYPE
              gps_comma_lag++;
              strcpy(_gps_type,gps_buffer);
              memset(gps_buffer,0,20);
            }
            break;
            
          case 2: // Build Latitude: DDmm.mmmmm
            if(gps_comma_lag < gps_comma){ //Lock Time
              gps_comma_lag++;
              _gps_time = atof(gps_buffer);
              memset(gps_buffer,0,20);
            }
            break;
            
          case 3: // Build lat dir: N
            if(gps_comma_lag < gps_comma){ //hold lat
              gps_comma_lag++;
              _temp_pos = atof(gps_buffer);
              clear_buf(gps_buffer,20);
            }
            break;
			
          case 4:
            if(gps_comma_lag < gps_comma){ //Lock lat
              gps_comma_lag++;
              _latitude_direction = gps_buffer[0];
              clear_buf(gps_buffer,20);
			  
      			  _latitude = int(_temp_pos/100) + (_temp_pos-int(_temp_pos/100)*100)/60;
      			  if(_latitude_direction == 'S'){
      				  _latitude = _latitude*-1;
      			  }
            }
            break;
			
          case 5:
            if(gps_comma_lag < gps_comma){ //hold lon
              gps_comma_lag++;
              _temp_pos = atof(gps_buffer);
              clear_buf(gps_buffer,20);
            }
            break;
			
          case 6:
            if(gps_comma_lag < gps_comma){ //Lock lon
              gps_comma_lag++;
              _longitude_direction = gps_buffer[0];
              clear_buf(gps_buffer,20);
			  
      			  _longitude = int(_temp_pos/100) + (_temp_pos-int(_temp_pos/100)*100)/60;
      			  if(_longitude_direction == 'W'){
      				  _longitude = _longitude*-1;
      			  }
            }
            break;
			
          case 7:
            if(gps_comma_lag < gps_comma){ //Lock quality
              gps_comma_lag++;
              _gps_quality = atoi(gps_buffer);
              clear_buf(gps_buffer,20);
            }
            break;
			
          case 8:
            if(gps_comma_lag < gps_comma){ //Lock SATS
              gps_comma_lag++;
              _gps_sats = atoi(gps_buffer);
              clear_buf(gps_buffer,20);
            }
            break;
			
          case 9:
            if(gps_comma_lag < gps_comma){ //Lock hdop
              gps_comma_lag++;
              _gps_hdop = atof(gps_buffer);
              clear_buf(gps_buffer,20);
            }
            break;
			
          case 10:
            if(gps_comma_lag < gps_comma){ //Lock ALT
              gps_comma_lag++;
              _gps_alt = atoi(gps_buffer);
              clear_buf(gps_buffer,20);
            }
            break;
			
          case 11:
            if(gps_comma_lag < gps_comma){ //Lock Unit
              gps_comma_lag++;
              //gps_alt = atoi(gps_buffer);
              //clear_buf(gps_buffer,20);
            }
            break;
			
          case 12:
            if(gps_comma_lag < gps_comma){ //Lock Alt Correction
              gps_comma_lag++;
              _gps_alt += atoi(gps_buffer);
              clear_buf(gps_buffer,20);
            }
            break;
			
          case 13: 
		        if(gps_comma_lag < gps_comma){ // Lock Alt Correction Unit
              gps_comma_lag++;
            }
            break;
			
          case 14:
            if(gps_comma_lag < gps_comma){ // Lcok diffAge
              gps_comma_lag++;
            }
            break;

          case 15:
            if(gps_comma_lag < gps_comma){ // Lcok diffAge
              gps_comma_lag++;
            }
            break;
        }
        
		    gps_buffer[gps_index] = parsed_byte;
      }
      
      //Serial.print(parsed_byte);
      gps_index++;
    }
  }
  
}

void clear_buf(char* buf, int length){
	memset(buf,0,length);
}
