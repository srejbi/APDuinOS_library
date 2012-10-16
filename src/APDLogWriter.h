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
 * APDLogWriter.h
 *
 *  Created on: Oct 15, 2012
 *      Author: George Schreiber
 */


#ifndef APDLOGWRITER_H_
#define APDLOGWRITER_H_

#include <Arduino.h>
#include "APDDebugLog.h"
#include "APDStorage.h"
#include "apd_utils.h"

class APDLogWriter {
public:
	static char szlogfname[13];

	static void begin();				// TODO add begin with filename
	static void enable_sync_writes();
	static void disable_sync_writes();
	static void log_writer_function(const char *pszLog);
	static void write_debug_log();
};

#endif /* APDLOGWRITER_H_ */
