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
 * APDuino.h
 *
 *  Created on: Mar 26, 2012
 *      Author: George Schreiber
 */

#ifndef APDUINO_H_
#define APDUINO_H_

#include <Arduino.h>
#include "apd_version.h"
#include "apd_utils.h"
#include <stdlib.h>
#include <avr/pgmspace.h>

#include "APDLogWriter.h"
#include "APDSensorArray.h"
#include "APDControlArray.h"
#include "APDRuleArray.h"
#include "APDSerial.h"
#include "APDStorage.h"
#include "APDSensor.h"
#include "APDControl.h"
#include "APDRule.h"
#include "APDTime.h"
#include "APDWeb.h"


class APDuino {
public:
	APDuino();
	APDuino(long baudrate);
	virtual ~APDuino();

	unsigned long getUpTime();
	char *get_uptime_str(char *psz_uptime);

	void loop();
	void loop_operations();
	void loop_core();
	void write_debug_log();

  boolean storage_ready();

	void setup_timekeeping();        // sets up the timekeeping source
	void check_timekeeping();        // checks if timekeeping needs an update

	void setup_networking();         // sets up the network, from config file or DHCP fallback
	boolean start_webserver();          // starts the local www interface
	boolean start_logging(unsigned long ulLoggingFreq);                   // starts logging to SD

	/* start config line parser callbacks */
	static void new_ethconf_parser(void *pAPD, int iline, char *psz);
	/* end config line parser callbacks */

	void print(char *string);
	void printP(void *Pstring);
	void debug(char *string, int iMsgLevel);
	void debugP(void *Pstring, int iMsgLevel);

	void start_idling(unsigned long uIdleDuration);
	void log_data();								// SD logger function

	boolean bConfigured();

	void unidle_device();
	void idle_device();

	DateTime timeNow();

	/* configuration items read into memory if present on SD, or defaults provided below */
	char *pstr_device_id;
	char *pstr_APDUINO_API_KEY;                         // the api key for apduino online
	char *pstr_Name;


	boolean bProcessRules;                   // rules on/off
	boolean toggleRuleProcessing();
	boolean enableRuleProcessing();
	boolean disableRuleProcessing();

	boolean reconfigure();						// reset application (reload configs)
	boolean reload_rules();						// reload rules

	APDSerial *pAPDSerial;
	APDWeb *pAPDWeb;

	int iDebugLevel;
	boolean bAPDuinoConfigured;

	Metro *pIdleMetro;
	Metro *pLoggingMetro;

	APDSensorArray *psa;                    // will replace APD Sensors pointer
	APDControlArray *pca;                    // will replace APD Controls pointer
	APDRuleArray *pra;                    // will replace APD Rules pointer

	volatile int iNextSensor;            // next sensor to poll, point to first one (if any)

	void (*pcustfuncs[10])() ;                        // allow 10 custom functions to be called
	int add_custom_function(int iPos, void (*pcf)());

	void setup_with_storage(int iChip, int iSpeed);

private:
	float bfIdle;                     // idle bit as float (it will be passed to rules as a sensor value)
	void init(long baudrate);

	//APDStorage *setupStorage(int iSS, int iChip, int iSpeed);
	bool setup_storage(int iSS, int iChip, int iSpeed);
	boolean init_app();

	boolean bInitialized;
	boolean bFirstLoopDone;
};

#endif /* APDUINO_H_ */
