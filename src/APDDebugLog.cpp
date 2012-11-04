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
 *  Created on: Oct 15, 2012
 *      Author: George Schreiber
 */

#include "APDDebugLog.h"

LOGITEM *APDDebugLog::bufstart = NULL;
LOGITEM *APDDebugLog::bufend = NULL;
int APDDebugLog::lostmessages = 0;
void (*APDDebugLog::plogwriterfunc)(const char *) = NULL;
void (*APDDebugLog::plogwriterfunc_off)(const char *) = NULL;
boolean APDDebugLog::logtoserial = true;			// todo this should be set in APDuino Config

// prints the debug code & message to Serial
void APDDebugLog::serialprint(unsigned int code, const char *psz_logstring){
	SerPrintP("0x"); Serial.print(code,HEX);
	if (psz_logstring) {
		SerPrintP(":");
		Serial.print(psz_logstring);
	}
	SerPrintP("-");
}

LOGITEM *APDDebugLog::makelog(unsigned int code, const char *psz_logstring){
	if (logtoserial) serialprint(code,psz_logstring);

	LOGITEM *newlog = (LOGITEM*)malloc(sizeof(LOGITEM));
	if (newlog) {
		SerPrintP(">");
		unsigned long tsmillis = millis();
		newlog->psz_logstring = (char *)malloc(sizeof(char)*((psz_logstring ? strlen(psz_logstring) : 0)+LOG_MESSAGE_OVERHEAD));
		newlog->pnext = NULL;

		if (newlog->psz_logstring) {
			SerPrintP("@");
			sprintf_P(newlog->psz_logstring,PSTR("%lu:0x%04x"), tsmillis, code);
			if (psz_logstring) {
				strcat_P(newlog->psz_logstring, PSTR(":"));
				strcat(newlog->psz_logstring, psz_logstring);
			}
			Serial.print(newlog->psz_logstring); SerPrintP("...");
		} else {
			serialprint(APDUINO_ERROR_LOGMSGOUTOFMEM,NULL);
		}
	} else {
		serialprint(APDUINO_ERROR_LOGITEMOUTOFMEM,NULL);
	}
	return newlog;
}

// will buffer / write to serial / write to SD (all and unbuffer) a log message
// buffer will be normally empty, if a writer function is set
// if wanting to debug SD operations, turn off writerfunc first
// logs will be buffered up
// reenable writerfunc when not working with the SD anymore
// and flush messages via APDLogWriter (or via pushing a new log message)
void APDDebugLog::log(unsigned int code, const char *psz_logstring) {
	if (freeMemory() < MIN_FREE_RAM && bufstart) {		// if low RAM, then FIFO flush
		flush_first();
		lostmessages++;				// keep track of dropped messages
	}
	// push to log buffer (will dump to serial)
  LOGITEM *newlog = makelog(code,psz_logstring);
 	if (newlog) {
		//delay(3);

		if (bufstart) {
			bufend->pnext = newlog;
			bufend = newlog;
		} else {
			bufstart = newlog;
			bufend = newlog;
		}

		// TODO auto-dump to log file if a writer is present
		if (plogwriterfunc) {
			while (bufstart) {
				SerPrintP(">");
				shifttowriter(plogwriterfunc);
				//delay(1);
			}
		} else SerPrintP("+");
	} else {
		serialprint(APDUINO_ERROR_LOGITEMOUTOFMEM,NULL);
		// todo toss out messages and retry
	}
	Serial.println();
}

// check if there is anything in the log buffer
bool APDDebugLog::is_empty() {
	return (bufstart == NULL);
}

// empty the log messages buffer
// return the number of elements flushed
int APDDebugLog::flush() {
	int iflushed = 0;
	while (bufstart) {
		flush_first();
		iflushed++;
	}
	return iflushed;
}

// flushes the first (oldest) log item
void APDDebugLog::flush_first() {
	if (bufstart) {
		free(bufstart->psz_logstring);
		LOGITEM *next = bufstart->pnext;
		free(bufstart);
		bufstart = next;
		if (bufstart==NULL) bufend = NULL;
	}
}

// enable synchronous write to SD by restoring the writer function
void APDDebugLog::enable_sync_writes() {
	plogwriterfunc = plogwriterfunc_off;
}

// disable synchronous write to SD by archiving the writer function
void APDDebugLog::disable_sync_writes() {
	plogwriterfunc_off = plogwriterfunc;
	plogwriterfunc = NULL;
}

// sets the log writer function (will enable writing to SD)
void APDDebugLog::setlogwriter(void (*writerfunc)(const char *)) {
	plogwriterfunc = writerfunc;
}

//
void APDDebugLog::shifttowriter(void (*writerfunc)(const char *)) {
	if (lostmessages) {
		char sztmp[11] = "";
		LOGITEM *newlog = makelog(APDUINO_WARN_MESSAGESFLUSHED,itoa(lostmessages,sztmp,10));
		if (writerfunc) (*(writerfunc))((const char *)bufstart->psz_logstring);
		free(newlog->psz_logstring);		// just clean up newlog string
		free(newlog);										// and newlog, it was never added to the logbuf
		lostmessages = 0;		// reset flushed message counter
	}
	//SerPrintP(">");
	if (writerfunc) (*(writerfunc))((const char *)bufstart->psz_logstring);
	flush_first();		// flush anyway
}
