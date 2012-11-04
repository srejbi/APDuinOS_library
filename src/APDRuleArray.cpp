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
 * APDRuleArray.cpp
 *
 *  Created on: Apr 6, 2012
 *      Author: George Schreiber
 */

#include "APDRuleArray.h"

APDRuleArray::APDRuleArray()
{
  this->pSA=NULL;
  this->pCA=NULL;
  this->pAPDRules=NULL;
  this->iRuleCount=0;
  this->bfIdle=NULL;
  this->lastCronMin = -1;
  this->nextrunmillis = millis() + 1000;
}

APDRuleArray::APDRuleArray(APDSensorArray *psa, APDControlArray *pca, float *bfidle)
{
  this->pSA=psa;
  this->pCA=pca;
  this->pAPDRules=NULL;
  this->iRuleCount=0;
  this->bfIdle=bfidle;
  this->lastCronMin = -1;
  this->nextrunmillis = millis() + 1000;
}

APDRuleArray::~APDRuleArray()
{
  // TODO Auto-generated destructor stub
  if (this->pAPDRules != NULL) {
      for (int i=0; i<this->iRuleCount; i++) {
          if (this->pAPDRules[i] != NULL) {
              delete(this->pAPDRules[i]);		// each rule was new
              this->pAPDRules[i] = NULL;
          }
      }
      free(this->pAPDRules);								// pAPDRules was malloc
      this->pAPDRules = NULL;
      this->iRuleCount=0;
  }
  //delete(this->pcronMetro);
}


void APDRuleArray::new_rule_parser(void *pRA, int iline, char *psz) {
  RDCONF rdc;
  // TODO check malloc results 2x
  rdc.pszcron = (char *)malloc(sizeof(char)*(MAX_CRON_EXPR_LEN+1));		// we might need this buffer for cron expr. if not, it will be freed
  rdc.pszconditions = (char *)malloc(sizeof(char)*(MAX_CRON_EXPR_LEN+1));		// we might need this buffer for cron expr. if not, it will be freed
#ifdef DEBUG
  Serial.print("RULE READ: "); Serial.print(psz);
#endif
  rdc.pszcron[0] = 0;
  rdc.pszconditions[0] = 0;
  //TODO add counter & checks on scanned parameters
  int iscand = sscanf_P( psz, RULE_PARSERSTRING,
      (rdc.label),
      &(rdc.rule_definition),
      &(rdc.rf_sensor_idx),
      &(rdc.rf_value),
      &(rdc.rule_true_action),
      &(rdc.rule_false_action),
      &(rdc.rule_control_idx),
      &(rdc.value_mapping),
      &(rdc.ra_value),
      &(rdc.ra_sensor_idx),
      &(rdc.reexec),
      (rdc.pszconditions),
      (rdc.pszcron));

if (iscand<13) {
	// we have failed to scan some params; candidates: conditions, pszcron
	// check if cron is in conditions (if config did not '_' empty strings)
	if (rdc.pszconditions[0]=='@') {		// if conditions start with '@' then '_' for empty conditions was omitted
		strcpy(rdc.pszcron, (const char *)(&rdc.pszconditions[1]));		// copy conditions after '@' to cron
		rdc.pszconditions[0]=0; // blank conditions
	}
}
  	// nice config should set '_' for empty strings
  	if (rdc.pszconditions[0]=='_'&&rdc.pszconditions[0]==0) rdc.pszconditions[0]=0;
    if (rdc.pszcron[0]=='_'&&rdc.pszcron[0]==0) rdc.pszcron[0]=0;

    // todo log this when enabled log levels (iscand) (" parameters parsed\n") (rdc.label) " -> cron:") (rdc.pszcron) (", conditions:'") (rdc.pszconditions) ("'\n");
// todo remove deprecated code
    // }

//  if (strlen(rdc.pszcron)) {		// TODO think about this; it will probably never be empty as it's followed by conditions(possibly). we likely have to enclose this...
//  	for (int i=0; i<strlen(rdc.pszcron); i++) {
//  		if (rdc.pszcron[i] == '_') rdc.pszcron[i] = ' ';		// "split" on underscores
//  	}
//  }

  ((APDRuleArray*)pRA)->pAPDRules[iline] = new APDRule(&rdc,((APDRuleArray*)pRA)->pSA, ((APDRuleArray*)pRA)->pCA);
  free(rdc.pszcron);						// no longer need the string buffer
  free(rdc.pszconditions);			// no longer need the string buffer
  //TODO check for errors and use an internal (class) index to keep track of the next rule to be populated

  // now do something with the values parsed...
}

