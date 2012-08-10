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
 * BMPSensor.cpp
 *
 *  Created on: Apr 5, 2012
 *      Author: George Schreiber
 *
 * BMPSensor is a wrapper around the ADAFruit BMP085 implementation.
 * Get it from https://github.com/adafruit/Adafruit-BMP085-Library
 *
 * Credits: Adafruit_BMP085 Copyright Limor Fried/Ladyada for Adafruit Industries
 *
 * Integrated into APDuinOS by George Schreiber 05-04-2012
 *
 */

#include "BMPSensor.h"

BMPSensor::BMPSensor(SDCONF *sdc, void *bmpsensor)
{
  this->initSensor(sdc);
  // TODO Auto-generated constructor stub
  this->sensor = (BMPSENS*)malloc(sizeof(BMPSENS));
  if (this->sensor != NULL) {
      if (bmpsensor!=NULL) {
          this->bPrimary = false;
          this->sensor->pbmp = ((BMPSensor*)bmpsensor)->sensor->pbmp;
      } else {
          this->bPrimary = true;
          this->sensor->pbmp = new Adafruit_BMP085();
          this->sensor->pbmp->begin();                  // TODO allow one of the BMP guys to take mode as extra
      }
  }
  this->_state = STATE_READY;
}

BMPSensor::~BMPSensor()
{
  // TODO Auto-generated destructor stub
  if (this->sensor != NULL) {
        if (this->sensor->pbmp != NULL && this->bPrimary) free(this->sensor->pbmp);
        free(this->sensor);
    }
    if (this->pmetro != NULL) free(this->pmetro);
}

boolean BMPSensor::perform_check()
{
  boolean retcode = false;
  // TODO switch sensor class and read pressure/temperature/altitude accordingly
  // FIXME !implement sensor class-dependent checks
  switch (this->config.sensor_class) {
    case SENSE_TEMP:
      retcode = (this->sensor->value = this->read_temperature());
      break;
    case SENSE_PRESSURE:
      retcode = (this->sensor->value = this->read_pressure());
      break;
    case SENSE_ALTITUDE:
      // TODO add calibrated reads
      retcode = (this->sensor->value = this->read_altitude());
      break;
    default:
      ;
  }
  if (retcode) this->fvalue = this->sensor->value;
  return retcode;
}

//test BMP085
float BMPSensor::read_pressure() {
  return ((BMPSENS*)this->sensor)->pbmp->readPressure();
}

//test BMP085
float BMPSensor::read_temperature() {
  return (((BMPSENS*)this->sensor)->pbmp->readTemperature());
}

//test BMP085
float BMPSensor::read_altitude() {
  // TODO add calibrated reading
  return ((BMPSENS*)this->sensor)->pbmp->readAltitude();
}


//test BMP085
void BMPSensor::diagnostics() {
  float fpressure =0;
  SerPrintP("Temperature = ");
  Serial.print(((BMPSENS*)this->sensor)->pbmp->readTemperature());
  SerPrintP(" *C");

  SerPrintP("Pressure = ");
  Serial.print(((BMPSENS*)this->sensor)->pbmp->readPressure());
  Serial.println(" Pa");

  // Calculate altitude assuming 'standard' barometric
  // pressure of 1013.25 millibar = 101325 Pascal
  SerPrintP("Altitude = ");
  Serial.print(((BMPSENS*)this->sensor)->pbmp->readAltitude());
  SerPrintP(" meters\n");

  // you can get a more precise measurement of altitude
  // if you know the current sea level pressure which will
  // vary with weather and such. If it is 1015 millibars
  // that is equal to 101500 Pascals.
  SerPrintP("Real altitude = ");
  Serial.print(((BMPSENS*)this->sensor)->pbmp->readAltitude(((BMPSENS*)this->sensor)->sea_level_pressure));
  SerPrintP(" meters\n");
}

