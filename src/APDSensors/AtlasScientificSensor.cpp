/* APDuinOS Library
 * Copyright (C) 2012 by György Schreiber
 *
 * This file is part of the APDuinOS Library
 *
 * This Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the APDuinOS Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * AtlasScientificSensor.cpp
 *
 *  Created on: Aug 27, 2012
 *      Author: George Schreiber
 */

#include "AtlasScientificSensor.h"
AtlasScientificSensor::AtlasScientificSensor(SDCONF *sdc, void *assensor)
{
  int edscand = 0;		// extra data parameters scanned
  this->initSensor(sdc);

  this->sensor = (ASSENS*)malloc(sizeof(ASSENS));		// allocate sensor data
  memset(this->sensor,0,sizeof(ASSENS));				// 0 the memory
  this->sensor->channel = -1;							// channel defaults to -1 (unused)

  ASENC ast;											// temporary buffer
  int cht;												// channel


  // pre-scan (before building sensor) the extra parameters from the config

  // extra data should hold RS-232 port splitter configuration -> Software Serial
  // if present, it should be "<YN>,<S0>,<S1>,<E>", where YN is the split port no. 0-3, S0 and S1 are the pins to select channel and E is the enable pin
  edscand = sscanf(this->config.extra_data, "%d,%d,%d,%d",
		  	  	  	  	  	  	  	&cht, &(ast.S0), &(ast.S1), &(ast.E));

  // todo we might want to bail out now if 0 < edscand  < 3 because it means the config for SW serial is corrupted

  // start building up the sensor
  if (this->sensor != NULL) {
      if (assensor!=NULL) {			// if reusing another AtlasScientific on this port (should be SW Serial!)
    	  if (((AtlasScientificSensor*)assensor)->is_soft_serial()) {	// reusable Atlas Sensor MUST be SW
			  this->bPrimary = false;
			  this->sensor->asenc = ((AtlasScientificSensor*)assensor)->sensor->asenc;
			  if (memcmp(&(ast.S0), &(this->sensor->asenc->S0), sizeof(byte)*3) == 0) {
				  if (cht != ((AtlasScientificSensor*)assensor)->sensor->channel) {
					  this->sensor->channel = cht;	// OK, reusing a softserial, setting channel
					  SerPrintP(" channel "); Serial.print(this->sensor->channel);
				  } else {
					  SerPrintP("E");		// ERROR same SW serial channel
				  }
			  } else {
				  SerPrintP("E");			// ERROR different SoftSerial config on same HW pins
			  }
    	  } else {
    		  SerPrintP("E");				// ERROR invalid reusable Atlas Sensor (not SW Serial)
    	  }
      } else {
          this->bPrimary = true;
          this->sensor->asenc = (ASENC*)malloc(sizeof(ASENC));
          if (this->sensor->asenc != NULL) {
        	 // get ptr to HW serial or allocate a new SW serial object, according to extra config
        	 if (edscand == 0) {	// hardware serial (0 SW serial parameters scanned)
        	 //#ifdef DEBUG
        	     SerPrintP("ATLAS: HW Serial.\n");
        	 //#endif
        	     if (this->selectHWSerial()) {		// will detect which HW serial to open
        	    	 // start the hardware serial port
        	    	 ((HardwareSerial*)(((ASSENS*)(this->sensor))->asenc->serialport))->begin(ATLAS_BAUD_RATE);
        	     }	else {
        	    	 SerPrintP("E"); 				// ERROR selecting HW serial
        	     }
        	     // todo halt?
        	   } else {			// Software Serial (3 or 4 SW serial params scanned)
        	 	  if (edscand >= 3) {	// we at least have YN,S0,S1
        	 		  SerPrintP("ATLAS: SW Serial.\n");
        	 		  SerPrintP("YN: ");Serial.print(cht);SerPrintP(" S0: ");Serial.print(ast.S0);SerPrintP(" S1: ");Serial.print(ast.S1);

        	 		  if (edscand > 3) {
        	 			  SerPrintP(" E: ");Serial.print(ast.E);
        	 		  } else {
        	 			  ast.E = -1;		// E should be hard pulled GND
        	 		  }
        	 		  this->sensor->asenc->S0 = ast.S0;
        	 		  this->sensor->asenc->S1 = ast.S1;
        	 		  this->sensor->asenc->E = ast.E;
//        	 		  this->sensor->channel = cht;		// todo, remove, this should have been done already
        	 		  if (this->selectSWSerial()) {
        	 			 ((SoftwareSerial*)(((ASSENS*)(this->sensor))->asenc->serialport))->begin(ATLAS_BAUD_RATE);
        	 		  } else {
        	 			  SerPrintP("E");		// ERROR selecting sw serial
        	 		  }
        	 	  } else {
        	 		  SerPrintP("E");			// ERROR configuration error, at least 3 parameters (YN,S0,S1) should be provided for softserial
        	 	  }
        	   }
        	 	// todo bail out on previous errors otherwise state will look like ready even if error(s) occurred
          	this->sensor->asenc->state = STATE_READY;
          } else {
          	SerPrintP("E");			// ERROR - out of ram
          }
      }

      this->sensor->value = 0;
      this->fvalue = 0;

  }

  this->_lm = millis();
  this->_state = STATE_READY;
}

