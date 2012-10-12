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
 * APDRule.h
 *
 *  Created on: Mar 27, 2012
 *      Author: George Schreiber
 */

#ifndef APDRULE_H_
#define APDRULE_H_

#include <Arduino.h>
#include <Metro.h>
#include "APDSensor.h"
#include "APDControl.h"
#include <MemoryFree.h>          // checking free ram
#include "APDEvaluator.h"
#include "APDTime.h"

// rule definitions
#define RF_FALSE              0
#define RF_TRUE               1
#define RF_METRO              3
#define RF_SCHEDULED          4
#define RF_IDLE_CHECK         5
#define RF_RAM_CHECK          6
#define RF_SENSOR_GT          64
#define RF_SENSOR_GTE         65
#define RF_SENSOR_LT          66
#define RF_SENSOR_LTE         67
#define RF_SENSOR_EQ          68
#define RF_SENSOR_GT_SENSOR   128
#define RF_SENSOR_GTE_SENSOR  129
#define RF_SENSOR_LT_SENSOR   130
#define RF_SENSOR_LTE_SENSOR  131
#define RF_SENSOR_EQ_SENSOR   132
#define RF_EVALUATE						127

//APDuino Rule Actions
#define APDRA_SET_OFF           0
#define APDRA_SET_ON            1
#define APDRA_SWITCH_VALUE      2
#define APDRA_SET_VALUE         3
#define APDRA_RCSWITCH_ON       16
#define APDRA_RCSWITCH_OFF      17
#define APDRA_RCPLUG_SET_VALUE  18
#define APDRA_RCPLUG_ON         19
#define APDRA_RCPLUG_OFF			  20
#define APDRA_VIRT_CUST_FUNC    128
#define APDRA_VIRT_SCREEN_NEXT  129
#define APDRA_VIRT_SYNCNTP      130
#define APDRA_NOOP              255


#define REEXEC_NONE								0
#define REEXEC_TRUE								1
#define REEXEC_FALSE							2
#define REEXEC_BOTH								3

#define MAX_CRON_EXPR_LEN         64		// max allowed length for cron expressions

// file storage structure
struct RDCONF {
  char label[13];
  int rule_definition;          // evaluation function
  int rf_sensor_idx;            // sensor index
  float rf_value;             // test value or test sensor index
  int rule_true_action;        // pick an APDuino Rule Action (APDRA_...)
  int rule_false_action;      // -"-
  int rule_control_idx;      // pick a control index to run the action on
  int value_mapping;          // pick a value mapping
  int ra_value;               // value to pass
  int ra_sensor_idx;          // -1 if ignored, sensor index if value from sensor
  int reexec;									// 0 - only exec if changed, 1 - reexec TRUE, 2 - reexec FALSE, 3 - REEXEC
  char *pszcron;							// pointer to a cron expression string (if specified); will be allocated dynamically
  char conditions[33];				// conditional expression to be evaluated
};


class APDRule
{
public:
  APDRule();
  APDRule(RDCONF *rdc,APDSensorArray *pSA,APDControlArray *pca);      // initialize with RDCONF
  APDRule(char *psz_rdc);                       // initialize with a string containing RDCONF
  void evaluateRule();
  virtual
  ~APDRule();

  char *getValueS(char *strdest);
  boolean bState();															// returns the last evaluation state

  //static APDRule *rule_parser(int iline, char *psz);

  RDCONF config;    // the APD Rule configuration
  boolean (*prulefunc)(APDRule *);   // the evaluation function pointer, rule
  void (*ptcontrolfunc)(APDControl *,int);        // control, value
  void (*pfcontrolfunc)(APDControl *,int);        // control, value
  APDSensor *psensor;                          // test - pointer to a sensor
  float *pvalue;                                // test - pointer to a float value
  float finternal;                              // if value testing, pvalue should point to finternal, or to a sensor's float value, otherwise
  APDControl *pcontrol;                        // outcome - pointer to a control
  int cvalue;                                   // integer control value
  float *pcsensorvalue;                         // pointer to a sensor to take control value from
  Metro *pmetro;

  // get the desired action's function ptr
  void (*get_rule_action_ptr(int rule_action))(APDControl *,int);

  static boolean apd_rule_idle_check(APDRule *pRule);
  static boolean apd_rule_metro(APDRule *pRule);
  static boolean apd_rule_scheduled(APDRule *pRule);
  //boolean apd_rule_rtc_metro(void *pRule);
  static boolean apd_rule_true(APDRule *pRule);
  static boolean apd_rule_false(APDRule *pRule);
  static boolean apd_rule_ram_check(APDRule *pRule);
  static boolean apd_rule_sensor_equ(APDRule *pRule);
  static boolean apd_rule_sensor_lt(APDRule *pRule);
  static boolean apd_rule_sensor_gt(APDRule *pRule);
  static boolean apd_rule_eval_conditions(APDRule *pRule);

private:
  void initBlank();
  boolean bLastState;
  APDSensorArray *psa;
  APDControlArray *pca;
  friend class APDRuleArray;
};

#endif /* APDRULE_H_ */
