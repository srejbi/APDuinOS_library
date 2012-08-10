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
 * DHTSensor.h
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

#ifndef DHTSENSOR_H_
#define DHTSENSOR_H_

#include "APDSensor.h"
#include "DHT.h"

struct DHTSENS {
  int type;
  DHT *dht;
  float value;
};

class DHTSensor : public APDSensor
{
public:
  DHTSENS *sensor;
  boolean perform_check();

  DHTSensor(SDCONF *sdc, void *dhtsensor);
  virtual
  ~DHTSensor();
private:
  boolean bPrimary;
};

#endif /* DHTSENSOR_H_ */
