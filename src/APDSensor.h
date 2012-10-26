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
 * APDSensor.h
 *
 *  Created on: Mar 27, 2012
 *      Author: George Schreiber
 */

#ifndef APDSENSOR_H_
#define APDSENSOR_H_

#include <Arduino.h>
#include "Metro.h"

#include "apd_utils.h"
#include "APDSerial.h"
#include "APDDebugLog.h"

// APDuino sensor types
#define ANALOG_SENSOR  0
#define DIGITAL_SENSOR 1
#define DHT_SENSOR     2
#define ONEWIRE_SENSOR 3
#define I2C_SENSOR     4
#define SONAR_SENSOR   5
#define BMP085_SENSOR   6
#define VIBRATION_SENSOR 7
#define ATLASSCIENTIFIC_SENSOR 8
#define VIRTUAL_SENSOR 128


// define sensor classes
#define SENSE_VOLTS     0
#define SENSE_BITS      1
#define SENSE_TEMP      2
#define SENSE_HUMIDITY  3
#define SENSE_DISTANCE  4
#define SENSE_POSITION  5
#define SENSE_PRESSURE  6
#define SENSE_ALTITUDE  7
#define SENSE_PH        32
#define SENSE_ORP       33
#define SENSE_EC		34
#define SENSE_DO		35

// Sensor Definition Configuration ("record") - the following definition should come from file
struct SDCONF {
  char label[13];
  int sensor_type;
  int sensor_class;
  int sensor_subtype;
  int sensor_pin;				   // sensor pin is the primary pin for *reading* (so if sensor needs 2 pins, this is RX/READ)
  int sensor_secondary_pin;        // special sensors operate with multiple pins (eg. RX/TX), we support 2 pins - this is TX/WRITE/TRIGGER
  int sensor_freq;
  int sensor_log;
  char extra_data[24];
};

// The SENSOR_PARSERSTRING is used for parsing controls config files by new_control_parser
#define SENSOR_PARSERSTRING PSTR("%s %d,%d,%d,%d,%d,%d,%d,%s")
// The following line needed for APDuino Online integration
// it should define valid Ruby code that will be executed
// in the context of the DeviceControl (self) when packaging configuration items
// The code must be inline with SENSOR_PARSERSTRING and new_sensor_parser (APDSensorArray)
#define AO_SENSOR_PACKSTRING "\"#{self.apduinos_label} #{self.sd_type},#{self.sd_class},#{self.sd_subtype||\"0\"},#{self.sd_pin.blank? ? \"-1\" : self.sd_pin},#{self.sd_secondary_pin.blank? ? \"-1\" : self.sd_secondary_pin},#{self.sd_poll_freq.blank? ? \"5000\" : self.sd_poll_freq},#{self.sd_logging.blank? ? \"0\" : self.sd_logging},#{self.sd_extra_data}\""


// sensor states; not really in use yet (1-wire & Atlas uses it)... #todo widespread the use of state
#define STATE_READY         0x00             // 0000
#define STATE_BUSY          0x01             // 0001
#define STATE_WAIT          0x02             // 0010
#define STATE_READ          0x04             // 0100
#define STATE_WRITE         0x08             // 1000

// concept:
// for sensors that need a longer delay between cmd and read, they should adjust their metro
// set state and bail out, the sensor array should take care to sort sensors by metro values
// and keep an execution plan up to date to always deal with the next sensor on time
// the sensor is responsible for carrying out it's operation according to it's state in say 10-20 ms max
// and bail out so the next sensor can be polled

// note: things like networking could interfere with these metros. #todo consider setting up interrupt for critical timers


// The following define is a stub for APDuino Online compatibility
// include it in every sensor type implementation header and
// maintain the supported classes in it (it is not used by the code, so don't worry about overdeclarations)
#define _SENSORNAME_CLASSES [SENSE_VOLTS]

class APDSensor
{
public:
  SDCONF config;
  void *sensor;    // pointer to a sensor definition structure (cast according to the type)

  APDSensor();
  APDSensor(SDCONF *sdc);
  virtual ~APDSensor();

  boolean check();

  virtual boolean perform_check();
  virtual char *getValueS(char *strdest);
  virtual void diagnostics();

  int iValue();
  float fValue();

//private:
protected:
  float fvalue;
  Metro *pmetro;   // we will metro according to config.sensor_freq ...
  byte _state;      // sensor state

  void initSensor();
  void initSensor(SDCONF *sdc);

  friend class APDRule;
  friend class APDRuleArray;
  friend class APDWeb;
};


#endif /* APDSENSOR_H_ */
