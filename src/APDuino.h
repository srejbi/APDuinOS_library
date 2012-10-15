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
	char *getUpTimeS(char *psz_uptime);

	/*int iLoadSensorCountFromSD();
	int iLoadRuleCountFromSD();
	int iLoadControlCountFromSD();*/

	void loop();
	void loop_operations();
	void loop_core();

  boolean storage_ready();

	void setupTimeKeeping();        // sets up the timekeeping source
	void checkTimeKeeping();        // checks if timekeeping needs an update

	void setupNetworking();         // sets up the network, from config file or DHCP fallback
	boolean startWebServer();          // starts the local www interface
	//boolean setupAPDuinoOnline(char *hostname, IPAddress *phostip, int port);    // sets up registration w/ APDuino Online server
	//boolean startWebLogging(unsigned long ulWebLoggingFreq);                   // starts weblogging to the registered server
	boolean startLogging(unsigned long ulLoggingFreq);                   // starts logging to SD

	/* start config line parser callbacks */
	//static void new_rule_parser(void *pAPD, int iline, char *psz);
	//static void new_control_parser(void *pAPD, int iline, char *psz);
	//static void new_sensor_parser(void *pAPD, int iline, char *psz);
	static void new_ethconf_parser(void *pAPD, int iline, char *psz);
	/* end config line parser callbacks */

	void Print(char *string);
	void PrintP(void *Pstring);
	void Debug(char *string, int iMsgLevel);
	void DebugP(void *Pstring, int iMsgLevel);

	//int setupRules();
	//int setupSensors();
	//int setupControls();

	void startIdling(unsigned long uIdleDuration);
	//void get_lastlog_string(char *szLogBuf);
	void log_data();

	boolean bConfigured();
	//int nextSensor();               // returns the next sensor index from array

	//void poll_apd_sensors();
	//void loop_apd_rules();
	//void evaluate_apd_rules_for_sensor(int iSensorIndex);

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

	APDSerial *pAPDSerial;
	//APDStorage *pAPDStorage;
	APDWeb *pAPDWeb;

	int iDebugLevel;
	boolean bAPDuinoConfigured;

	Metro *pIdleMetro;
	Metro *pLoggingMetro;

	APDSensorArray *psa;                    // will replace APD Sensors pointer
	APDControlArray *pca;                    // will replace APD Controls pointer
	APDRuleArray *pra;                    // will replace APD Rules pointer

	volatile int iNextSensor;            // next sensor to poll, point to first one (if any)
	//APDTime *pAPDTime;		// deprecating this as APDTime is being made static

	void (*pcustfuncs[10])() ;                        // allow 10 custom functions to be called
	int AddCustomFunction(int iPos, void (*pcf)());

	void setupWithStorage(int iChip, int iSpeed);
private:
	float bfIdle;                     // idle bit as float (it will be passed to rules as a sensor value)
	void init(long baudrate);

	//APDStorage *setupStorage(int iSS, int iChip, int iSpeed);
	bool setupStorage(int iSS, int iChip, int iSpeed);
	boolean initApplication();

	boolean bInitialized;
	boolean bFirstLoopDone;
};

#endif /* APDUINO_H_ */
