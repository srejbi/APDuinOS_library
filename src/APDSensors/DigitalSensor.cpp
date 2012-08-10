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
 * DigitalSensor.cpp
 *
 *  Created on: Apr 5, 2012
 *      Author: George Schreiber
 */

#include "DigitalSensor.h"

DigitalSensor::DigitalSensor(SDCONF *sdc)
{
  this->initSensor(sdc);
  // TODO Auto-generated constructor stub
  this->sensor = (DIGISENS*)malloc(sizeof(DIGISENS));
  this->_state = STATE_READY;
  this->sensor->value = 0;
}

DigitalSensor::~DigitalSensor()
{
  // TODO Auto-generated destructor stub
  if (this->sensor != NULL) free(this->sensor);
  if (this->pmetro != NULL) free(this->pmetro);
}

boolean DigitalSensor::perform_check()
{
  boolean retcode = (this->sensor->value = digitalRead(this->config.sensor_pin));
  this->fvalue = this->sensor->value;
  return retcode;
}

char *DigitalSensor::getValueS(char *strdest) {
  char *retstr = NULL;
  sprintf(strdest,"%d",this->sensor->value);
  retstr=strdest;
  return retstr;
}
