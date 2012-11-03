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
 * apd_utils.cpp
 *
 *  Created on: Mar 26, 2012
 *      Author: George Schreiber
 */

#include "apd_utils.h"

bool testprintf()  {
  char sztest[10] ="";
  sprintf_P(sztest,PSTR("%f"), 2012.0326);
  if (sztest[0] == '?') { // minimal version :(
    return false;
  } else if (sztest[0] == '2' ) {	// float-capable :)
    return true;
  }
  return false; // should never get here...
}

bool testscanf()  {
  float ftest = 0;
  sscanf_P("2012.0326", PSTR("%f"), &ftest);
  if (ftest == 0) {
    return false;
  } else if (ftest == 2012.0326 ) {
    return true;
  }
  return false;  // should never get here...
}



// count the lines in a file
// will be used as a primitive record counter for config files
// assuming that config files are correct (one item per line in the appropriate format)
// \return the number of lines counted, -1 on error
int get_line_count_from_file(const char *szFile) {
  int i=0;
  char line[RCV_BUFSIZ]="";
  int bread=0;
  SdFile dataFile(szFile, O_RDONLY );
  if (dataFile.isOpen()) {
    while (bread=dataFile.fgets(line, sizeof(line))) {      // get the next line
      if (strlen(line)) { i++; }
    }
    dataFile.close();
  } else {
    return -1;    // negative rc means error
  }
  return i;
}



char *apsprintf(char * __restrict str, char const * __restrict fmt, ...)
{
    int ret;
    va_list ap;

    va_start(ap, fmt);
    //ret = vsnprintf(str, SHRT_MAX, fmt, ap);
    va_end(ap);
    return (char *)(ret);
}








/*

// Dew Point calculations - based on http://arduino.cc/playground/Main/DHT11Lib
//
// DewPoint functions
// The sample sketch shows two dewPoint functions. One more professional (NOAA - based) and
// a faster one called dewPointFast(). This latter is smaller and 5x faster and has a maximum
// error of 0.6544 C compared to the NOAA one. As the DHT11 sensor has ~ 1% accuracy the
// fast version might be accurate enough for most applications.
//
//   FILE:  dht11_test1.pde
// PURPOSE: DHT11 library test sketch for Arduino
//

//Celsius to Fahrenheit conversion
double Fahrenheit(double celsius)
{
  return 1.8 * celsius + 32;
}

//Celsius to Kelvin conversion
double Kelvin(double celsius)
{
  return celsius + 273.15;
}

// dewPoint function NOAA
// reference: http://wahiduddin.net/calc/density_algorithms.htm
double dewPoint(double celsius, double humidity)
{
  double A0= 373.15/(273.15 + celsius);
  double SUM = -7.90298 * (A0-1);
  SUM += 5.02808 * log10(A0);
  SUM += -1.3816e-7 * (pow(10, (11.344*(1-1/A0)))-1) ;
  SUM += 8.1328e-3 * (pow(10,(-3.49149*(A0-1)))-1) ;
  SUM += log10(1013.246);
  double VP = pow(10, SUM-3) * humidity;
  double T = log(VP/0.61078);   // temp var
  return (241.88 * T) / (17.558-T);
}

// delta max = 0.6544 wrt dewPoint()
// 5x faster than dewPoint()
// reference: http://en.wikipedia.org/wiki/Dew_point
double dewPointFast(double celsius, double humidity)
{
  double a = 17.271;
  double b = 237.7;
  double temp = (a * celsius) / (b + celsius) + log(humidity/100);
  double Td = (b * temp) / (a - temp);
  return Td;
}
*/

byte *hexbytes(char *hexcode,byte *destbytes, int destbytes_count) {
  byte *rbytes = 0;
  if (hexcode && strlen(hexcode) > 2 && strlen(hexcode)%2 == 0) {
    char *sptr = hexcode;    // will shift as string bytes (2chars)
    char abyte[3] = "";      // a byte as a string, 2 characters
    int i = 0;
    while (strlen(sptr) && i<destbytes_count) {
      strncpy(abyte,sptr,2);
      int ibyte = read_hex_byte(abyte);
      destbytes[i] = ibyte;
      sptr+=2;
      i++;
    }
    if (i==destbytes_count) {
      rbytes = destbytes;
    }
  }
  return rbytes;
}




int hex_digit_val(char c)
{
    if (c >= '0' && c <= '9') {
          return c - '0';
    } else if (c >= 'a' && c <= 'f') {
          return c - 'a' + 10;
    } else if (c >= 'A' && c <= 'F') {
          return c - 'A' + 10;
    } else {
          return -1;   // not a hex digit
    }
}

int read_hex_byte(char *szByte)
{
    byte a = hex_digit_val(szByte[0]);
    byte b = hex_digit_val(szByte[1]);
    if (a<0 || b<0) {
          return -1;  // nonhex
    } else {
          return (a*16) + b;
    }
}


// you must free the address received!
char *getPstr(void *Pstring) {
  char *psob ;
  if (Pstring!=NULL) {
    int ilen = strlen_P((char*)Pstring);
    psob = (char*)malloc(sizeof(char)*ilen+1);
    if (psob != NULL) {
      memset(psob,0,sizeof(char)*ilen+1);
      strcpy_P(psob, (char*)Pstring);
    } else {
      Serial.print("OUT OF RAM. could not allocate "); Serial.print(ilen,DEC); Serial.println(" bytes.");
    }
  }
  return psob;
}


void soft_reset() {
	asm volatile ("  jmp 0");
}
