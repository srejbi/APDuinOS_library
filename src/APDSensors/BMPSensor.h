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
 * BMPSensor.h
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
 */

#ifndef BMPSENSOR_H_
#define BMPSENSOR_H_

#include "APDSensor.h"
#include <Wire.h>
#include <Adafruit_BMP085.h>				// TODO

struct BMPSENS {
  Adafruit_BMP085 *pbmp;
  float value;
  float sea_level_pressure;
};

class BMPSensor : public APDSensor
{
public:
  BMPSENS *sensor;

  BMPSensor(SDCONF *sdc, void *bmpsensor);
  boolean perform_check();
  virtual
  ~BMPSensor();
  void diagnostics();

private:
  float read_pressure();
  float read_temperature();
  float read_altitude();
  boolean bPrimary;
};

#endif /* BMPSENSOR_H_ */
