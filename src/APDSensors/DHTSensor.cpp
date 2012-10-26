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
 * DHTSensor.cpp
 *
 *  Created on: Apr 5, 2012
 *      Author: George Schreiber
 *
 * DHTSensor is a wrapper around the Adafruit DHT library
 * Get it from https://github.com/adafruit/DHT-sensor-library
 *
 * Credits: DHT-sensor-library Copyright Adafruit Industries
 *
 * Integrated into APDuinOS by George Schreiber 05-04-2012
 */

#include "DHTSensor.h"

DHTSensor::DHTSensor(SDCONF *sdc, void *dhtsensor)
{
  this->initSensor(sdc);
  // TODO Auto-generated constructor stub
  this->sensor = (DHTSENS*)malloc(sizeof(DHTSENS));
  if (this->sensor != NULL) {
      if (dhtsensor!=NULL) {
          this->bPrimary = false;
          ((DHTSENS*)this->sensor)->dht = ((DHTSensor*)dhtsensor)->sensor->dht;
      } else {
          this->bPrimary = true;
          ((DHTSENS*)this->sensor)->dht = new DHT(this->config.sensor_pin, this->config.sensor_subtype);
          if (((DHTSENS*)this->sensor)->dht != NULL) {
              ((DHTSENS*)this->sensor)->dht->begin();
              //SerPrintP("DHT object should be initialized.\n");
          }
      }
  }
  this->fvalue = 0;
  this->_state = STATE_READY;
}

DHTSensor::~DHTSensor()
{
  if (this->sensor != NULL) {
      if (this->sensor->dht != NULL) {
      	if (this->bPrimary ) {
      		delete(this->sensor->dht);
      	}
      	this->sensor->dht = NULL;
      }
      free(this->sensor);
      this->sensor = NULL;
  }
  delete(this->pmetro);
  this->pmetro = NULL;
}

boolean DHTSensor::perform_check()
{
  float value = 0;
  switch (this->config.sensor_class) {
    case SENSE_TEMP:
#ifdef DEBUG
      SerPrintP("TEMP");
#endif
      value = ((DHTSENS *)this->sensor)->dht->readTemperature();
      break;
    case SENSE_HUMIDITY:
#ifdef DEBUG
      SerPrintP("HUM");
#endif
      value = ((DHTSENS *)this->sensor)->dht->readHumidity();
      break;
    default:
//#ifdef DEBUG
      SerPrintP("DHTSensor: Invalid sensor class!"); Serial.print(this->config.sensor_class);
//#endif
      break;
  }
  // if (value == NAN) { this-> status |= }

  return (this->fvalue = value);
}
