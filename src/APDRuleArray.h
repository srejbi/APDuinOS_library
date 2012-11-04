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
 * APDRuleArray.h
 *
 *  Created on: Apr 6, 2012
 *      Author: George Schreiber
 */

#ifndef APDRULEARRAY_H_
#define APDRULEARRAY_H_

#include <Arduino.h>
#include "apd_version.h"
#include "apd_utils.h"
#include "APDSensorArray.h"
#include "APDControlArray.h"
#include "APDStorage.h"
#include "APDRule.h"
#include "APDLogWriter.h"

class APDRuleArray
{
public:
  APDRuleArray();
  APDRuleArray(APDSensorArray *psa, APDControlArray *pca, float *bfidle);
  virtual
  ~APDRuleArray();

  static void new_rule_parser(void *pRA, int iline, char *psz);
  int load_rules();
  void dump_to_file(char *pszFileName);

  APDRule *first_rule_by_sensor_index(int iSensorIdx);
  APDRule *first_rule_by_control_index(int iControlIdx);

  void evaluate_rules_by_sensor_index(int iSensorIndex);
  static void evaluate_sensor_rules(void *pra, APDSensor *pSensor);
  void loop_rules();
  void evaluate_scheduled_rules();

private:
  void adjust_next_cron_minute();

  APDSensorArray *pSA;
  APDControlArray *pCA;
  APDRule** pAPDRules;
  int iRuleCount;
  float *bfIdle;
  unsigned long nextrunmillis;
  int lastCronMin;
  friend class APDuino;         //FIXME if needed
  friend class APDSensorArray;
};


#endif /* APDRULEARRAY_H_ */