int APDRuleArray::loadRules() {
  if (!this->pAPDRules) {    // if no sensor array
  	char szConfFile[32] = "";
  	strcpy_P(szConfFile,PSTR("RULES.CFG"));
  	APDDebugLog::log(APDUINO_MSG_LOADINGRULES,szConfFile);
      // TODO check if SD is available!
      iRuleCount = get_line_count_from_file(szConfFile);
      // TODO should log this
      char sztemp[10]="";
      APDDebugLog::log(APDUINO_MSG_RULECOUNT,itoa(iRuleCount,sztemp,10));
      if (iRuleCount > 0) {
      	// todo log this when enabled log levels ("Rule Array: allocating ") sizeof(APDRule*)*iRuleCount (" bytes of RAM\n")
        pAPDRules = (APDRule**)malloc(sizeof(APDRule*)*iRuleCount);

        if (pAPDRules) {
          memset(pAPDRules,0,sizeof(APDRule*)*iRuleCount);
          // todo log this when enabled log levels ("Rule Array allocated. Populating from RULES.CFG...\n")
          APDStorage::readFileWithParser(szConfFile,&new_rule_parser,(void*)this);

          APDDebugLog::log(APDUINO_MSG_RULESLOADED,NULL);
          // postprocessing of sensor & control pointers
          // todo log this when enabled log levels ("Rules postprocessing.\n")
          for (int i=0; i<iRuleCount; i++) {
            if (pAPDRules[i]->config.rf_sensor_idx > -1 && pAPDRules[i]->config.rf_sensor_idx < this->pSA->iSensorCount) {
                // mapping with Sensor Array
            	// todo log this when enabled log levels SerPrintP("Setting sensor idx"); Serial.print(pAPDRules[i]->config.rf_sensor_idx, DEC); SerPrintP("for rule idx"); Serial.println(i,DEC);
                pAPDRules[i]->psensor = (this->pSA->pAPDSensors[pAPDRules[i]->config.rf_sensor_idx]);
            }

            // mapping sensor value for control input, if specified
            if (pAPDRules[i]->config.ra_sensor_idx >= 0 && pAPDRules[i]->config.ra_sensor_idx < this->pSA->iSensorCount) {
            	// todo log this when enabled log levels SerPrintP("Setting sensor idx"); Serial.print(pAPDRules[i]->config.ra_sensor_idx, DEC); SerPrintP("for control value for rule "); Serial.println(i,DEC);
                pAPDRules[i]->pcsensorvalue = &(this->pSA->pAPDSensors[pAPDRules[i]->config.ra_sensor_idx]->fvalue);
            }

           //idling trick
            if (pAPDRules[i]->config.rule_definition == RF_IDLE_CHECK) {
            	// todo log this when enabled log levels SerPrintP("Setting pointer for idle bit");
                // internal idling "sensor"
                pAPDRules[i]->pcsensorvalue = this->bfIdle;
            }

            // mapping Control Array
            // todo log this when enabled log levels SerPrintP("Setting control idx"); Serial.print(pAPDRules[i]->config.rule_control_idx, DEC); SerPrintP("for rule "); Serial.println(i,DEC);
            pAPDRules[i]->pcontrol = (this->pCA->pAPDControls[pAPDRules[i]->config.rule_control_idx]);

            // todo remove relocated (deprecated here) code block
            //custom functions (only on true branch)
  //          if (pAPDRules[i]->config.rule_true_action == APDRA_VIRT_CUST_FUNC) {
  //              // THIS HAS TO BE SET ON THE APDUINO LEVEL
  //              SerPrintP("Setting a custom function");
  //              if (pAPDRules[i]->pcontrol != NULL && pAPDRules[i]->pcontrol->config.control_type == SOFTWARE_CONTROL ) {
  //                  if (pAPDRules[i]->pcontrol->config.control_pin < 10 && pAPDRules[i]->pcontrol->config.control_pin > 0) {
  //                      if (this->pcustfuncs[pAPDRules[i]->pcontrol->config.control_pin] != NULL) {
  //                          // assign the custom function pointer to the control; take control's PIN as IDX
  ////#ifdef DEBUG
  //                          SerPrintP("SETTING CUSTOM FUCTION IDX "); Serial.print(pAPDRules[i]->pcontrol->config.control_pin);
  //                          SerPrintP(" FOR CONTROL IDX"); Serial.print(pAPDRules[i]->config.rule_control_idx);
  //                          SerPrintP(".\n");
  ////#endif
  //                          (pAPDRules[i]->pcontrol)->pcustfunc = this->pcustfuncs[pAPDRules[i]->pcontrol->config.control_pin];      // cvalue must hold the cfunc idx
  //                      } else {
  //                          SerPrintP("NO CUSTOM FUNCTION PTR AT SPECIFIED INDEX. DID YOU ADD CUSTOM FUNC?");
  //                      }
  //                  } else {
  //                      SerPrintP("INVALID CUSTOM FUNCTION INDEX")
  //                  }
  //              } else {
  //                  SerPrintP("INVALID CONTROL.")
  //              }
  //          }
          }       // end enumerating rules
          // todo remove invalid log message
          APDDebugLog::log(APDUINO_MSG_RULESPOSTPROCESSED,NULL);

          this->nextrunmillis += 1000;
        } else {
        	APDDebugLog::log(APDUINO_ERROR_RAALLOCFAIL,NULL);
        }
      } else {
      	APDDebugLog::log(APDUINO_ERROR_RANORULES,NULL);
      }
    } else {
    	APDDebugLog::log(APDUINO_ERROR_RAALREADYALLOC,NULL);
    }
}

