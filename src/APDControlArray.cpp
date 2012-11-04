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
 * APDControlArray.cpp
 *
 *  Created on: Apr 6, 2012
 *      Author: George Schreiber
 */

#include "APDControlArray.h"

APDControlArray::APDControlArray()
{
  // TODO Auto-generated constructor stub
  this->pAPDControls = NULL;
  this->iControlCount = 0;
  this->pcustfuncs = NULL;
}

APDControlArray::APDControlArray(void *pcustomfunctions)
{
  // TODO Auto-generated constructor stub
  this->pAPDControls = NULL;
  this->iControlCount = 0;
  this->pcustfuncs = (void **)pcustomfunctions;
}

APDControlArray::~APDControlArray()
{
  // TODO Auto-generated destructor stub
  if (this->pAPDControls != NULL) {
      for (int i=0; i<this->iControlCount; i++) {
          if (this->pAPDControls[i] != NULL) {
              delete(this->pAPDControls[i]);		// each Control was new by 'new_control_parser'
              this->pAPDControls[i] = NULL;
          }
      }
      free(this->pAPDControls);								// pAPDControls was malloc'd
      this->pAPDControls = NULL;
      this->iControlCount=0;
  }
}

void APDControlArray::new_control_parser(void *pCA, int iline, char *psz) {
  CDCONF cdc;
  // todo log this when enabled log levels "CONTROL READ: " (psz)
  int iscand = sscanf_P( psz, CONTROL_PARSERSTRING,
      (cdc.label),
      &(cdc.control_type),
      &(cdc.control_pin),
      &(cdc.initial_value),
      &(cdc.control_log),
      (cdc.extra_data));

  if (iscand < 6) {
  	// todo log this when enabled log levels "no extra data"
		memset(cdc.extra_data,0,sizeof(char)*24);
	}
	if (iscand < 5) {
		// todo log this when enabled log levels "no logging"
		cdc.control_log = 0;
	}

  APDControl *newcontrol = NULL;
  APDControl *preusablecontrol = ((APDControlArray *)pCA)->findReusableControl(&cdc);
  newcontrol = new APDControl(&cdc,preusablecontrol);
  ((APDControlArray*)pCA)->pAPDControls[iline] = newcontrol;

  //TODO check for errors and use an internal (class) index to keep track of the next rule to be populated

  // now do something with the values parsed...
}

int APDControlArray::loadControls() {
  if (!this->pAPDControls) {    // if no sensor array
  	char szConfFile[32] = "";
  	strcpy_P(szConfFile,PSTR("CONTROLS.CFG"));
  	APDDebugLog::log(APDUINO_MSG_LOADINGCONTROLS,szConfFile);						// debug
    // TODO check if SD is available!
    iControlCount = get_line_count_from_file(szConfFile);
    char sztemp[11]="";
    APDDebugLog::log(APDUINO_MSG_CONTROLCOUNT,itoa(iControlCount,sztemp,10));
    if (iControlCount > -1) {
    	// todo log this when enabled log levels "Control Array: allocating " (sizeof(APDControl*)*iControlCount)
      pAPDControls = (APDControl**)malloc(sizeof(APDControl*)*iControlCount);

      if (pAPDControls) {
        memset(pAPDControls,0,sizeof(APDControl*)*iControlCount);

        // todo log this when enabled log levels "CA Allocated. Parsing CONTROLS.CFG..."

        APDStorage::readFileWithParser(szConfFile,&new_control_parser,(void*)this);

        APDDebugLog::log(APDUINO_MSG_CONTROLSLOADED,NULL);

        // TODO add any postprocessing
        for (int i=0; i<iControlCount; i++) {
            APDControl *pc = pAPDControls[i];
          if (pc != NULL && pc->config.control_type == SOFTWARE_CONTROL ) {
            if (pc->config.control_pin < 10 && pc->config.control_pin > 0) {
              if (this->pcustfuncs[pc->config.control_pin] != NULL) {
                // assign the custom function pointer to the control; take control's PIN as IDX
              	// todo log this when enabled log levels "SETTING CUSTOM FUCTION IDX " pc->config.control_pin " FOR CONTROL IDX" i
                pc->pcustfunc = (void (*)())this->pcustfuncs[pc->config.control_pin];      // cvalue must hold the cfunc idx
              } else {
              	APDDebugLog::log(APDUINO_ERROR_CAMISSINGCUSTFUNC,NULL);
              }
            } else {
            	APDDebugLog::log(APDUINO_ERROR_CAINVALIDCUSTFUNC,NULL);
            }
          }
        }

        APDDebugLog::log(APDUINO_MSG_CONTROLSPOSTPROCESSED,NULL);

      } else {
      	APDDebugLog::log(APDUINO_ERROR_CAALLOCFAIL,NULL);
      }
    } else {
    	APDDebugLog::log(APDUINO_ERROR_CANOCONTROLS,NULL);
    }
  } else {
  	APDDebugLog::log(APDUINO_ERROR_CAALREADYALLOC,NULL);
    // TODO should implement cleanup and reload (?)
  }
}

int APDControlArray::dumpToFile(char *pszFileName) {
  // make a string for assembling the data to log:
	// todo log this when enabled log levels "Dumping CA Config..."
    if (APDStorage::p_sd->exists(pszFileName)) {
          APDStorage::p_sd->remove(pszFileName);
        }
  SdFile dataFile(pszFileName, O_WRITE | O_CREAT );
  if (dataFile.isOpen()) {
    for (int i=0; i<iControlCount; i++) {
      char line[RCV_BUFSIZ]="";
      APDControl *pc = pAPDControls[i];
      // TODO update with recent fields
      sprintf_P(line,PSTR("%s %d,%d,%d"),
          (pc->config.label),
          (pc->config.control_type),
          (pc->config.control_pin),
          (pc->config.initial_value));
      dataFile.println(line);
    }
    dataFile.close();
    // todo log this when enabled log levels "Rule Control Config dumped."
  }
  else {
    // TODO add an error macro in storage, replace all error opening stuff with reference to that
  	APDDebugLog::log(APDUINO_ERROR_CADUMPOPENFAIL,pszFileName);
  }

}

APDControl *APDControlArray::firstControlByPin(int iPin, int iType) {
	APDControl *pcont = NULL;
	for (int i=0; i<this->iControlCount && this->pAPDControls[i] != NULL; i++) {
			if (this->pAPDControls[i]->config.control_type == iType &&
					this->pAPDControls[i]->config.control_pin == iPin) {
					pcont = this->pAPDControls[i];
					break;
			}
	}
	return pcont;
}

APDControl *APDControlArray::findReusableControl(CDCONF *cdc) {
	APDControl *preusable = NULL;
  switch (cdc->control_type) {
  // TODO: put all reusable controls below
  case RCPLUG_CONTROL:
    preusable = this->firstControlByPin(cdc->control_pin, cdc->control_type);
    // todo log this when enabled log levels (the VERBOSE defines below)
#ifdef VERBOSE
    if (preusable) {
        SerPrintP("REUSING @");
        Serial.print((unsigned int)preusable,HEX);
    }
#endif
    break;
  default:
  	;
#ifdef VERBOSE
    SerPrintP("NOT ");
#endif
  }
#ifdef VERBOSE
  SerPrintP("REUSABLE");
#endif
  // todo log this - the above VERBOSES - when enabled log levels
  return preusable;
}

APDControl *APDControlArray::byIndex(int idx) {
	APDControl *ret = NULL;
  if (idx >= 0 && idx <= this->iControlCount) {
  	ret = this->pAPDControls[idx];
  }
  return ret;
}
