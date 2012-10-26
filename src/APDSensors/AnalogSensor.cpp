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
  // TODO Auto-generated constructor stub
  this->sensor = (ANASENS*)malloc(sizeof(ANASENS));
  this->sensor->value = 0;
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

char *AnalogSensor::getValueS(char *strdest) {
  char *retstr = NULL;
  sprintf(strdest,"%d",this->sensor->value);
  retstr=strdest;
  return retstr;
}