AtlasScientificSensor::~AtlasScientificSensor() {
  // TODO Auto-generated destructor stub
  if (this->sensor != NULL) {
      if (this->sensor->asenc != NULL && this->bPrimary) {
      	if (this->sensor->asenc->serialport != NULL) {
      	    if (this->is_soft_serial()) {
      	    	SoftwareSerial *sp = (SoftwareSerial *)(this->sensor->asenc->serialport);
      	    	sp->end();
      	    	delete(sp);
      	    } else {
      	    	HardwareSerial *sp = (HardwareSerial *)(this->sensor->asenc->serialport);
      	    	sp->end();
      	    	delete(sp);
      	    }
      	}
      	free(this->sensor->asenc);
      }
      free(this->sensor);
  }
  if (this->pmetro != NULL) delete(this->pmetro);
}

// returns true if using software serial
bool AtlasScientificSensor::is_soft_serial() {
	// decision is made if S0 and S1 filled and a channel is provided
	//return (this->sensor != NULL && this->sensor->asenc != NULL && this->sensor->asenc->S0 != 0 && this->sensor->asenc->S1 != 0 && this->sensor->channel > -1 );
	// if channel is used, it should be a software serial
	return (this->sensor != NULL && this->sensor->channel > -1);
}

// adopted from the Atlas-Scientific sample code for RS-232
void AtlasScientificSensor::openChannel(short channel) {
	switch (channel) {
	case 0: //open channel Y0
		digitalWrite(this->sensor->asenc->S0, LOW); //S0=0
		digitalWrite(this->sensor->asenc->S1, LOW); //S1=0
		break;
	case 1: //open channel Y1
		digitalWrite(this->sensor->asenc->S0, HIGH); //S0=1
		digitalWrite(this->sensor->asenc->S1, LOW); //S1=0
		break;
	case 2: //open channel Y2
		digitalWrite(this->sensor->asenc->S0, LOW); //S0=0
		digitalWrite(this->sensor->asenc->S1, HIGH); //S1=1
		break;
	case 3: //open channel Y3
		digitalWrite(this->sensor->asenc->S0, HIGH); //S0=0
		digitalWrite(this->sensor->asenc->S1, HIGH); //S1=1
		break;
	}
	this->print('/r');
	//the print cr was put in place to improve stability.
	//sometimes, when switching channels errant data
	//was passed. The print CR clears any incorrect data that was
	//transmitted to atlas scientific device.
	return;
}

// opens the sensor's SW serial channel (if any)
void AtlasScientificSensor::openChannel() {
	if (this->is_soft_serial() && this->sensor->channel > -1) {
		this->openChannel(this->sensor->channel);
	}
}



