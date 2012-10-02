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
#ifdef DEBUG
  SerPrintP("COPY SENSOR DEF\n");
#endif
  memcpy((void*)&(this->config),(void*)sdc,sizeof(SDCONF));       // copy the structure to config
#ifdef VERBOSE
  SerPrintP("SENSOR DEFINITION:"); SerPrintP(" - "); Serial.print(this->config.label); SerPrintP("(label), ");
      Serial.print(this->config.sensor_type); SerPrintP(" (sensor_type),"); Serial.print(this->config.sensor_class);
      SerPrintP(" (class),"); Serial.print(this->config.sensor_pin); SerPrintP("(pin),");
      Serial.print(this->config.sensor_secondary_pin); SerPrintP("(secondary_pin),");
      Serial.print(this->config.sensor_subtype); SerPrintP(" (subtype),"); Serial.print(this->config.sensor_freq);
      SerPrintP("(freq),"); Serial.print(this->config.sensor_log); SerPrintP("(log),");
      Serial.print(this->config.extra_data); SerPrintP("(extra data),"); SerPrintP("|\n");
#endif
   boolean bInstantiated = false;      // set if instance was setup ok - type dependent
   if (this->config.sensor_type > 0) {
#ifdef VERBOSE
       SerPrintP("This is a specific sensor type...");
#endif
   }
   this->pmetro = NULL;
   this->sensor = NULL;
}


APDSensor::~APDSensor()
{
  // TODO Auto-generated destructor stub
  if (this->pmetro) delete(this->pmetro);
  if (this->sensor) free(this->sensor);
}

void APDSensor::initSensor() {
  memset(&config,0,sizeof(SDCONF));
  sensor = NULL;
  pmetro = NULL;

  fvalue = 0;

  _state = STATE_BUSY;
}

void APDSensor::initSensor(SDCONF *sdc) {
  this->initSensor();
  memcpy(&(this->config),sdc,sizeof(SDCONF));
  this->pmetro = new Metro(this->config.sensor_freq, true);
#ifdef VERBOSE
  SerPrintP("\ninitSensor("); Serial.print(this->config.label); SerPrintP(") done.\n");
#endif
}



char *APDSensor::getValueS(char *strdest) {
#ifdef DEBUG
  SerPrintP("getValueS running\n");
#endif
  char *retstr = NULL;
  sprintf(strdest,"%3.1f",this->fvalue);
  retstr=strdest;
  return retstr;
}


boolean APDSensor::check() {
  boolean retcode = false;
  if (this->pmetro != NULL && this->pmetro->check()) {
#ifdef DEBUG
      Serial.print(this->config.label); SerPrintP(" - metro up. Performing check...");
#endif
      retcode = this->perform_check();
#ifdef DEBUG
      SerPrintP(" - done, reset metro\n");
#endif
      this->pmetro->reset();
  }/* else {
  	Serial.print(this->config.label); SerPrintP(" metroskip.");
  	if (this->pmetro == NULL)
  		SerPrintP(" NULLMETRO\n");
  }*/
  return retcode;
}




boolean APDSensor::perform_check() {
  // implement the sensor-specific check
}

void APDSensor::diagnostics() {
  // implement the sensor-specific check
}



int APDSensor::iValue() {
	return (int)(this->fvalue);
}

float APDSensor::fValue() {
	return (this->fvalue);
}




