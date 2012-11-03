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
 * APDLogWriter.cpp
 *
 *  Created on: Oct 15, 2012
 *      Author: George Schreiber
 */


#include "APDLogWriter.h"

char APDLogWriter::szlogfname[13] = "";

void APDLogWriter::begin() {
	//SerPrintP("APD LOG WRITER BEGIN");
	strcpy_P(szlogfname,PSTR("APDDEBUG.LOG"));
	// todo should allocate filename iso static, and copy szFileName
	APDStorage::rotate_file(szlogfname,0);
	//SerPrintP("APD LOG WRITER SHOULD HAVE ROTATED "); Serial.println(szlogfname);
	delay(200);
	if (APDStorage::ready()) {
		//SerPrintP("APD LOG WRITER SETTING LOG WRITER CALLBACK\n");
		APDDebugLog::setlogwriter(&(APDLogWriter::log_writer_function));
	}
}

void APDLogWriter::enable_sync_writes() {
	//SerPrintP("APD LOG WRITER ENABLING SYNCW\n");
	if (APDStorage::ready()) {
		APDDebugLog::setlogwriter(&(APDLogWriter::log_writer_function));
		APDDebugLog::enable_sync_writes();
	}
}

void APDLogWriter::disable_sync_writes() {
	//SerPrintP("APD LOG WRITER DISABLING SYNCW\n");
	APDDebugLog::disable_sync_writes();
}



void APDLogWriter::write_debug_log() {
	//SerPrintP("APD LOG WRITER WRITING ANY BUFFERED MESSAGES\n");
	while (!APDDebugLog::is_empty()) {
		//SerPrintP("APD LOG WRITER WRITING\n");
		APDDebugLog::shifttowriter(&(APDLogWriter::log_writer_function));
	}
}

void APDLogWriter::log_writer_function(const char *pszLog) {
	APDStorage::write_log_line(szlogfname,pszLog);
}

