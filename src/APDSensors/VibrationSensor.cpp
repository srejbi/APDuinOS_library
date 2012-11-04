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
 * VibrationSensor.cpp
 *
 *  Created on: Apr 10, 2012
 *      Author: George Schreiber
 */

#include "VibrationSensor.h"

VibrationSensor::VibrationSensor(SDCONF *sdc)
{
  this->initSensor(sdc);
  // todo log this when enabled log levels ("VIBRATION SENSOR INITIALIZING...");

  this->sensor = (VIBSENS*)malloc(sizeof(VIBSENS));
  if (sscanf(this->config.extra_data, "%d", &(this->sensor->readings)) != 1) {
    this->sensor->readings = 10;                           // DEFAULT VALUE FIXME define in header
    APDDebugLog::log(APDUINO_MSG_VIBSENDEFAULTRC,NULL);
  } else {
  	// todo log this when enabled log levels ("VIBRATION: CALIBRATIONDATA:"); Serial.println(this->sensor->readings);
  }

  this->history = (int*)malloc(sizeof(int)*this->sensor->readings);
  if (this->history) {
      memset(this->history,0,sizeof(int)*this->sensor->readings);
  }
  this->lastreading = 0;
  this->ltotal = 0;
  this->boverflow = false;
  this->sensor->value = NAN;
  // todo log this when enabled log levels ("VIBRATION SENSOR INIT DONE");
}

VibrationSensor::~VibrationSensor()
{
  free(this->history);
  this->history = NULL;
  free(this->sensor);
  this->sensor = NULL;
  delete(this->pmetro);
  this->pmetro = NULL;
}

boolean VibrationSensor::perform_check()
{
  boolean retcode = (this->sensor->value = this->read_sensor());
  this->fvalue = this->sensor->value;
  return retcode;
}

// do an averaged reading over a short period of time window
// should ultimately be evaluated as true/false
// (don't want interrupts for now)
float VibrationSensor::read_sensor() {
	// todo log this when enabled log levels("VIBRATION SENSOR READING");
  float avg_val = 0;                        // stores the average value

  if (this->history != NULL) {
      int i = this->lastreading % this->sensor->readings;
      // todo log this when enabled log levels ("reading"); Serial.print(i,DEC);
      this->ltotal -= this->history[i];
      this->history[i] = analogRead(this->config.sensor_pin);
      this->ltotal += this->history[i];
      this->lastreading++;
      int j =  this->boverflow ? this->sensor->readings : this->lastreading;
      if (this->lastreading >= this->sensor->readings) {
          this->lastreading = 0;
          this->boverflow = true;
      }
      // todo log this when enabled log levels ("MOD"); Serial.print(j); SerPrintP(" - TOT: "); Serial.println(this->ltotal,DEC);
      avg_val =  this->ltotal / j;

  } else {
  	// todo log this when enabled log levels ("SINGLE-READ");
      avg_val = analogRead(this->config.sensor_pin);
  }
  // todo log this when enabled log levels ("Average vibration voltage: "); Serial.println(avg_val, DEC);         // print out the average distance to the debugger
  return avg_val;
}
