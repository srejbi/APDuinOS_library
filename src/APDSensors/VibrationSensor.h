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
 * VibrationSensor.h
 *
 *  Created on: Apr 10, 2012
 *      Author: George Schreiber
 */

#ifndef VIBRATIONSENSOR_H_
#define VIBRATIONSENSOR_H_

#include "APDSensor.h"

struct VIBSENS {
  int value;
  int readings;
};


class VibrationSensor : public APDSensor
{
public:
  VIBSENS *sensor;
  VibrationSensor(SDCONF *sdc);
  boolean perform_check();
  virtual
  ~VibrationSensor();
private:
  float read_sensor();
  int *history;
  int lastreading;
  boolean boverflow;
  long ltotal;
};

#endif /* VIBRATIONSENSOR_H_ */