// opens the hardware serial port specified by the sensor pin
bool AtlasScientificSensor::selectHWSerial()
{
	bool bRetCode = false;
	if (this->sensor != NULL && ((ASSENS*)(this->sensor))->asenc->serialport == NULL) {
		HardwareSerial* pser = NULL;
		if (this->config.sensor_pin - this->config.sensor_secondary_pin == 1) {	// hardware serial pins must be neighbours, TX RX order
			switch (this->config.sensor_pin) {
			case 19:	// TX 18 RX 19 = Serial1
				pser = &Serial1;
				break;
			case 17:	// TX 16 RX 17 = Serial2
				pser = &Serial2;
				break;
			case 15:	// TX 14 RX 15 = Serial3
				pser = &Serial3;
				break;
			default:
				SerPrintP("E");			// ERROR invalid RX PIN for Hardware Serial
			}
		} else {
			SerPrintP("E");			// ERROR invalid PIN sequence for Hardware Serial
		}
		// todo check serial state
		((ASSENS*)(this->sensor))->asenc->serialport = (void *)pser;		// STORE pointer to serial port
		bRetCode = (pser != NULL);			// return true if a HW serial was selected
	} else {
		SerPrintP("E");				// ERROR, invalid object
	}
	return bRetCode;
}


// opens the sw serial port specified by the sensor pin
bool AtlasScientificSensor::selectSWSerial()
{
	bool bRetCode = false;
	if (this->sensor != NULL && ((ASSENS*)(this->sensor))->asenc->serialport == NULL) {
		// allocate a new SoftwareSerial
		SoftwareSerial *pser = new SoftwareSerial(this->config.sensor_pin, this->config.sensor_secondary_pin );
		if (pser != NULL) {
			((ASSENS*)(this->sensor))->asenc->serialport = (void *)pser;		// STORE pointer to serial port
			pinMode(this->sensor->asenc->S0, OUTPUT);							// S0 is OUTPUT
			pinMode(this->sensor->asenc->S1, OUTPUT);							// S1 is OUTPUT
			if (this->sensor->asenc->E > 0) {		// if E pin specified (not pulled to GND fixed)
				pinMode(this->sensor->asenc->E, OUTPUT);						// E is OUTPUT (LOW to enable, HIGH to disable)
			}
			bRetCode = true;
		} // else?
	} else {
		SerPrintP("E");				// ERROR, invalid object
	}
	return bRetCode;
}


boolean AtlasScientificSensor::perform_check()
{
  float nv = this->as_sensor_read();
  // todo revise the special values (-50,-100) below and change sensors to hold validity bit in _state (indicating if reading was valid) instead of altering value with custom "invalid values"
  if (this->_state == STATE_READY && nv > -50) this->fvalue = nv;
  return (nv > -100);
}


