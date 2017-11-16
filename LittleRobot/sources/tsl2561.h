/*
	SFE_TSL2561 illumination sensor library for Arduino
	Mike Grusin, SparkFun Electronics
	
	This library provides functions to access the TAOS TSL2561
	Illumination Sensor.

	Rewriten in C for the need of compatibility with  LOccam .

*/

 #include <Wire.h>

 
#define TSL2561_CMD           0x80
#define TSL2561_REG_TIMING    0x01
#define TSL2561_ADDR	      0x39
#define	TSL2561_REG_CONTROL   0x00
#define  TSL2561_REG_DATA_0   0x0C
#define TSL2561_REG_DATA_1    0x0E


  byte _error				              ;
  unsigned char _i2c_adress	    	;
  
  boolean   begin(){
	  _i2c_adress = TSL2561_ADDR    ;
	  Wire.begin();
	  return(true);
}
  

  boolean writeByte(unsigned char adress, unsigned char value ){
	  Wire.beginTransmission(_i2c_adress);
	  Wire.write((adress & 0x0F) | TSL2561_CMD);
   //
	  Wire.write(value)             ;
	  _error = Wire.endTransmission()        ;
	  if (_error == 0) return(true) ;
	  else            return(false) ;
  }  // writeByte()

  
  boolean readByte(unsigned char address, unsigned char *value){
    Wire.beginTransmission(_i2c_adress);
    Wire.write((address & 0x0F) | TSL2561_CMD);
     _error = Wire.endTransmission();

  // Read requested byte
  if (_error == 0){
      Wire.requestFrom(_i2c_adress,1);
      if (Wire.available() == 1) {
        *value = Wire.read();
        return(true);
      }
  }
  return false  ;
  }  // readByte()
  
  
  
  boolean readUInt(unsigned char address, unsigned int *value){
	// Reads an unsigned integer (16 bits) from a TSL2561 address (low byte first)
	// Address: TSL2561 address (0 to 15), low byte first
	// Value will be set to stored unsigned integer
	
	char high, low;
	
	Wire.beginTransmission(_i2c_adress);
	Wire.write((address & 0x0F) | TSL2561_CMD);
	_error = Wire.endTransmission();

	// Read two bytes (low and high)
	if (_error == 0){
		Wire.requestFrom(_i2c_adress,2);
		if (Wire.available() == 2){
			low = Wire.read();
			high = Wire.read();
			// Combine bytes into unsigned int
			*value = word(high,low);
			return(true);
		}
	}	
	return(false);
}

  
  ////////////////////////////////////////////////////////////////
  
boolean setPowerUp(void){
	// Turn on TSL2561, begin integrations
	
	return(writeByte(TSL2561_REG_CONTROL,0x03));
}


boolean getData(unsigned int *data0, unsigned int *data1){
	// Retrieve raw integration results
	// data0 and data1 will be set to integration results
	
	// Get data0 and data1 out of result registers
	if (readUInt(TSL2561_REG_DATA_0,data0) && readUInt(TSL2561_REG_DATA_1,data1)) {
		  return(true);
	}
	return(false);
}
  
  
boolean setTiming(boolean gain, unsigned char stime, unsigned int *ms){
       byte _error		      ;
       unsigned char timing ;

	switch (stime){
		case 0: *ms = 14; break;
		case 1: *ms = 101; break;
		case 2: *ms = 402; break;
		default:*ms = 0;
	}
 
	// setTiming(gain,stime)
 
	if (readByte(TSL2561_REG_TIMING,&timing)){
		if (gain)  timing |= 0x10	;
		else       timing &= ~0x10	;


		// Set integration time (0 to 3)
		timing &= ~0x03;
		timing |= (stime & 0x03);
		if (writeByte(TSL2561_REG_TIMING,timing)){
			    return(true);
		}
		else {
		  Serial.println("Pb setTiming 1");
		  return false 	;
		}
	} 
	else{
	  Serial.println("Pb setTiming 0");
	  return false ;	
	}
 }  // setTiming()

 
 
boolean getLux(unsigned char gain, unsigned int ms, unsigned int CH0, unsigned int CH1, double *lux){
	// Convert raw data to lux
	// gain: 0 (1X) or 1 (16X), see setTiming()
	// ms: integration time in ms, from setTiming() or from manual integration
	// CH0, CH1: results from getData()
	// lux will be set to resulting lux calculation
 
	double ratio, d0, d1;
 
	// Determine if either sensor saturated (0xFFFF)
	// If so, abandon ship (calculation will not be accurate)
	if ((CH0 == 0xFFFF) || (CH1 == 0xFFFF)){
		*lux = 0.0;
		return(false);
	}

	// Convert from unsigned integer to floating point
	d0 = double(CH0) ; d1 = double(CH1) ;

	// We will need the ratio for subsequent calculations
	if(d0 > double(0))  ratio = d1 / d0;
	else("ERREUR d0 nul");

	d0 *= (402.0/ms); // normalize for integration time
	d1 *= (402.0/ms);

	if (!gain){ 	  // normalize for gain
		d0 *= 16;
		d1 *= 16;
	}

	// Determine lux per datasheet equations:
	
	if (ratio < 0.5){
		*lux = 0.0304 * d0 - 0.062 * d0 * pow(ratio,1.4);
		return(true);
	}
	if (ratio < 0.61){
		*lux = 0.0224 * d0 - 0.031 * d1;
		return(true);
	}
	if (ratio < 0.80){
		*lux = 0.0128 * d0 - 0.0153 * d1;
		return(true);
	}
	if (ratio < 1.30){
		*lux = 0.00146 * d0 - 0.00112 * d1;
		return(true);
	}
	// if (ratio > 1.30)
	*lux = 0.0;
	return(true);
}

