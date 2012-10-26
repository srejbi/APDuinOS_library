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
#include "APDDebugLog.h"

// Rule Evaluation Definitions
// Each signifying a method how the rule is to be evaluated in the first place
// (in the second place expression will be evaluated if provided)
#define RF_FALSE              0				// Rule will go on false branch
#define RF_TRUE               1				// Rule will go on true branch
#define RF_METRO              3				// Rule will go on true branch (unless expression evaluates false) every N milliseconds
#define RF_SCHEDULED          4				// Rule will go on true branch (unless expression evaluates false) as per cron-like time specification, false every other minute
#define RF_IDLE_CHECK         5				// Rule will go on true if device is idle, false if not (used for hardware UI)
#define RF_RAM_CHECK          6				// Rule not implemented
#define RF_SENSOR_GT          64			// Rule will go on true if sensor value is greater than test value, false otherwise
#define RF_SENSOR_GTE         65			// Rule will go on true if sensor value is greater than or equal to test value, false otherwise
#define RF_SENSOR_LT          66			// Rule will go on true if sensor value is less than test value, false otherwise
#define RF_SENSOR_LTE         67			// Rule will go on true if sensor value is less than or equal to test value, false otherwise
#define RF_SENSOR_EQ          68			// Rule will go on true if sensor value is equal to the test value, false otherwise
#define RF_SENSOR_GT_SENSOR   128			// Rule will go on true if sensor value is greater than the reference sensor value, false otherwise
#define RF_SENSOR_GTE_SENSOR  129			// Rule will go on true if sensor value is greater than or equal to the reference sensor value, false otherwise
#define RF_SENSOR_LT_SENSOR   130			// Rule will go on true if sensor value is less than the reference sensor value, false otherwise
#define RF_SENSOR_LTE_SENSOR  131			// Rule will go on true if sensor value is less than or equal to the reference sensor value, false otherwise
#define RF_SENSOR_EQ_SENSOR   132			// Rule will go on true if sensor value is equal to the reference sensor value, false otherwise
#define RF_EVALUATE						127			// Rule will evaluate expression and go to the true/false action accordingly

//APDuino Rule Actions
#define APDRA_SET_OFF           0			// Turn off control (value 0 written to pin)
#define APDRA_SET_ON            1			// Turn on control (depending on type value 1/255 written to pin)
#define APDRA_SWITCH_VALUE      2			// Invert the value of the control pin (according to type)
#define APDRA_SET_VALUE         3			// Write a specific value to the pin (according to type, conversion might occur)
#define APDRA_RCSWITCH_ON       16		// Turn on an RCSwitch control (send on to the address)
#define APDRA_RCSWITCH_OFF      17		// Turn off an RCSwitch control (send off to the address)
#define APDRA_RCPLUG_SET_VALUE  18		// Write 0/1 to an RCPlug control (set on or off)
#define APDRA_RCPLUG_ON         19		// Turn RCPlug control ON
#define APDRA_RCPLUG_OFF			  20		// Turn RCPlug control OFF
#define APDRA_VIRT_CUST_FUNC    128		// reserved
#define APDRA_VIRT_SCREEN_NEXT  129		// reserved for hardware display paging
#define APDRA_VIRT_SYNCNTP      130		// reserved for NTP syncing
#define APDRA_NOOP              255		// Do nothing (can't hurt :))

// The following ControlActionMatrix definition is to support APDuino Online
// building up a Hash of Controls and Actions supported
// (could be used by APDuinOS too for validations, but concept is to provide
// valid configurations and spare validation cycles & code)
// CONTROL_ACTIONS_MATRIX = {  0 => [ 0, 1, 2, 3, 255 ],  #"ANALOG_CONTROL"
// 													   1 => [ 0, 1, 2, 3, 255 ],  #"DIGITAL_CONTROL",
//													   2 => [ 0, 1, 2, 3, 255 ],  #"RCSWITCH_CONTROL"
// 													   3 => [ 19, 20, 255 ],      #"RCPLUG_CONTROL"
// 												   127 => [ 255 ] 							#"SOFTWARE_CONTROL"
//  }
// furthermore: as Controls/Rules sources most likely will be reorganized in a
// Sensors-like fashion APDuino Online should always check versions and be prepared
// that from version X the extraction of this information must be based on other source files
#define CONTROL_ACTIONS_MATRIX = [ANALOG_CONTROL,[APDRA_SET_OFF,APDRA_SET_ON,APDRA_SWITCH_VALUE,APDRA_SET_VALUE,APDRA_NOOP],DIGITAL_CONTROL,[APDRA_SET_OFF,APDRA_SET_ON,APDRA_SWITCH_VALUE,APDRA_SET_VALUE,APDRA_NOOP],RCSWITCH_CONTROL,[APDRA_SET_OFF,APDRA_SET_ON,APDRA_SWITCH_VALUE,APDRA_SET_VALUE,APDRA_NOOP],RCPLUG_CONTROL,[APDRA_RCPLUG_ON,APDRA_RCPLUG_OFF,APDRA_NOOP],SOFTWARE_CONTROL,[APDRA_NOOP]]

// reexecute constants
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
  char *pszconditions;				// conditional expression to be evaluated
};

// The RULE_PARSERSTRING is used for parsing controls config files by new_control_parser
#define RULE_PARSERSTRING PSTR("%s %d,%d,%f,%d,%d,%d,%d,%d,%d,%d %s @%s")
// The following line needed for APDuino Online integration
// it should define valid Ruby code that will be executed
// in the context of the DeviceControl (self) when packaging configuration items
// The code must be inline with RULE_PARSERSTRING and new_rule_parser (APDRuleArray)
#define AO_RULE_PACKSTRING "\"#{self.label.gsub(/[ ]/,\"_\")[0..11]} #{self.evaluation_function_id},#{self.sensor.nil? ? -1 : self.sensor.sd_position},#{self.test_value},#{self.true_action_id},#{self.false_action_id},#{self.control.nil? ? -1 : self.control.control_index},#{self.control_value_mapping},#{self.control_value},#{self.control_sensor.nil? ? -1 : self.control_sensor.sd_position},#{self.reexec} #{self.conditions||'_'} @#{self.getcronspec||'_'}\""

class APDRule
{
public:
  APDRule();
  APDRule(RDCONF *rdc,APDSensorArray *pSA,APDControlArray *pca);      // initialize with RDCONF
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

  static boolean cronposeval(int curval,const char*pcpos);

private:
  void initBlank();
  boolean bLastState;
  APDSensorArray *psa;
  APDControlArray *pca;
  friend class APDRuleArray;
};

#endif /* APDRULE_H_ */
