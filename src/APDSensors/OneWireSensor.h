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
 * OneWireSensor.h
 *
 *  Created on: Apr 5, 2012
 *      Author: George Schreiber
 *
 * OneWireSensor is a wrapper around the Arduino OneWire library (v2.1).
 * Get it from http://www.pjrc.com/teensy/td_libs_OneWire.html
 *
 * Credits: OneWire library: Copyright (c) 2007, Jim Studt (original)
 * Contributors of later versions:
 * Paul Stoffregen, Guillermo Lovato, Jason Dangel,
 * Glenn Trewitt, Robin James, Tom Pollard, Josh Larios.
 *
 * Integrated into APDuinOS by George Schreiber 05-04-2012
 *
 */

#ifndef ONEWIRESENSOR_H_
#define ONEWIRESENSOR_H_

#include "APDSensor.h"
#include <OneWire.h>

// The following define is for APDuino Online compatibility
// maintain the supported classes in it
#define ONEWIRE_SENSOR_CLASSES [SENSE_TEMP]

struct OWENC {	// OneWire encapsulation
	OneWire *ow;		// the shared object
	byte state;			// state of the shared object (OneWire)
};

struct OWSENS {	// OneWire sensor struct
  byte address[8];
  OWENC *owenc;
  float value;
};

class OneWireSensor : public APDSensor
{
public:
  OWSENS *sensor;
  OneWireSensor(SDCONF *sdc, void *owsensor);
  boolean perform_check();
  virtual
  ~OneWireSensor();

  void diagnostics();

private:
  void verify_address();
  float ow_temperature_read();
  byte _type_s;
  boolean bPrimary;
  unsigned long _lm;
};

#endif /* ONEWIRESENSOR_H_ */
