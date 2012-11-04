/* APDuinOS Library
 * Copyright (C) 2012 by Gyorgy Schreiber
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
 * OneWireSensor.cpp
 *
 *  Created on: Apr 5, 2012
 *      Author: George Schreiber
 *
 * OneWireSensor is a wrapper around the Arduino OneWire library (v2.1).
 * Get it from http://www.pjrc.com/teensy/td_libs_OneWire.html
 *
 * Credits: OneWire library: Copyright (c) 2007, Jim Studt (original)
 * Contributors of later versions:
 * Paul Stoffregen, Guillermo Lovato, Jason Dangel,
 * Glenn Trewitt, Robin James, Tom Pollard, Josh Larios.
 *
 * Integrated into APDuinOS by George Schreiber 05-04-2012
 */

#include "OneWireSensor.h"

OneWireSensor::OneWireSensor(SDCONF *sdc, void *owsensor)
{
  this->initSensor(sdc);
  this->sensor = (OWSENS*)malloc(sizeof(OWSENS));

  if (this->sensor != NULL) {
      if (owsensor!=NULL) {
          this->bPrimary = false;
          this->sensor->owenc = ((OneWireSensor*)owsensor)->sensor->owenc;
      } else {
          this->bPrimary = true;
          this->sensor->owenc = (OWENC*)malloc(sizeof(OWENC));
          if (this->sensor->owenc != NULL) {
          	this->sensor->owenc->ow = new OneWire(this->config.sensor_pin);
          	this->sensor->owenc->state = STATE_READY;
          } else {
          	// todo log this when enabled log levels SerPrintP("malloc error");
          }
      }
      //TODO add address, other parameters ,etc
      if (strlen(this->config.extra_data)) {
      	hexbytes(this->config.extra_data,(byte*)(&this->sensor->address),8);
      } else {
      	memset(&(this->sensor->address),0,sizeof(this->sensor->address));
      }
      this->sensor->value = NAN;
      this->fvalue = NAN;

      // todo log this when enabled log levels
      SerPrintP("ADDR: ");
      for(byte i = 0; i < 8; i++) {
        Serial.write(' ');
        Serial.print(this->sensor->address[i], HEX);
      }
      SerPrintP("\n");
      // end todo log this when enabled log levels

      // verify address before use; if address invalid, then invalidate the address field and the sensor
      this->verify_address();
  }
  this->_type_s = 0;
  this->_lm = millis();
  this->_state = STATE_READY;
}

OneWireSensor::~OneWireSensor()
{
  // TODO Auto-generated destructor stub
  if (this->sensor != NULL) {
      if (this->sensor->owenc != NULL) {
      	if (this->bPrimary) {
					if (this->sensor->owenc->ow != NULL) {
						delete(this->sensor->owenc->ow);
						this->sensor->owenc->ow = NULL;
					}
					free(this->sensor->owenc);
      	}
      	this->sensor->owenc = NULL;
      }
      free(this->sensor);
      this->sensor = NULL;
  }
  delete(this->pmetro);
  this->pmetro = NULL;
}


