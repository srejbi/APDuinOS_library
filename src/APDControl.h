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
 * APDControl.h
 *
 *  Created on: Mar 26, 2012
 *      Author: George Schreiber
 */

#ifndef APDCONTROL_H_
#define APDCONTROL_H_

//APDUINO control types
#define ANALOG_CONTROL      0
#define DIGITAL_CONTROL     1
#define RCSWITCH_CONTROL    2
#define RCPLUG_CONTROL			3
#define SOFTWARE_CONTROL    127      // not used

#include <Arduino.h>
#include <Metro.h>
#include <RCSwitch.h>
#include "APDSerial.h"

// Control Definition Configuration ("record")
struct CDCONF {
  char label[13];
  int control_type;
  int control_pin;
  int initial_value;
  int control_log;
  char extra_data[24];
};

class APDControl {
public:
	APDControl();
	APDControl(CDCONF *cdc,APDControl *preusablecontrol);                 // setup control from definition
	virtual ~APDControl();
	char *getValueS(char *strdest);

	CDCONF config;
	void *control;
	int iValue;
	Metro *pmetro;
	void (*pcustfunc)();

	/* control actions */
	static void apd_action_request_layout(APDControl *pAPDControl, int iLayout);
	static void apd_action_next_screen(APDControl *pAPDControl, int iReserved);
	static void apd_action_sync_ntp(APDControl *pAPDControl, int iReserved);
	static void apd_action_noop(APDControl *pAPDControl, int iReserved);
	static void apd_write_control_pin(APDControl *pAPDControl, int iValue);
	static void apd_action_set_on(APDControl *pAPDControl, int iValue);
	static void apd_action_set_off(APDControl *pAPDControl, int iValue);
	static void apd_action_switch(APDControl *pAPDControl, int iValue);
	static void apd_action_set_value(APDControl *pAPDControl, int iValue);
	static void apd_action_custom_function(APDControl *pAPDControl, int iValue);
	static void apd_action_rc_switch_on(APDControl *pAPDControl, int iValue);
	static void apd_action_rc_switch_off(APDControl *pAPDControl, int iValue);
	static void apd_action_rc_plug_set_value(APDControl *pAPDControl, int iValue);
	static void apd_action_rc_plug_on(APDControl *pAPDControl, int iReserved);
	static void apd_action_rc_plug_off(APDControl *pAPDControl, int iReserved);
private:
	void initBlank();
	void *psharedclass;

	bool primary;
};

#endif /* APDCONTROL_H_ */
