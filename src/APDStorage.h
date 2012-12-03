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
 * APDStorage.h
 *
 *   Created on: Mar 27, 2012
 *       Author: George Schreiber
 *
 * APDStorage is a wrapper around SdFat library
 * -- see http://code.google.com/p/sdfatlib/
 * to get APDStorage to compile, you must download SdFat Library
 * and include in the build (via compiler and linker includes)
 *
 * Credits: Arduino Sd2Card Library
 * Copyright (C) 2009 by William Greiman
 *
 */

#ifndef APDSTORAGE_H_
#define APDSTORAGE_H_

#include <Arduino.h>
#include "apd_utils.h"
#include "SdFat.h"               // SD card
#include "APDDebugLog.h"				// buffered logging

// TODO enclose hardware specific parts in the appropriate defines
#define SS_PIN		SS			// SS should be defined in SdFatLib

#define MAX_PATH_LEN		64		// the max length for path entries
#define MAX_LOG_SIZE    1048576		// 1M

class APDStorage {
public:
	//APDStorage(int iSS, int iChip, int iSpeed);
	//virtual ~APDStorage();

	static boolean begin(int iSS, int iChip, int iSpeed);
	static boolean ready();
	static void stop();

	static int rotate_file(const char *szLogFile, int backups, unsigned long maxsize);
	static void write_log_line(const char *szLogFile, const char *szLogLine);

	static int read_file_with_parser(char *szFile, void (*pParserFunc)(void*, int, char*), void *pAPD  );
	static uint64_t get_sd_free_cluster_bytes();

	static int sdChipSelect;
	static int iSSPin;
	static int iSDSpeed;

	static SdFat *p_sd;                                 // SD fat used for file IO
	static SdFile *p_root;                              // fileserver /webserver use

private:
	static boolean bReady;
};

#endif /* APDSTORAGE_H_ */
