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
 * apd_utils.h
 *
 *  Created on: Mar 26, 2012
 *      Author: George Schreiber
 */

#ifndef APD_UTILS_H_
#define APD_UTILS_H_

#include <Arduino.h>
#include <SdFat.h>               // SD card
#include "apd_version.h"
//#include <avr/io.h>
//#include <avr/wdt.h>
#define BUFSIZ 100

bool testprintf();
bool testscanf();
char *getPstr(const void *Pstring);           // you must free the address returned!

int get_line_count_from_file(const char *szFile);

/*double dewPointFast(double celsius, double humidity);
double dewPoint(double celsius, double humidity);
double Kelvin(double celsius);
double Fahrenheit(double celsius);*/

byte *hexbytes(const char *hexcode,byte *destbytes, int destbytes_count);
int read_hex_byte(const char *szByte);

// reset macro from http://support.atmel.com/bin/customer.exe?=&action=viewKbEntry&id=21
// TODO check why the code below causes debug to flash rapidly, device becomes unresponsive
//#define Reset_AVR() wdt_enable(WDTO_30MS); while(1) {}

#endif /* APD_UTILS_H_ */
