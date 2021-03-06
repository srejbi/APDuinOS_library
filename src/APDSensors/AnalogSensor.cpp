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
 * AnalogSensor.cpp
 *
 *  Created on: Apr 5, 2012
 *      Author: George Schreiber
 */

#include "AnalogSensor.h"

AnalogSensor::AnalogSensor(SDCONF *sdc)
{
  this->initSensor(sdc);
  this->sensor = (ANASENS*)malloc(sizeof(ANASENS));
  this->sensor->value = NAN;
  this->_state = STATE_READY;
}

AnalogSensor::~AnalogSensor()
{
  free(this->sensor);
  this->sensor = NULL;
  delete(this->pmetro);
  this->pmetro = NULL;
}

boolean AnalogSensor::perform_check()
{
  boolean retcode = (this->sensor->value = analogRead(this->config.sensor_pin));
  this->fvalue = this->sensor->value;
  return retcode;
}

// returns the sensor value as string (base 10)
// strdest must point to a character buffer large enough to receive an integer 0-255 (4bytes with \0)
// returns a pointer to the character buffer, NULL on error
char *AnalogSensor::get_value_str(char *strdest) {
  char *retstr = NULL;
  if (sprintf_P(strdest,PSTR("%d"),this->sensor->value) != EOF) retstr = strdest;
  return retstr;
}