// TODO this function is out of structure-sync...
void APDRuleArray::dumpToFile(char *pszFileName) {
  // make a string for assembling the data to log:
	// todo log this when enabled log levels SerPrintP("Dumping Rule Array Config...");
    if (APDStorage::p_sd->exists(pszFileName)) {
          APDStorage::p_sd->remove(pszFileName);
        }
  SdFile dataFile(pszFileName, O_WRITE | O_CREAT );
  if (dataFile.isOpen()) {
    for (int i=0; i<iRuleCount; i++) {
      char line[RCV_BUFSIZ]="";
      APDRule *pr = pAPDRules[i];
      // TODO update with recent fields
      sprintf_P(line,PSTR("%s %d,%d,%f,%d,%d,%d,%d,%d,%d"),
          pr->config.label, pr->config.rule_definition,
          pr->config.rf_sensor_idx,
          pr->config.rf_value,
          pr->config.rule_true_action,
          pr->config.rule_false_action,
          pr->config.rule_control_idx,
          pr->config.value_mapping,
          pr->config.ra_value,
          pr->config.ra_sensor_idx);
      dataFile.println(line);
    }
    dataFile.close();
    // todo log this when enabled log levels SerPrintP("Rule Array Config dumped.");
  }
  else {
  	APDDebugLog::log(APDUINO_ERROR_RADUMPOPENFAIL,NULL);
  }
}


APDRule *APDRuleArray::firstRuleBySensorIdx(int iSensorIdx) {
// todo
}

APDRule *APDRuleArray::firstRuleByControlIdx(int iControlIdx) {
// todo
}

