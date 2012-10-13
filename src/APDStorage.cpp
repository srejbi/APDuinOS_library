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
 * APDStorage.cpp
 *
 *  Created on: Mar 27, 2012
 *      Author: George Schreiber
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

#include "APDStorage.h"
#include "APDSerial.h"

APDStorage::APDStorage(int iSS, int iChip, int iSpeed = SPI_HALF_SPEED ) {
  // TODO Auto-generated constructor stub
  sdChipSelect = iChip;
  iSSPin = iSS;		// SS_PIN
  iSDSpeed =  (iSpeed == SPI_HALF_SPEED || iSpeed == SPI_FULL_SPEED) ? iSpeed : SPI_HALF_SPEED;

  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
#ifdef DEBUG
  SerPrintP("SS PIN: ") Serial.print(iSS,DEC); SerPrintP("(OUT+HIGH)");
#endif
  pinMode(iSS, OUTPUT);        // set SS PIN as output -- see SD readings
  digitalWrite(iSS, HIGH);     // turn off the W5100 chip -- see SD readings
  Serial.println(APDUINO_MSG_SSPINPREPARED,HEX);
#ifdef DEBUG
  SerPrintP(" - prepared...");
#endif
  p_sd = NULL;
  p_root = NULL;
  bReady = false;
}

APDStorage::~APDStorage() {
  // TODO Auto-generated destructor stub
  free(p_root);
  free(p_sd);
}

/** Attempts to start the storage with SdFat
 *
 */
boolean APDStorage::start() {
	Serial.println(APDUINO_MSG_STORAGEINIT,HEX);
#ifdef VERBOSE
  SerPrintP("Storage ");
#endif
  if (this->p_sd == NULL ) { // && !bReady
      this->bReady = false;
      Serial.println(APDUINO_MSG_STORAGESTART,HEX);
#ifdef VERBOSE
      SerPrintP(" starting.");
#endif
      this->p_sd = new SdFat();
      //SerPrintP("..");
      if (p_sd) {
      	Serial.println(APDUINO_MSG_SDFATINIT,HEX);
#ifdef VERBOSE
          SerPrintP("SdFat init("); Serial.print(iSDSpeed, DEC); SerPrintP(","); Serial.print(sdChipSelect,DEC);SerPrintP(")...");
#endif
          if (p_sd->init(iSDSpeed, sdChipSelect)) {
              // should be initialized
              p_root = new SdFile();
              p_root->openRoot(p_sd->vol());
              //root.openRoot(sd.vol());
#ifdef VERBOSE
              SerPrintP("OK.\n");
#endif
              Serial.println(APDUINO_MSG_SDFATSTARTED,HEX);
              bReady = true;
          } else {
              p_root = NULL;
              Serial.println(APDUINO_ERROR_SDFATSTARTERR,HEX);
#ifdef VERBOSE
              SerPrintP("ERR.\n");
#endif
              bReady = false;
          }
      }
  }
#ifdef DEBUG
  SerPrintP("APDStorage is");
  if (!bReady) SerPrintP(" Not");
  SerPrintP(" Ready.\n");
#endif
  return bReady;
}

/** Tell if storage is ready or not.
 *
 * \return true if storage is ready to use, false if not.
 */
boolean APDStorage::ready() {
  return bReady;
}

/** Rotate log files.
 *
 *
 * \return the number of files rotated
 */
