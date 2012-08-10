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
}

void APDControlArray::new_control_parser(void *pCA, int iline, char *psz) {
  CDCONF cdc;
  Serial.print("CONTROL READ: "); Serial.print(psz);
  int iscand = sscanf( psz, "%s %d,%d,%d,%d,%s",
      (cdc.label),
      &(cdc.control_type),
      &(cdc.control_pin),
      &(cdc.initial_value),
      &(cdc.control_log),
      (cdc.extra_data));

  if (iscand < 6) {
		SerPrintP("no extra data");
		memset(cdc.extra_data,0,sizeof(char)*24);
	}
	if (iscand < 5) {
		SerPrintP("no logging");
		cdc.control_log = 0;
	}

  APDControl *newcontrol = NULL;
  APDControl *preusablecontrol = ((APDControlArray *)pCA)->findReusableControl(&cdc);
  newcontrol = new APDControl(&cdc,preusablecontrol);
  ((APDControlArray*)pCA)->pAPDControls[iline] = newcontrol;

  //TODO check for errors and use an internal (class) index to keep track of the next rule to be populated

  // now do something with the values parsed...
}

int APDControlArray::loadControls(APDStorage *pAPDStorage) {
  if (!this->pAPDControls) {    // if no sensor array
#ifdef DEBUG
    SerPrintP("Counting controls...");
#endif
    // TODO check if SD is available!
    iControlCount = get_line_count_from_file("CONTROLS.CFG");
#ifdef DEBUG
    Serial.print(iControlCount); SerPrintP(" controls seem to be defined...");
#endif
    if (iControlCount > -1) {
#ifdef DEBUG
      SerPrintP("Control Array: allocating "); Serial.print(sizeof(APDControl*)*iControlCount,DEC); SerPrintP(" bytes of RAM\n");
#endif

      pAPDControls = (APDControl**)malloc(sizeof(APDControl*)*iControlCount);

      if (pAPDControls) {
        memset(pAPDControls,0,sizeof(APDControl*)*iControlCount);

  #ifdef DEBUG
        SerPrintP("CA Allocated. Parsing CONTROLS.CFG...\n");
  #endif

        pAPDStorage->readFileWithParser("CONTROLS.CFG",&new_control_parser,(void*)this);
#ifdef DEBUG
        SerPrintP("CA Done.\n");
#endif
        // TODO add any postprocessing
        for (int i=0; i<iControlCount; i++) {
            APDControl *pc = pAPDControls[i];
          if (pc != NULL && pc->config.control_type == SOFTWARE_CONTROL ) {
            if (pc->config.control_pin < 10 && pc->config.control_pin > 0) {
              if (this->pcustfuncs[pc->config.control_pin] != NULL) {
                // assign the custom function pointer to the control; take control's PIN as IDX
#ifdef DEBUG
                SerPrintP("SETTING CUSTOM FUCTION IDX "); Serial.print(pc->config.control_pin);
                SerPrintP(" FOR CONTROL IDX"); Serial.print(i); SerPrintP(".\n");
#endif
                pc->pcustfunc = (void (*)())this->pcustfuncs[pc->config.control_pin];      // cvalue must hold the cfunc idx
              } else {
                  SerPrintP("E406");	// CA: missing custom function.
              }
            } else {
                SerPrintP("E405");		// CA: invalid custom function.
            }
          }
        }

      } else {
        SerPrintP("E403\n");	//CA alloc failed.
      }
    } else {
      SerPrintP("W402\n");	//No controls defined.
    }
  } else {
    SerPrintP("E401\n");		//CA already allocated.
    // TODO should implement cleanup and reload
  }
}

int APDControlArray::dumpToFile(APDStorage *pAPDStorage, char *pszFileName) {
  // make a string for assembling the data to log:
#ifdef DEBUG
  SerPrintP("Dumping CA Config...");
#endif
    if (pAPDStorage->p_sd->exists(pszFileName)) {
          pAPDStorage->p_sd->remove(pszFileName);
        }
  SdFile dataFile(pszFileName, O_WRITE | O_CREAT );
  if (dataFile.isOpen()) {
    for (int i=0; i<iControlCount; i++) {
      char line[BUFSIZ]="";
      APDControl *pc = pAPDControls[i];
      // TODO update with recent fields
      sprintf(line,"%s %d,%d,%d",
          (pc->config.label),
          (pc->config.control_type),
          (pc->config.control_pin),
          (pc->config.initial_value));
      dataFile.println(line);
    }
    dataFile.close();
#ifdef DEBUG
    SerPrintP("Rule Control Config dumped.");
#endif
  }
  else {
      // TODO add an error macro in storage, replace all error opening stuff with reference to that
    SerPrintP("E429('"); Serial.print(pszFileName); SerPrintP("')\n");	// error opening dumpfile
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
    if (preusable) {
        SerPrintP("REUSING @");
        Serial.print((unsigned int)preusable,HEX);
    }
    break;
  default:
    SerPrintP("NOT ");
  }
  SerPrintP("REUSABLE");
  return preusable;
}

APDControl *APDControlArray::byIndex(int idx) {
	APDControl *ret = NULL;
  if (idx >= 0 && idx <= this->iControlCount) {
  	ret = this->pAPDControls[idx];
  }
  return ret;
}