void OneWireSensor::verify_address()
{
  // diagnostics
	char sztmp[64] = "";
  byte type_s = 0;
  byte nulladdr[8] = {0,0,0,0,0,0,0,0};

  sprintf_P(sztmp, PSTR("'%s' (pin %d)"), this->config.label, this->config.sensor_pin);		// construct debug log string part
  APDDebugLog::log(APDUINO_MSG_OWADDRVERIFY,sztmp);				// log verification activity

  if (this->sensor->owenc == NULL || this->sensor->owenc->ow == NULL ) {
  	APDDebugLog::log(APDUINO_ERROR_OWNOOBJ, NULL);
  	return;
  }
  if (this->sensor->owenc->state != STATE_READY) {
  	APDDebugLog::log(APDUINO_ERROR_OWNOTREADY, NULL);
  	return;
  }

  OneWire *dsp = ((OWSENS*)(this->sensor))->owenc->ow;

  if (memcmp(this->sensor->address, nulladdr,sizeof(byte)*8) != 0) {	// if looking for a specific addr.
  	dsp->reset_search();		// reset search (we might used it before)
  } else {
  	APDDebugLog::log(APDUINO_MSG_OWSEARCHING,NULL);				// log verification activity
  }

  byte addr[8];
  boolean bAfound = false;				// address found flag
  boolean bfound = false;					// a 1-wire found flag
  int iCount = 0;
  while ( !bAfound && iCount<10 && dsp->search(addr) ) {		// TODO: check the max no. (anyway, now with shared, the max should be tracked globally)
    bfound = true;
    // todo log this when enabled log levels
    SerPrintP("Loop "); Serial.print(iCount++); SerPrintP("...");
    SerPrintP("ROM =");
    for(byte i = 0; i < 8; i++) {
      Serial.write(' ');
      Serial.print(addr[i], HEX);
    }
    // end todo log this when enabled log levels

    if (OneWire::crc8(addr, 7) != addr[7]) {
    		APDDebugLog::log(APDUINO_ERROR_OWBADCRC, NULL);
        return;
    }

    // if we had no address set in config, or we had and it matches the address found
    if (memcmp(this->sensor->address, &nulladdr,sizeof(byte)*8) == 0 ||
    		memcmp(this->sensor->address, addr,sizeof(byte)*8) == 0) {
    	// check if we had an expectation, if not, we can save the address to the sensor config (will not be saved) todo fixup sensor dump enables storing found addresses for next run
    	if (memcmp(this->sensor->address, &nulladdr,sizeof(byte)*8) == 0) {	// if we had a config without a specific address (nulladdress)
    		memcpy(this->sensor->address, addr,sizeof(byte)*8);
    		APDDebugLog::log(APDUINO_MSG_OWFIRSTADDRSAVED,NULL);				// todo pass string with address
    	}
    	// anyway (we now have an address in the sensor config)
    	// todo construct log string with address
    	APDDebugLog::log(APDUINO_MSG_OWADDRFOUND,NULL);				// todo pass string with address
    	bAfound = true;
    } else {		// unexpected address (we were looking for a specific address and it was not found)
    	APDDebugLog::log(APDUINO_WARN_OWBADADDRFOUND,NULL);	 // log not the address we expected
    }

    // the first ROM byte indicates which chip
    //type_s = 0; already set
    switch (addr[0]) {
      case 0x10:
      	APDDebugLog::log(APDUINO_MSG_OWDS18S20, NULL);  // or old DS1820
        type_s = 1;
        break;
      case 0x28:
        APDDebugLog::log(APDUINO_MSG_OWDS18B20, NULL);
        break;
      case 0x22:
      	APDDebugLog::log(APDUINO_MSG_OWDS1822, NULL);
        break;
      default:
      	APDDebugLog::log(APDUINO_ERROR_OWNOTDS18X20, NULL);
        return;
    }

    if (memcmp(this->sensor->address, addr,sizeof(byte)*8) == 0) {
    	this->_type_s = type_s;
    	// todo log this when enabled log levels ("type_s saved:") (this->_type_s);
    }
  }
  if (!bfound && !bAfound) {
  	APDDebugLog::log(APDUINO_MSG_OWNOMOREADDR, NULL);
		dsp->reset_search();
		delay(250);
// todo remove deprecated code         return;
  } else if (bAfound) {
  	APDDebugLog::log(APDUINO_MSG_OWADDRVEROK, NULL);
  } else {
  	memset(this->sensor->address,0,sizeof(byte)*8);
  	APDDebugLog::log(APDUINO_ERROR_OWNOADDRESSES, NULL);
  }
  // todo remove deprecated code //dsp->reset();

  APDDebugLog::log(APDUINO_MSG_OWADDRVERDONE, NULL);
}


boolean OneWireSensor::perform_check()
{
  float nv = this->ow_temperature_read();
  if (this->_state == STATE_READY && nv != NAN) this->fvalue = nv;
  return (nv != NAN);
}