int APDStorage::logrotate() {
  int iRetCode = -1;	// something wrong
  if (bReady) {
      // rename any old log file(s) -- logrotate
      char fname[13] = "";
      char ofname[13] = "";
      strcpy_P(fname,PSTR("APDLOG.TXT"));
      if (p_sd->exists(fname)) {
      	Serial.println(APDUINO_MSG_LOGCHECK,HEX);
#ifdef VERBOSE
          SerPrintP("APDLOG.TXT -- Log existst, checking size - \n");		// TODO remove these, do messaging via APDSerial
#endif
          // check file size
          SdFile logFile(fname, O_RDONLY);
          unsigned long fSize = logFile.fileSize();
          logFile.close();
          // File size check
#ifdef VERBOSE
          Serial.print(fSize); SerPrintP(" bytes...\n");				// TODO remove debug
#endif
          // 1M size check
          if (fSize >= 1048576) {											// TODO move max size to param
          	Serial.println(APDUINO_MSG_LOGROTATENEEDED,HEX);
#ifdef VERBOSE
              SerPrintP(" -- Too big, rotate needed.");					// TODO remove debug
#endif
              int ibak = 0;
              sprintf_P(fname, PSTR("APDLOG.%03d"), ibak);
              // do check for the last backup
              for (ibak = 0; ibak < 999 && p_sd->exists(fname); sprintf_P(fname, PSTR("APDLOG.%03d"), ++ibak)) {
#ifdef VERBOSE
                  Serial.print(fname); SerPrintP("\n exists already.");
#endif
              }
#ifdef VERBOSE
              Serial.print(fname); SerPrintP("should be the last log file. Rotating...\n");
#endif
              Serial.println(APDUINO_MSG_LOGROTATE,HEX);
              if (p_sd->exists(fname)) {
                  p_sd->remove(fname);
              }

              while (ibak > 0) {
                  sprintf_P(ofname,PSTR("APDLOG.%03d"),ibak-1);
                  sprintf_P(fname,PSTR("APDLOG.%03d"),ibak);
#ifdef VERBOSE
                  SerPrintP("Renaming "); Serial.print(ofname); SerPrintP(" to "); Serial.print(fname); SerPrintP(".\n");
#endif
                  p_sd->rename(ofname,fname);
                  ibak--;
                  iRetCode++;			// number of logs that will be rotated
              }
              // rename APDLOG.TXT to APDLOG.000 using PSTRINGS for filenames to save on RAM
              strcpy_P(ofname,PSTR("APDLOG.TXT"));
              strcpy_P(fname,PSTR("APDLOG.000"));
              p_sd->rename(ofname,fname);
              iRetCode++;				//+1
              Serial.println(APDUINO_MSG_LOGROTATED,HEX);
#ifdef VERBOSE
              Serial.print(iRetCode);SerPrintP("\n logs rotated.");
#endif
          } else {
              iRetCode = 0;		// no logs rotated
#ifdef VERBOSE
              SerPrintP("\n -- less than 1M in size, appending.");
#endif
          }
      } else {
          iRetCode = 0;		// no logs rotated
#ifdef VERBOSE
          SerPrintP("\nNo previous log.");
#endif
      }
#ifdef VERBOSE
      SerPrintP("\nShould be ok to open new log or append to the previous one.");
#endif
  }
  return iRetCode;
}


/** Reads lines from a file and passes it to a specified parser fuction.
 *
 * \param[in] szFile path to the file
 * \param[in] pParserFunc(void *,int,char*) pointer to the line parser function
 * \param[in] pAPD pointer to an APDuino instance (allowing access to APDuino data/methods)
 *
 *
 * \return -1 on error, the number of lines read otherwise.
 */
int APDStorage::readFileWithParser(char *szFile, void (*pParserFunc)(void *, int, char*), void *pAPD) {
  int retcode = -1;
#ifdef DEBUG
  SerPrintP("Reading "); Serial.print(szFile); SerPrintP(" with parser @");
  Serial.print((unsigned int)pParserFunc,DEC);
  SerPrintP("\n");
#endif

  int i=0;
  char line[BUFSIZ]="";
  int bread=0;

  SdFile dataFile(szFile, O_RDONLY );
  if (dataFile.isOpen()) {
      // TODO upgrade and also pass bread
    while ((bread=dataFile.fgets(line, sizeof(line)))) {      // get the next line
      (*pParserFunc)(pAPD,i,line);                                           // pass it to the parser
      i++;
    }
    dataFile.close();
    retcode = i;
  } else {
  	Serial.print(APDUINO_ERROR_FILEOPEN);SerPrintP(":");Serial.println(szFile);
      //SerPrintP("E219 ("); Serial.print(szFile); Serial.println(")");     // error opening file
  }
  return retcode;
}



void APDStorage::write_log_line(char *szLogLine) {
  if (bReady) {
    // make a string for assembling the data to log:
  	Serial.println(APDUINO_MSG_SDLOGGING,HEX);
#ifdef VERBOSE
    SerPrintP("\nLogging to SD...\n");
#endif

    SdFile dataFile("APDLOG.TXT", O_WRITE | O_CREAT | O_APPEND);
    if (dataFile.isOpen()) {
        dataFile.println(szLogLine);
        dataFile.close();
    }
    // if the file isn't open, pop up an error:
    else {
    	Serial.println(APDUINO_ERROR_LOGOPENERR,HEX);
        //SerPrintP("E220\n");			// could not open log file
    }
  } else {
  		Serial.println(APDUINO_ERROR_LOGSDERR,HEX);
      //SerPrintP("E201\n");			// storage not ready
  }
  // return nothing
}
