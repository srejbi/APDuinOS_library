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
 * AnalogSensor.h
 *
 *  Created on: Apr 5, 2012
 *      Author: George Schreiber
 */

#ifndef ANALOGSENSOR_H_
#define ANALOGSENSOR_H_

#include "APDSensor.h"

// The following define is for APDuino Online compatibility
// maintain the supported classes in it
#define ANALOG_SENSOR_CLASSES [SENSE_VOLTS]

struct ANASENS {
  int value;
};

class AnalogSensor : public APDSensor
{
public:
  ANASENS *sensor;
  boolean perform_check();

  AnalogSensor(SDCONF *sdc);
  ~AnalogSensor();

  char *getValueS(char *strdest);
private:
};

#endif /* ANALOGSENSOR_H_ */
