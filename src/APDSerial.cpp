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
 * APDSerial.cpp
 *
 *  Created on: Mar 27, 2012
 *      Author: George Schreiber
 */

#include "APDSerial.h"

APDSerial::APDSerial(long baudrate) {
	// TODO Auto-generated constructor stub
	Serial.begin(baudrate);
	Serial.println(APDUINO_MSG_SERIAL_INIT);			// todo replace this with future msg handler
	Serial.print(baudrate); SerPrintP(" bauds.\n");
}

APDSerial::~APDSerial() {
	// TODO Auto-generated destructor stub
	Serial.end();
}

void APDSerial::println(char *string) {
	Serial.println(string);
}

void APDSerial::print(char *string) {
	Serial.print(string);
}

void APDSerial::printlnP(void *Pstring) {
	myPrintP(Pstring,true);
}

void APDSerial::printP(void *Pstring) {
	myPrintP(Pstring,false);
}

void APDSerial::dumpIP(byte *ipaddr) {
	for (int i=0;i<4;i++) {
			Serial.print(ipaddr[i],DEC);
			SerPrintP(".");
	}
}


void APDSerial::myPrintP(void *Pstring,boolean bLn) {
  if (Pstring!=NULL) {
    //char sob[64] = "";
    int ilen = strlen_P((char*)Pstring);
    char *psob = (char*)malloc(sizeof(char)*ilen+1);
    if (psob != 0) {
      memset(psob,0,sizeof(char)*ilen+1);
      strcpy_P(psob, (char*)Pstring);
      Serial.print(psob);
      free(psob);
    } else {
    	Serial.print(APDUINO_ERROR_OUTOFRAM);		// todo replace this with the future error handler
//      Serial.print("OUT OF RAM. ("); Serial.print(ilen,DEC); Serial.println(")");
    }
  }
}
