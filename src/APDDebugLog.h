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
 * APDDebugLog.cpp
 *
 * TODO: add callback for display updates (allowing rerouting log to a hard display)
 *
 *  Created on: Oct 15, 2012
 *      Author: George Schreiber
 */

#ifndef APDDEBUGLOG_H_
#define APDDEBUGLOG_H_

#include <Arduino.h>
#include "APDSerial.h"
#include "MemoryFree.h"

#define LOG_MESSAGE_HEADER_LEN			20		// "<unsigned long:10chars>:(1char)0x(2chars)<unsigned int:(5chars)>":(1char)\0(1char)"
struct LOGITEM {
	char *psz_logstring;
	LOGITEM *pnext;
};

#define MIN_FREE_RAM					512			// if less, will flush a message (if any in the buff) on adding a new one

class APDDebugLog {
public:
	static void log(uint16_t code, const char *psz_logstring);
	static bool is_empty();
	static void setlogwriter(void (*writerfunc)(const char *));
	static void shifttowriter(void (*writerfunc)(const char *));
	static int flush();
	static void flush_first();

	static void enable_sync_writes();
	static void disable_sync_writes();

	// TODO add loglevel variable and related code that discards certain events (INFO/WARN/ERROR)
	static boolean logtoserial;
	static int loglevel;			// WARNING - todo
	static int logfilter;			// future use to filter SD logging to specific issues
	static LOGITEM *makelog(uint16_t code, const char *psz_logstring);
	static void serialprint(uint16_t code, const char *psz_logstring);

	static void set_loglevel(uint8_t newlevel) {	loglevel = newlevel; }
	static void set_logfilter(uint8_t newfilter) {	logfilter = newfilter; }
private:

	static void (*plogwriterfunc)(const char *);				// log writer callback when enabled
	static void (*plogwriterfunc_off)(const char *);		// saving callback while disabled
	static int lostmessages;
	static LOGITEM *bufstart;
	static LOGITEM *bufend;
};
extern void logwriterfunc(const char*logmessage);
#endif /* APDDEBUGLOG_H_ */
