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
 * APDSensor.cpp
 *
 *  Created on: Mar 27, 2012
 *      Author: George Schreiber
 */

#include "APDSensor.h"

APDSensor::APDSensor() {
  initSensor();
  this->pmetro = NULL;
  this->sensor = NULL;
}



APDSensor::APDSensor(SDCONF *sdc) {
  initSensor();
  // todo log this when enabled log levels ("COPY SENSOR DEF\n");
  memcpy((void*)&(this->config),(void*)sdc,sizeof(SDCONF));       // copy the structure to config
  // todo log this when enabled log levels SerPrintP("SENSOR DEFINITION:"); SerPrintP(" - "); Serial.print(this->config.label); SerPrintP("(label), "); \
      Serial.print(this->config.sensor_type); SerPrintP(" (sensor_type),"); Serial.print(this->config.sensor_class); \
      SerPrintP(" (class),"); Serial.print(this->config.sensor_pin); SerPrintP("(pin),"); \
      Serial.print(this->config.sensor_secondary_pin); SerPrintP("(secondary_pin),"); \
      Serial.print(this->config.sensor_subtype); SerPrintP(" (subtype),"); Serial.print(this->config.sensor_freq); \
      SerPrintP("(freq),"); Serial.print(this->config.sensor_log); SerPrintP("(log),"); \
      Serial.print(this->config.extra_data); SerPrintP("(extra data),"); SerPrintP("|\n"); \

   boolean bInstantiated = false;      // set if instance was setup ok - type dependent
   if (this->config.sensor_type > 0) {
  	 // todo log this when enabled log levels ("This is a specific sensor type...");
   }
   this->pmetro = NULL;
   this->sensor = NULL;
}


APDSensor::~APDSensor()
{
  delete(this->pmetro);
  this->pmetro = NULL;
  free(this->sensor);
  this->sensor = NULL;
}

void APDSensor::initSensor() {
  memset(&config,0,sizeof(SDCONF));
  sensor = NULL;
  pmetro = NULL;
  fvalue = NAN;
  _state = STATE_BUSY;
}

void APDSensor::initSensor(SDCONF *sdc) {
  this->initSensor();
  memcpy(&(this->config),sdc,sizeof(SDCONF));
  this->pmetro = new Metro(this->config.sensor_freq, true);
  // todo log this when enabled log levels ("\ninitSensor("); Serial.print(this->config.label); SerPrintP(") done.\n");
}


char *APDSensor::get_value_str(char *strdest) {
  char *retstr = NULL;			// returns NULL on error
  // todo make the formatstring customizable to allow higher precision on demand (requires revision of references and making sure buffer is large enough for receiving more digits)
  if (sprintf_P(strdest,PSTR("%3.1f"),this->fvalue) != EOF) retstr = strdest;		// return the pointer to the buffer unless EOF was recived (error)
  return retstr;
}


boolean APDSensor::check() {
  boolean retcode = false;
  if (this->pmetro != NULL && this->pmetro->check()) {
  	// todo log this when enabled log levels (this->config.label); SerPrintP(" - metro up. Performing check...");
      retcode = this->perform_check();
      // todo log this when enabled log levels (" - done, reset metro\n");
      this->pmetro->reset();
  }
#ifdef DEBUG
  else {
  	// todo log this when enabled log levels Serial.print(this->config.label);  (" metroskip.");
  	if (this->pmetro == NULL)
  		// todo log this when enabled log levels(" NULLMETRO\n");
  }
#endif
  return retcode;
}



// stub for method that performs the check
boolean APDSensor::perform_check() {
  // implement the sensor-specific check: put all code here needed for communication with sensor
}

// stub for method that performs diagnostics on sensor
void APDSensor::diagnostics() {
  // implement the sensor-specific check, if supported
}

// returns the sensor value as integer (may overflow)
int APDSensor::iValue() {
	return (int)(this->fvalue);
}

// returns the sensor value as float
float APDSensor::fValue() {
	return (this->fvalue);
}