float OneWireSensor::ow_temperature_read()
{
	byte nulladdr[8] = {0,0,0,0,0,0,0,0};
  float celsius = NAN;
  if (this->config.sensor_type == ONEWIRE_SENSOR && this->config.sensor_class == SENSE_TEMP) {
		if (memcmp(this->sensor->address, &nulladdr, sizeof(byte)*8)) {		// if not null address
			 if (this->sensor->owenc != NULL && this->sensor->owenc->ow != NULL ) {		// the shared config & onewire object must exist
				 OneWire *dsp = ((OWSENS*)(this->sensor))->owenc->ow;

				 if (this->_state == STATE_READY) {			// attempt to trigger reading if ready
					 if (this->sensor->owenc->state == STATE_READY) {		// if sensor class is ready
						 //SerPrintP("OW "); Serial.print(this->config.label); SerPrintP(" POLL ON PIN "); Serial.print(this->config.sensor_pin,DEC); SerPrintP("...");
						 this->sensor->owenc->state = STATE_BUSY;						// set the 1-wire sensor class as busy (reused on the same pin)
						 this->_state = STATE_WRITE;												// set this class state as write
						 dsp->reset();
						 dsp->select(this->sensor->address);
						 dsp->write(0x44,1);         // start conversion, with parasite power on at the end

						 this->pmetro->interval(1000);							// reschedule checking this sensor in 750 ms (will go to the STATE_WRITE branch)
						 this->_lm = millis();

					 } else {																						// the shared sensor class is not ready
						 this->pmetro->interval(10);												// set a short interval so we time out quickly for retry
					 }
				 } else if (this->_state == STATE_WRITE) {		// if ready to fetch data
					 // we assume nobody else is using the shared object in this state, should be STATE_BUSY (by this sensor)

					 // todo log this when enabled log levels
					 //SerPrintP("OW "); Serial.print(this->config.label); SerPrintP(" FETCH PIN "); Serial.print(this->config.sensor_pin,DEC); SerPrintP("...");
					 //Serial.print(millis() - this->_lm);
					 this->_lm = millis();
					 byte i;
						byte present = 0;
						byte data[12];
						byte addr[8];
						memcpy(addr,this->sensor->address,sizeof(byte)*8);

						// we might do a ds.depower() here, but the reset will take care of it.
						present = dsp->reset();
						dsp->select(addr);
						dsp->write(0xBE);         // Read Scratchpad

						for ( i = 0; i < 9; i++) {           // we need 9 bytes
							data[i] = dsp->read();
						}
						// convert the data to actual temperature

						//The DS18x20_Temperature has a known bug. Remove "unsigned" from the raw variable on line 88, for correct results below zero degrees Celsius!
						//unsigned
						int raw = (data[1] << 8) | data[0];
						if (this->_type_s) {
							raw = raw << 3; // 9 bit resolution default
							if (data[7] == 0x10) {
								// count remain gives full 12 bit resolution
								raw = (raw & 0xFFF0) + 12 - data[6];
							}
						} else {
							byte cfg = (data[4] & 0x60);
							if (cfg == 0x00) raw = raw << 3;  // 9 bit resolution, 93.75 ms
							else if (cfg == 0x20) raw = raw << 2; // 10 bit res, 187.5 ms
							else if (cfg == 0x40) raw = raw << 1; // 11 bit res, 375 ms
							// default is 12 bit resolution, 750 ms conversion time
						}
						celsius = (float)raw / 16.0;
						// todo the following should be potentially added as an expression (by enduser) to convert sensor value
						//fahrenheit = celsius * 1.8 + 32.0;

						// set ready states
						this->_state = STATE_READY;
						this->sensor->owenc->state = STATE_READY;
						this->pmetro->interval(this->config.sensor_freq);						// reset normal poll time
				 } else {
					 SerPrintP("Unknown state. Slowing sensor.\n");
					 this->pmetro->interval(this->config.sensor_freq*10);
				 }
			 } else {		// no onewire object
				 APDDebugLog::log(APDUINO_ERROR_OWNOOBJ,NULL);
			 }
		} else {			// if sensor has nulladdress
			APDDebugLog::log(APDUINO_ERROR_OWNOADDR,NULL);
		}
  }
  return celsius;
}


void OneWireSensor::diagnostics()
{
	char sztmp[64] = "";
	sprintf_P(sztmp, PSTR("'%s' (pin %d)"), this->config.label, this->config.sensor_pin);		// construct debug log string part

	APDDebugLog::log(APDUINO_MSG_OWDIAGNOSTICS,sztmp);
  if (this->sensor->owenc == NULL || this->sensor->owenc->ow == NULL ) {
  	APDDebugLog::log(APDUINO_ERROR_OWNOOBJ,NULL);
  	return;
  }
  if (this->sensor->owenc->state != STATE_READY) {
  	APDDebugLog::log(APDUINO_ERROR_OWNOTREADY,NULL);
  	return;
  }

  // only call diagnostics for the primary 1-wire instance
  if (!this->bPrimary) {
      SerPrintP("Not the primary 1-wire instance on PIN.");
      return;
  }
  // if here, it's a primary instance
  OneWire *dsp = ((OWSENS*)(this->sensor))->owenc->ow;

  dsp->reset_search();		// reset search (we might used it before)
  delay(1000);
  byte addr[8];
  boolean bfound = false;
  int iCount = 0;
  while ( dsp->search(addr) && iCount<10) {
    bfound = true;
    // todo log this when enabled log levels
    SerPrintP("Loop "); Serial.print(iCount++); SerPrintP("...");
    SerPrintP("ROM =");
    for(byte i = 0; i < 8; i++) {
      Serial.write(' ');
      Serial.print(addr[i], HEX);
    }

    if (OneWire::crc8(addr, 7) != addr[7]) {
    	// todo log this when enabled log levels
        SerPrintP("CRC is not valid!");
        return;
    }
    Serial.println();

    SerPrintP("  Chip ");
    // the first ROM byte indicates which chip
    // todo log this when enabled log levels
    switch (addr[0]) {
      case 0x10:
        SerPrintP("= DS18S20");  // or old DS1820
        break;
      case 0x28:
        SerPrintP("= DS18B20");
        break;
      case 0x22:
        SerPrintP("= DS1822");
        break;
      default:
        SerPrintP("is not DS18x20 family.");
        return;
    }
  }
  if (bfound) {
  	APDDebugLog::log(APDUINO_MSG_OWNOMOREADDR, NULL);
		dsp->reset_search();
		delay(250);
//          return;
  } else {

  }
  dsp->reset();

  APDDebugLog::log(APDUINO_MSG_OWDIAGNOSTICSDONE, NULL);
  delay(1000);
}



