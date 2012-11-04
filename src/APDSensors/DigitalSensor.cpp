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
 * DigitalSensor.cpp
 *
 *  Created on: Apr 5, 2012
 *      Author: George Schreiber
 */

#include "DigitalSensor.h"

DigitalSensor::DigitalSensor(SDCONF *sdc)
{
  this->initSensor(sdc);
  this->sensor = (DIGISENS*)malloc(sizeof(DIGISENS));
  this->_state = STATE_READY;
  this->sensor->value = NAN;
}

DigitalSensor::~DigitalSensor()
{
	free(this->sensor);
	this->sensor = NULL;
	delete(this->pmetro);
	this->pmetro = NULL;
}

// wrapper around digitalRead() (wiring_digital)
boolean DigitalSensor::perform_check()
{
  boolean retcode = (this->sensor->value = digitalRead(this->config.sensor_pin));
  this->fvalue = this->sensor->value;
  return retcode;
}
// prints value as string, "nan" on NAN into strdest
// strdest must be large enough to receive
// returns ptr to string (strdest), NULL on error
char *DigitalSensor::get_value_str(char *strdest) {
  char *retstr = NULL;
  if (sprintf_P(strdest,PSTR("%d"),this->sensor->value) != EOF) retstr=strdest;
  return retstr;
}
