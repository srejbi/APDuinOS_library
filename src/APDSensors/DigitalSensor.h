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
 * DigitalSensor.h
 *
 *  Created on: Apr 5, 2012
 *      Author: George Schreiber
 */

#ifndef DIGITALSENSOR_H_
#define DIGITALSENSOR_H_

#include "APDSensor.h"

// The following define is for APDuino Online compatibility
// maintain the supported classes in it
#define DIGITAL_SENSOR_CLASSES [SENSE_BITS]

struct DIGISENS {
  int value;
};

class DigitalSensor : public APDSensor
{
public:
  DIGISENS *sensor;

  DigitalSensor(SDCONF *sdc);
  boolean perform_check();
  virtual
  ~DigitalSensor();
  char *getValueS(char *strdest);
private:

};

#endif /* DIGITALSENSOR_H_ */
