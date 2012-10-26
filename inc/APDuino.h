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
#include "APDSerial.h"
#include <MemoryFree.h>
#include <Metro.h>
#include <RTClib.h>



class APDuino {
public:
	APDuino();
	APDuino(long baudrate);
	virtual ~APDuino();

	unsigned long getUpTime();
	char *getUpTimeS(char *psz_uptime);

  boolean storage_ready();

	void loop();
	void loop_operations();
	void loop_core();

	void setupTimeKeeping();        // sets up the timekeeping source
	void checkTimeKeeping();        // checks if timekeeping needs an update

	void setupNetworking();         // sets up the network, from config file or DHCP fallback

	boolean startWebServer();          // starts the local www interface
	boolean startLogging(unsigned long ulLoggingFreq);                   // starts logging to SD

	/* start config line parser callbacks */
	static void new_ethconf_parser(void *pAPD, int iline, char *psz);
	/* end config line parser callbacks */

	void Print(char *string);
	void PrintP(void *Pstring);
	void Debug(char *string, int iMsgLevel);
	void DebugP(void *Pstring, int iMsgLevel);

	void startIdling(unsigned long uIdleDuration);
	void log_data();

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

	int iDebugLevel;
	boolean bAPDuinoConfigured;

	Metro *pIdleMetro;
	Metro *pLoggingMetro;

	void (*pcustfuncs[10])() ;                        // allow 10 custom functions to be called
	int AddCustomFunction(int iPos, void (*pcf)());

	void setupWithStorage(int iChip, int iSpeed);
};

#endif /* APDUINO_H_ */
