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
 * AtlasScientificSensor.h
 *
 *  Created on: Aug 27, 2012
 *      Author: George Schreiber
 */

#ifndef ATLASSCIENTIFICSENSOR_H_
#define ATLASSCIENTIFICSENSOR_H_

#include "APDSensor.h"
#ifdef ATLAS_SOFTSERIAL
#include <SoftwareSerial.h>
#endif
// The following define is for APDuino Online compatibility
// maintain the supported classes in it
#define ATLASSCIENTIFIC_SENSOR_CLASSES [SENSE_PH,SENSE_DO,SENSE_EC,SENSE_ORP]

// serial/softserial indicator
// mask the encapsulation struct state with it
#define AS_SERIAL			0x00			// 0000 0000 - using serial
#define AS_SOFTSERIAL		0x10			// 0001 0000 - using software serial

#define ATLAS_BAUD_RATE		38400			// AtlasScientific hw. baud rate

struct ASENC {	// AtlasScientific serial connection encapsulation struct
	void *serialport;	// holds a pointer to a HardwareSerial or to a SoftwareSerial
    byte S0;			// channel select pin 1 (!=0 if softser)
    byte S1;			// channel select pin 2 (!=0 if softser)
    byte E;				// enable pin (!= 0 if softser & not hw pulled low)
    int csel;			// the currently selected channel -> do not re-set if selected because selection sends \r

	byte state;			// state of the port
};

struct ASSENS {
  ASENC *asenc;
  int channel;		// NAN - unused, 0-3 YN
  float value;
};

class AtlasScientificSensor: public APDSensor {
public:
	ASSENS *sensor;
	AtlasScientificSensor(SDCONF *sdc, void *assensor);
	virtual ~AtlasScientificSensor();

	boolean perform_check();

	void command(const void *cmd);

private:
	bool is_hw_serial();
	bool is_demux();
	void openChannel(short channel);
	void openChannel();
	bool selectHWSerial();
#ifdef ATLAS_SOFTSERIAL
	bool selectSWSerial();
#endif
	float as_sensor_read();

    boolean bPrimary;					// is the sensor primary (having a shared object)
    unsigned long _lm;
    size_t print(const char *psz);
    size_t print(const char pc);
    int fetch(char *psz_rx);
    int read();
    int available();
};

#endif /* ATLASSCIENTIFICSENSOR_H_ */