void APDRuleArray::evaluateSensorRules(void *pra, APDSensor *pSensor) {
	if (pra != NULL && pSensor->fvalue != NAN) {			// todo add switch on nan?
		APDRuleArray *pRA = (APDRuleArray *)pra;

		int iSensorIndex = pRA->pSA->indexBySensor(pSensor);
		if (iSensorIndex >= 0) {
			for (int i=0; i < pRA->iRuleCount; i++) {      // loop through rules
				if (pRA->pAPDRules[i]->config.rf_sensor_idx == iSensorIndex || pRA->pAPDRules[i]->config.ra_sensor_idx == iSensorIndex) {    // if rule is bound to the sensor either as trigger or input value for control
					if (pRA->pAPDRules[i]->config.rule_definition != RF_SCHEDULED) pRA->pAPDRules[i]->evaluateRule();	// evaluate but scheduled rules
				}  // evaluate if rule is for sensor
			}  // loop rules
		}
		//this->pRuleMetro->reset();    // restart the metro
	} else {
		// do nothing on NAN
	}
}

void APDRuleArray::evaluateSensorRulesByIdx(int iSensorIndex) {
	// todo log this when enabled log levels ("RULEEVAL!");
  if (this->pSA->pAPDSensors[iSensorIndex]->fvalue != NAN) {
		for (int i=0; i < this->iRuleCount; i++) {      // loop through rules
			if (this->pAPDRules[i]->config.rf_sensor_idx == iSensorIndex || this->pAPDRules[i]->config.ra_sensor_idx == iSensorIndex) {    // if rule is bound to the sensor either as trigger or input value for control
				// todo log this when enabled log levels SerPrintP("RULE_EVAL:"); Serial.print(this->pAPDRules[i]->config.label);
				if (this->pAPDRules[i]->config.rule_definition != RF_SCHEDULED) this->pAPDRules[i]->evaluateRule(); // evaluate but scheduled rules
			}  // evaluate if rule is for sensor
		}  // loop rules
		//this->pRuleMetro->reset();    // restart the metro
  } else {
  	// do nothing if sensor value is NAN
  }
}


void APDRuleArray::loopRules() {
	// todo log this when enabled log levels SerPrintP("loop rules\n");
	evaluateScheduledRules();
  for (int i=0; i < this->iRuleCount; i++) {      // loop through rules
  	if (this->pAPDRules[i]->config.rule_definition != RF_SCHEDULED) {	// except scheduled rules
  		this->pAPDRules[i]->evaluateRule();
  	}
  }

  //pRuleMetro->reset();    // restart the metro
}

// sets the nextrunmillis to the millis remaining till the next minute 0s
void APDRuleArray::adjustnextcronminute() {
	DateTime now = APDTime::now();
	this->lastCronMin = now.minute();
	this->nextrunmillis = millis();
	this->nextrunmillis += ((unsigned long)(60 - now.second())*1000);
}

// iterate through RF_SCHEDULED rules and execute the scheduler evaluation
// as this should be done 1x a minute, nextrunmillis is checked against millis
// (and readjusted to next run if we did a check)
void APDRuleArray::evaluateScheduledRules() {
	if (this->nextrunmillis < millis()) {		// check if time's up
		if (this->lastCronMin == -1) {					// if cron has been just started, we evaluate next 00:00
			adjustnextcronminute();								// push nextrunmillis to next 0s
			char sztmp[10] = "";
			APDDebugLog::log(APDUINO_MSG_CRONLAUNCHED,ultoa(nextrunmillis,sztmp,10));
			return;
		}

		DateTime now = APDTime::now();
		if (now.minute() != this->lastCronMin) {
			APDDebugLog::log(APDUINO_MSG_CRONCHECK,NULL);
			for (int i=0; i < this->iRuleCount; i++) {      // loop through rules
				if (this->pAPDRules[i]->config.rule_definition == RF_SCHEDULED) {	// only check scheduled
					this->pAPDRules[i]->evaluateRule();
				}
			}
		}
		adjustnextcronminute();
	}
}
