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

#define SS_PIN		SS			// SS should be defined in SdFatLib

class APDStorage {
public:
	APDStorage(int iSS, int iChip, int iSpeed);
	virtual ~APDStorage();

	boolean start();
	boolean ready();

	int logrotate();
	void write_log_line(char *szLogLine);

	int readFileWithParser(char *szFile, void (*pParserFunc)(void*, int, char*), void *pAPD  );

	int sdChipSelect;
	int iSSPin;
	int iSDSpeed;

	SdFat *p_sd;                                 // SD fat used for file IO
	SdFile *p_root;                              // fileserver /webserver use

private:
	boolean bReady;
};

#endif /* APDSTORAGE_H_ */