float AtlasScientificSensor::as_sensor_read()
{
	float sensorval = -120;
	//if (this->config.sensor_type != ATLASSCIENTIFIC_SENSOR || this->config.sensor_class != SENSE_PH )
	//	return -98;                 // no good type

	if (this->sensor->asenc == NULL || this->sensor->asenc->serialport == NULL ) {
		SerPrintP("E");			// error, no sensor port etc.
		return -999;
	}

   if (this->_state == STATE_READY) {			// attempt to trigger reading if ready
  	 if (this->sensor->asenc->state == STATE_READY) {		// if serial port is ready
  		 this->sensor->asenc->state = STATE_BUSY;				// set the AS serial port flag as busy (reused on the same pin)
  		 this->_state = STATE_WRITE;							// set this class state as write

  		 // instruct reading sensor
  		 this->print("r\r");				// AtlasScientific read command (valid for all classes /pH,EC,DO,ORP/)
  		 //this->print('\r');				// command confirmed by carriage return

  		 // and "come back" in 1100 ms for results
  		 this->pmetro->interval(1100);							// reschedule checking this sensor in 1100 ms (will go to the STATE_WRITE branch)
  		 //SerPrintP("COMMANDSENT:");Serial.print(millis() - this->_lm);
  		 this->_lm = millis();
  	 } else {												// the serial port is occupied (by another instance using the shared serial port)
  		 //SerPrintP(".");
  		 this->pmetro->interval(10);												// set a short interval so we time out quickly for retry
  	 }
   } else if (this->_state == STATE_WRITE) {		// if ready to fetch data
  	 // we assume nobody else is using the shared object in this state, should be STATE_BUSY (by this sensor)
	 int databytes = 0;		// bytes to read
	 char sz_rx[64];
	 databytes = this->fetch(sz_rx);
	 Serial.print(databytes); SerPrintP(" bytes received.\n");
	 Serial.print(sz_rx);

	 float sensorval = 0;
	 if (sscanf(sz_rx, "%f", &sensorval) == 0) {
		 SerPrintP("Something is not right.");
		 Serial.print(sz_rx);
	 } else {
		 this->fvalue = sensorval;
		 SerPrintP("Storing value: ");
		 Serial.print(this->fvalue);
	 }

  	 //SerPrintP("AS "); Serial.print(this->config.label); SerPrintP(" FETCH PIN "); Serial.print(this->config.sensor_pin,DEC); SerPrintP("...");
  	 //Serial.print(millis() - this->_lm);
  	 this->_lm = millis();

      // set ready states
      this->_state = STATE_READY;
      this->sensor->asenc->state = STATE_READY;
      this->pmetro->interval(this->config.sensor_freq);						// reset normal poll time
   } else {
  	 SerPrintP("Unknown state. Slowing sensor polling.\n");
  	 this->pmetro->interval(this->config.sensor_freq*10);
   }
  return sensorval;
}


void AtlasScientificSensor::diagnostics() {
}

void AtlasScientificSensor::calibrate() {
}


size_t AtlasScientificSensor::print(const char *psz) {
	if (this->is_soft_serial()) {
		this->openChannel();
		SoftwareSerial *sp = (SoftwareSerial *)this->sensor->asenc->serialport;
		return sp->print(psz);
	} else {
		HardwareSerial *sp = (HardwareSerial *)this->sensor->asenc->serialport;
		return sp->print(psz);
	}
}

size_t AtlasScientificSensor::print(const char pc) {
	if (this->is_soft_serial()) {
		this->openChannel();
		SoftwareSerial *sp = (SoftwareSerial *)this->sensor->asenc->serialport;
		return sp->print(pc);
	} else {
		HardwareSerial *sp = (HardwareSerial *)this->sensor->asenc->serialport;
		return sp->print(pc);
	}
}

size_t AtlasScientificSensor::fetch(char *psz_rx) {
	int bytes_avail = this->available();
	int i = 0;
	//char *pc = psz_rx;			// char pointer
	if (bytes_avail > 0) {		// AtlasScientific example was looking for 3+ bytes, why? (\r?)
      for (i = 0; i < bytes_avail ; i++) {
    	  psz_rx[i] = (char)this->read();			// this will be slow (each time checks if SW/HW) TODO consider using 2 separate iterations
    	  if (psz_rx[i] == '\r') {					// if end of command (CR)
    		  psz_rx[i] = 0;							// replace \r with \0
    		  break;									// get outta here
    	  }
      }
	}
	return (size_t)(sizeof(char)*i);
}

// returns number of available bytes on HW/SW serial, -1 on error
int AtlasScientificSensor::available() {
	int bytes_available = -1;
	if (this->sensor && this->sensor->asenc && this->sensor->asenc->serialport) {
	bytes_available = this->is_soft_serial() ?
							((SoftwareSerial *)this->sensor->asenc->serialport)->available() :
							((HardwareSerial *)this->sensor->asenc->serialport)->available();
	}
	return bytes_available;
}

// returns a character from HW/SW serial, -1 on error
int AtlasScientificSensor::read() {
	char cread = 0;
	if (this->sensor && this->sensor->asenc && this->sensor->asenc->serialport) {
	cread = this->is_soft_serial() ?
							((SoftwareSerial *)this->sensor->asenc->serialport)->read() :
							((HardwareSerial *)this->sensor->asenc->serialport)->read();
	}
	return cread;
}
