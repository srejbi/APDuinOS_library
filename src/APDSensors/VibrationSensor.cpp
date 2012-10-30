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
#ifdef DEBUG
  SerPrintP("VIBRATION SENSOR INITIALIZING...");
#endif
  // TODO Auto-generated constructor stub
  this->sensor = (VIBSENS*)malloc(sizeof(VIBSENS));
  if (sscanf(this->config.extra_data, "%d", &(this->sensor->readings)) != 1) {
    SerPrintP("VibrationSensor: using default readings (10).\n");
    this->sensor->readings = 10;                           // DEFAULT VALUE FIXME define in header
  }
#ifdef DEBUG
  else {
      SerPrintP("VIBRATION: CALIBRATIONDATA:"); Serial.println(this->sensor->readings);
  }
#endif
  this->history = (int*)malloc(sizeof(int)*this->sensor->readings);
  if (this->history) {
      memset(this->history,0,sizeof(int)*this->sensor->readings);
  }
  this->lastreading = 0;
  this->ltotal = 0;
  this->boverflow = false;
  this->sensor->value = NAN;
#ifdef DEBUG
  SerPrintP("VIBRATION SENSOR INIT DONE");
#endif
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

float VibrationSensor::read_sensor() {
#ifdef DEBUG
  SerPrintP("VIBRATION SENSOR READING");
#endif
  //delay(10);
  float avg_val = 0;                        // stores the average value

  if (this->history != NULL) {
      int i = this->lastreading % this->sensor->readings;
#ifdef DEBUG
      SerPrintP("reading"); Serial.print(i,DEC);
#endif
      this->ltotal -= this->history[i];
      this->history[i] = analogRead(this->config.sensor_pin);
      this->ltotal += this->history[i];
      this->lastreading++;
      int j =  this->boverflow ? this->sensor->readings : this->lastreading;
      if (this->lastreading >= this->sensor->readings) {
          this->lastreading = 0;
          this->boverflow = true;
      }
#ifdef DEBUG
      SerPrintP("MOD"); Serial.print(j); SerPrintP(" - TOT: "); Serial.println(this->ltotal,DEC);
#endif
      avg_val =  this->ltotal / j;

  } else {
#ifdef DEBUG
      SerPrintP("SINGLE-READ");
#endif
      avg_val = analogRead(this->config.sensor_pin);
  }
#ifdef DEBUG
  SerPrintP("Average vibration voltage: "); Serial.println(avg_val, DEC);         // print out the average distance to the debugger
#endif
  return avg_val;
}
