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
 * APDSensorArray.h
 *
 *  Created on: Apr 3, 2012
 *      Author: George Schreiber
 */

#ifndef APDSENSORARRAY_H_
#define APDSENSORARRAY_H_

#include <Arduino.h>
#include "apd_version.h"
#include "apd_utils.h"
#include "APDSensor.h"
#include "APDStorage.h"
#include "APDLogWriter.h"
#include "APDSensors/AnalogSensor.h"
#include "APDSensors/DigitalSensor.h"
#include "APDSensors/DHTSensor.h"
#include "APDSensors/OneWireSensor.h"
#include "APDSensors/SonarSensor.h"
#include "APDSensors/BMPSensor.h"
#include "APDSensors/VibrationSensor.h"
#include "APDSensors/AtlasScientificSensor.h"

class APDSensorArray
{
public:
  APDSensor** pAPDSensors;          // will hold a pointer to the array of APD Sensors (ptrs)
  int iSensorCount;

  APDSensorArray();
  int dumpToFile(char *pszFileName);
  virtual
  ~APDSensorArray();

  void enableRuleEvaluation(void (*pfunc)(void*, APDSensor *),void *pra);
  APDSensor *firstSensorByPin(int iPin, int iType);
  int indexBySensor(APDSensor *pSensor);
  APDSensor *findReusableSensor(SDCONF *sdc);
  APDSensor *byIndex(int idx);

  static void new_sensor_parser(void *pSA, int iline, char *psz);
  int loadSensors();
  void pollSensors(boolean bProcessRules);
  void diagnostics();

  int count();
  char *labelS(int iSensorIdx, char *szlabel);
  char *valueS(int iSensorIdx, char *szvalue);

private:
  int iNextSensor;
  void (*pfruleeval)(void *,APDSensor *);        // rule_array as void ptr, value
  void *pRA;														// pointer to rule array

  friend class APDRuleArray;
};

#endif /* APDSENSORARRAY_H_ */
