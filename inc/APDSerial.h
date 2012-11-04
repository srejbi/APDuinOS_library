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
 * APDSerial.h
 *
 *  Created on: Mar 27, 2012
 *      Author: George Schreiber
 */

#ifndef APDSERIAL_H_
#define APDSERIAL_H_

#include "apd_utils.h"

class APDSerial {
public:
	APDSerial(long baudrate);
	virtual ~APDSerial();

	static void print(char *string);
	static void println(char *string);
	static void printP(void *Pstring);
	static void printlnP(void *Pstring);

	static void dumpIP(byte *ipaddr);

private:
	static void myPrintP(void *Pstring,boolean bLn);
};

#define SerPrintP(s) APDSerial::printP(PSTR(s));
#define SerPrintPln(s) APDSerial::printlnP(PSTR(s));
#define SerDumpIP(ip) APDSerial::dumpIP(ip);

#endif /* APDSERIAL_H_ */
