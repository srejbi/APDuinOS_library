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
 * APDRule.cpp
 *
 *  Created on: Mar 27, 2012
 *      Author: George Schreiber
 */

#include "APDRule.h"


APDRule::APDRule()
{
  // TODO Auto-generated constructor stub
  initBlank();
}

// initialize with RDCONF
APDRule::APDRule(RDCONF *rdc, APDSensorArray *pSA, APDControlArray *pCA) {
  initBlank();
  int l = 0;

  memcpy((void*)&(this->config),(void*)rdc,sizeof(RDCONF));       // copy the structure to config
  if (rdc->pszcron && (l = strlen(rdc->pszcron)) > 0 ) {
  	if (this->config.pszcron = (char *)malloc(sizeof(char)*(l+1))) {
  		strcpy(this->config.pszcron, rdc->pszcron);
  	} else {
  		APDDebugLog::log(APDUINO_ERROR_RAOUTOFMEM,NULL);
  	}
  } else {
  	this->config.pszcron = NULL;
  }
  if (rdc->pszconditions && (l = strlen(rdc->pszconditions)) > 0 ) {
  	if (this->config.pszconditions = (char *)malloc(sizeof(char)*(l+1))) {
  		strcpy(this->config.pszconditions, rdc->pszconditions);
  	} else {
  		APDDebugLog::log(APDUINO_ERROR_RAOUTOFMEM,NULL);
  	}
  } else {
  	this->config.pszconditions = NULL;
  }
  this->psa = pSA;
  this->pca = pCA;

// NOW's the time to do some tests (if any :p)
//            boolean (*prulefunc)(void *);   // the evaluation function pointer, rule
//            void (*ptcontrolfunc)(APD_CONTROL *,int);        // control, value
//            void (*pfcontrolfunc)(APD_CONTROL *,int);        // control, value
//            APD_SENSOR *psensor;                          // test - pointer to a sensor
//            float *pvalue;                                // test - pointer to a float value
//            float finternal;                              // if value testing, pvalue should point to finternal, or to a sensor's float value, otherwise
//            APD_CONTROL *pcontrol;                        // outcome - pointer to a control
//            int cvalue;                                   // integer control value
//            float *pcsensorvalue;                         // pointer to a sensor to take control value from
//            void *pmetro;                                 // we might put a metro, or something else on it...
      this->finternal = this->config.rf_value;                                  // fill the internal float with the provided static test value
      // todo log this when enabled log levels"RULE: '" this->config.label "'"
      this->pcsensorvalue = NULL;               // done in init
      this->pmetro = NULL;                      // done in init
      // set rule evaluation sensor / value
      if (this->config.rf_sensor_idx > -1 ) {              // TODO reimplement sensor indexing : && this->config.rf_sensor_idx < iSensorCount) {
				//TODO - this has to be done outside of the class - in APDUINO
				// todo remove deprecated code
				//this->psensor = &(pAPDSensors[this->config.rf_sensor_idx]);
      	//this->pvalue = &(pAPDSensors[this->config.rf_sensor_idx].fvalue);
      	// todo log this when enabled log levels " IN SENSOR(id" (this->config.rf_sensor_idx) (pAPDSensors[this->config.rf_sensor_idx].config.label);
      } else {
        this->psensor = NULL;
        // todo remove deprecated code
				// this->pvalue = &(this->finternal);      // use the provided static value
				// SerPrintP(" evals Static Value=");Serial.print(this->finternal); SerPrintP(" ");
        // todo log this when enabled log levels " IS VIRTUAL"
      }

      // test value is always the internal number, for now
      this->pvalue = &(this->finternal);      // use the provided static value

      // set rule execution control / value
      // TODO - reimplement the following check somehow (ArrayWrapper?)
      if (this->config.rule_control_idx > -1 ) {//&& this->config.rule_control_idx < pAPD->iControlCount) {
        // TODO THIS HAS TO BE SET ON APDUINO LEVEL
        //this->pcontrol = &(pAPDControls[this->config.rule_control_idx]);

          //TODO REIMPLEMENT CHECK
        if (this->config.ra_sensor_idx >= 0) {// && this->config.ra_sensor_idx < pAPD->iSensorCount) {
            // THIS HAS TO BE SET ON THE APDUINO LEVEL
          //this->pcsensorvalue = &(pAPDSensors[this->config.ra_sensor_idx].fvalue);
        	// todo log this when enabled log levels " SENSORVAL "); pAPDSensors[this->config.ra_sensor_idx].config.label, this->config.ra_sensor_idx
        } else {
          // TODO check the config.value_mapping
        	// todo log this when enabled log levels " STATIC VALUE "
          this->cvalue = this->config.ra_value;      // use the provided static value

          //TODO reimplement outside
          //this->cvalue = (pAPDControls[this->config.rule_control_idx].iValue);
        }
        // todo log this when enabled log levels " CONTROL " (this->config.rule_control_idx); SerPrintP(" -\" name can't be known yet. postproc needed..."); // DO NOT access pcontrol, ptr not set!
      } else {
      	APDDebugLog::log(APDUINO_WARN_RULEINVALIDCONTROL,NULL);
      	// todo log this when enabled log levels " NULL/INVALID CONTROL."
        this->pcontrol = NULL;
        // todo revise this
        //this->cvalue = &(this->finternal);      // use the provided static value
        this->cvalue = 0;      // use the provided static value
        //this->cvalue = this->config.ra_value;      // use the provided static value
      }
      if (this->config.pszconditions[0]!=0) {
      	// todo log this when enabled log levels " EVALS: \"" (this->config.pszconditions) ("\" --")
      }

      // select control functions
      // todo log this when enabled log levels (" T: ");
      this->ptcontrolfunc= get_rule_action_ptr(this->config.rule_true_action);
      // todo log this when enabled log levels (" F: ");
      this->pfcontrolfunc= get_rule_action_ptr(this->config.rule_false_action);

      // choose the evaluation function
      // todo log this when enabled log levels (" depending on -")
      /* link in the rule evaluation funtion */
      this->prulefunc = NULL;    // initialize as nullptr
      switch (this->config.rule_definition) {
        case RF_FALSE:
        	// todo log this when enabled log levels ("FALSE Rule")
          this->prulefunc = (&apd_rule_false);
          break;
        case RF_TRUE:
        	// todo log this when enabled log levels ("TRUE Rule")
          this->prulefunc = (&apd_rule_true);
          break;
        case RF_METRO:
        	// todo log this when enabled log levels ("METRO Rule")
          this->prulefunc = (&apd_rule_metro);
          if (this->pmetro = new Metro(this->config.rf_value)) {      // the correct metro value is the sensor's test val
          	// todo log this when enabled log levels ("Rule Metro allocated")
          } else {
          	APDDebugLog::log(APDUINO_ERROR_RMETROALLOCFAIL,NULL);
          }
          break;
        case RF_SCHEDULED:
        	// todo log this when enabled log levels ("RTC Rule")
          this->prulefunc = (&apd_rule_scheduled);
          break;
        case RF_IDLE_CHECK:
        	// todo log this when enabled log levels ("IDLE Rule")
          this->prulefunc = (&apd_rule_idle_check);
          break;
        case RF_RAM_CHECK:
        	// todo log this when enabled log levels ("RAM Rule")
          this->prulefunc = (&apd_rule_ram_check);
          break;
        case RF_SENSOR_GT:
        	// todo log this when enabled log levels ("SENSOR GT VAL Rule")
			this->prulefunc = (&apd_rule_sensor_gt);
			break;
        case RF_SENSOR_GTE:
        	break;
        case RF_SENSOR_LT:
        	// todo log this when enabled log levels ("SENSOR LT VAL Rule")
            this->prulefunc = (&apd_rule_sensor_lt);
            break;
        case RF_SENSOR_LTE:
        	// todo log this when enabled log levels ("NOT SUPPORTED")
          break;
        case RF_SENSOR_EQ:
        	// todo log this when enabled log levels ("SENSOR EQ VAL Rule")
            this->prulefunc = (&apd_rule_sensor_equ);
            break;
        case RF_SENSOR_GT_SENSOR:
        case RF_SENSOR_GTE_SENSOR:
        case RF_SENSOR_LT_SENSOR:
        case RF_SENSOR_LTE_SENSOR:
        case RF_SENSOR_EQ_SENSOR:
        	// todo log this when enabled log levels("NOT SUPPORTED")
          break;
        case RF_EVALUATE:
        	// todo log this when enabled log levels ("EVALUATE CONDITIONS")
					this->prulefunc = (&apd_rule_eval_conditions);
					break;
        default:
        	APDDebugLog::log(APDUINO_ERROR_RDEFINVALID,NULL);
        	// todo log this when enabled log levels ("Invalid rule definition.") (this->config.rule_definition)
			}
}


APDRule::~APDRule()
{
	if (this->pmetro) delete(this->pmetro);		// delete metro if any
	free(this->config.pszcron);									// free up any dynamically allocated cron string
	free(this->config.pszconditions);					  // free up any dynamically allocated conditions string
	initBlank();
}

void APDRule::initBlank() {
  memset(&config,0,sizeof(RDCONF));		// this NULLs the cron and conditions string ptrs too
  prulefunc = NULL;
  ptcontrolfunc = NULL;
  pfcontrolfunc = NULL;
  psensor = NULL;
  pvalue = NULL;
  finternal = 0;
  pcontrol = NULL;
  cvalue = 0;
  pcsensorvalue = NULL;
  pmetro = NULL;
  bLastState = false;
  psa = NULL;
  pca = NULL;
}


// evaluate the rule and call the corresponding true or false actions
void APDRule::evaluateRule() {
	// todo log this when enabled log levels ("EVAL RULE ") (this->config.label);
    if (this->pcsensorvalue && *(this->pcsensorvalue) == NAN) {
    	return;			// bail out if rule sensor is NAN
    }
    // todo expression should be pre-scanned or evaluator should be rewritten to do nothing on NAN's

    boolean bRuleOn = (*(this->prulefunc))(this);
    if ((bRuleOn != this->bLastState ||
    		(bRuleOn && (this->config.reexec & REEXEC_TRUE)) ||
    		(!bRuleOn && (this->config.reexec & REEXEC_FALSE))
    		) || (bRuleOn && this->config.rule_definition == RF_SCHEDULED )) {
    	this->bLastState = bRuleOn;
		  int iControlValue = 0;
		  if (bRuleOn) {                           // if the rule evaluated as TRUE
		  	// todo log this when enabled log levels (" EVALUATES TRUE -> ")
			if (this->pcsensorvalue != NULL) {    // if a sensor is specified for control value
			  //iControlValue = *((float*)(this->pcsensorvalue));      // take the sensor value
				iControlValue = map(*((float*)(this->pcsensorvalue)), 0, 1024, 0, 255);
				// todo log this when enabled log levels (" sensorval ")
			} else {                                                    // else
			  iControlValue = this->cvalue;                        // the value provided as control value
			  // todo log this when enabled log levels (" static ")
			}
			// todo log this when enabled log levels (iControlValue) " will be passed "

			(*(this->ptcontrolfunc))(this->pcontrol, iControlValue);      // call the TRUE control function with the selected control value
		  } else {                                 // else; do as described above, except the control function called is the FALSE control function
		  	// todo log this when enabled log levels " EVALUATES FALSE ->"
			if (this->pcsensorvalue != NULL) {                    // ...as above...
				// todo log this when enabled log levels " sensorval "
			  iControlValue = map(*((float*)(this->pcsensorvalue)), 0, 1024, 0, 255);
			} else {
				// todo log this when enabled log levels " static "
			  iControlValue = this->cvalue;
			}
			// todo log this when enabled log levels (iControlValue) (" will be passed ")
			(*(this->pfcontrolfunc))(this->pcontrol, iControlValue);      // call the FALSE control function
		  }

	//    }  // if virtual sensor rule
		// other rules should be evaluated on value change detected
  } // else no state change
}


boolean APDRule::bState() {
	return this->bState();
}

char *APDRule::getValueS(char *strdest) {
  char *retstr = NULL;
  sprintf(strdest,"%d",(int)this->bLastState);
  retstr=strdest;
  return retstr;
}



// get the desired action's function ptr
void (*APDRule::get_rule_action_ptr(int rule_action))(APDControl *,int) {
  //void *pfunc = NULL;
  void (*pfunc)(APDControl *,int) = NULL;

  switch (rule_action) {
    case APDRA_SET_OFF:
    	// todo log this when enabled log levels "Off Control"
      pfunc= (&APDControl::apd_action_set_off);
      break;
    case APDRA_SET_ON:
    	// todo log this when enabled log levels("On Control")
      pfunc= (&APDControl::apd_action_set_on);
      break;
    case APDRA_SWITCH_VALUE:
    	// todo log this when enabled log levels ("Switch Control")
      pfunc= (&APDControl::apd_action_switch);
      break;
    case APDRA_SET_VALUE:
    	// todo log this when enabled log levels "Set Value Control"
      pfunc= (&APDControl::apd_action_set_value);
      break;
    case APDRA_RCSWITCH_ON:
    	// todo log this when enabled log levels ("RC-Switch On")
          pfunc= (&APDControl::apd_action_rc_switch_on);
          break;
    case APDRA_RCSWITCH_OFF:
    	// todo log this when enabled log levels("RC-Switch Off")
          pfunc= (&APDControl::apd_action_rc_switch_off);
          break;
    case APDRA_RCPLUG_SET_VALUE:
    	// todo log this when enabled log levels ("RC-Plug Set Value")
          pfunc= (&APDControl::apd_action_rc_plug_set_value);
          break;
    case APDRA_RCPLUG_ON:
    	// todo log this when enabled log levels ("RC-Plug On")
          pfunc= (&APDControl::apd_action_rc_plug_on);
          break;
    case APDRA_RCPLUG_OFF:
    	// todo log this when enabled log levels ("RC-Plug Off")
          pfunc= (&APDControl::apd_action_rc_plug_off);
          break;
    case APDRA_VIRT_CUST_FUNC:
    	// todo log this when enabled log levels ("Custom Function Control")
      pfunc= (&APDControl::apd_action_custom_function);
      break;
    case APDRA_VIRT_SCREEN_NEXT:
    	// todo log this when enabled log levels ("Next Screen Control")
      pfunc= (&APDControl::apd_action_next_screen);
      break;
    case APDRA_VIRT_SYNCNTP:
    	// todo log this when enabled log levels ("NTPSync Control")
      pfunc= (&APDControl::apd_action_sync_ntp);
      break;
    case APDRA_NOOP:
    	// todo log this when enabled log levels ("NOOP Control")
      pfunc= (&APDControl::apd_action_noop);
      break;
    default:
    	char sztemp[10] ="";
    	APDDebugLog::log(APDUINO_ERROR_RACTIONINVALID,itoa(rule_action,sztemp,10));
  }
  return pfunc;
}







boolean APDRule::apd_rule_idle_check(APDRule *pRule) {
	// todo log this when enabled log levels ("IDLE CHK")
  boolean bIdle = (*(pRule->pcsensorvalue) == 1);
  // todo log this when enabled log levels (bIdle, DEC)
  return bIdle;
}

boolean APDRule::apd_rule_metro(APDRule *pRule) {
	// todo log this when enabled log levels ("METRO CHK")
  boolean retcode = (((APDRule*)pRule)->pmetro != NULL ? (((APDRule*)pRule)->pmetro)->check() : false);
  if (retcode) {
  	// todo log this when enabled log levels ("METRO INTERVAL");
      pRule->pmetro->reset();          // reset metro if passed
  }
  return retcode;
}

boolean APDRule::cronposeval(int curval,const char*pcpos) {
	char cnow[3]="";
	sprintf_P(cnow,PSTR("%02d"),curval);
	// todo log this when enabled log levels (">:") (cnow);
	int di = 0;			// used for extracting dividers
	// check if minute is *, equal to value or list containing value
	return (!strcmp_P(pcpos,PSTR("*")) || !strcmp(pcpos,cnow) ||
			(strstr(pcpos,cnow) && !strchr(pcpos,'/')) ||
			(strchr(pcpos,'/') && ((di=atoi(strchr(pcpos,'/')+1)) && (curval-(di*(curval/di)))==0 ) ) ) ;
}

// processes a cron-like time specification and returns true if job has to run
boolean APDRule::apd_rule_scheduled(APDRule *pRule) {
  boolean bret = false;						// return true if job should run, false otherwise
  if (pRule->config.pszcron) {		// if a cron timing is specified
  	// copy the cron definition to a temporary buffer for in-place splitting to mins/hrs/days/months/weekdays substrings
  	int ilen = strlen(pRule->config.pszcron);
  	char *psztemp = (char *)malloc(sizeof(char)*(ilen+1));
  	if (psztemp) {
			strcpy(psztemp,pRule->config.pszcron);
			// alloc pointers for substrings (will be set to point to locations "within" psztemp pointed string)
			char *pmins=0,*phours=0,*pdays=0,*pmonths=0,*pweekdays=0;
			// set up an array containing pointers to the chararray pointers
			char **ppcronstrs[5] = { &pmins, &phours, &pdays, &pmonths, &pweekdays };	// put the pointers to an array
			int i=0;	// will point to a position in psztemp
			// split string: replace '_' separators by \0's to produce smaller strings, store them to the pointers in the positions array
			for (int j=0; j < 5 && i < ilen; j++) {	// iterate the positions
				*(ppcronstrs[j]) = &psztemp[i];					// store the starting pointer to mins, hours, days, months, weekdays as we iterate through the string
				while (psztemp[i] != '_' && i < ilen) { i++; }		// skip to the next underscore
				if (psztemp[i] == '_') { psztemp[i] = 0; i++; }	// terminate substring (staring address just saved to a position)
			}
			// now we should have substrings
			// todo log this when enabled log levels ("CRON STRING: "); Serial.print(pmins); Serial.print(","); Serial.print(phours); Serial.print(","); Serial.print(pdays); Serial.print(",");  Serial.print(pmonths); Serial.print(","); Serial.println(pweekdays);
			DateTime now = APDTime::now();
			// check if minute is *, equal to value or list containing value
			if (cronposeval(now.minute(),pmins) ) {		// should handle "*", "10", "10,20" type inputs on minute
				// if minute was matching
				// todo log this when enabled log levels (".min.")
				if (cronposeval(now.hour(),phours)) {
						// if hour was matching
					// todo log this when enabled log levels (".hour.")
						if (cronposeval(now.day(),pdays)) {
							// if day was matching
							// todo log this when enabled log levels (".day.")
							if (cronposeval(now.month(),pmonths)) {
								// if month was matching
								// todo log this when enabled log levels (".month.")
								if (cronposeval(now.dayOfWeek(),pweekdays)) {		// should be also 2-digits! (or change cronposeval)
									APDDebugLog::log(APDUINO_MSG_CRONRUN,NULL);
									bret = true;
									// todo store cron evaluation timestamp to avoid reeval?
								}  // weekday
							}  // month
						}  // day
				}  // hour
			}	// minute

			free(psztemp);	// release temp str buf
  	} else {
  		APDDebugLog::log(APDUINO_ERROR_CRONOUTOFRAM,NULL);
  	}
  } else {
  	APDDebugLog::log(APDUINO_ERROR_NOCRONSPEC,NULL);
  }
  return bret;
}

//boolean apd_rule_rtc_metro(void *pRule) {
//  Metro* pMyMetro = (Metro *)(((APD_RULE*)pRule)->pmetro);
//  return (pMyMetro!=NULL ? pMyMetro->check() : false);
//}

boolean APDRule::apd_rule_true(APDRule *pRule) {
	// todo log this when enabled log levels ("TRUE CHK")
  return true;
}


boolean APDRule::apd_rule_false(APDRule *pRule) {
	// todo log this when enabled log levels ("FALSE CHK")
  return false;
}

boolean APDRule::apd_rule_ram_check(APDRule *pRule) {
	// todo log this when enabled log levels ("RAM CHK")
  return (((APDRule*)pRule)->cvalue > freeMemory());
}


boolean APDRule::apd_rule_sensor_equ(APDRule *pRule) {
	// todo log this when enabled log levels "SENSOR EQ CHK"
  APDRule *pr = (APDRule*)pRule;
  APDSensor *ps = pr->psensor;

  // simplified rule checking : take fvalue from the sensor
  // todo revise type conversions here!
  return ((int)ps->fvalue == (int)*((float *)(pr->pvalue)));

  // todo remove type specific checks deprecated code
/*  switch(ps->config.sensor_type) {
    case ANALOG_SENSOR:
    case DIGITAL_SENSOR:
    // todo log this when enabled log levels SerPrintP("ADCK: "); Serial.print(ps->config.label); SerPrintP(" : "); Serial.print(ps->fvalue,DEC); SerPrintP("(S) ==(?) "); Serial.print((int)*(float *)(pr->pvalue),DEC); SerPrintP("(R) ? ");
//      return ((ANADEF *)(ps->sensor))->value == *(int *)(pr->pvalue);
      return ((int)ps->fvalue == (int)*((float *)(pr->pvalue)));
      break;
//      return (ANADEF*)pr->psensor->sensor->value;
//      break;
    case DHT_SENSOR:
      return ((DHTSENS *)ps->sensor)->value == *(float *)(pr->pvalue);
      break;
    case ONEWIRE_SENSOR:
      return ((OWSENS *)ps->sensor)->value == *(float *)(pr->pvalue);
      break;
    case I2C_SENSOR:
      //return ((I2CDEF *)ps->sensor)->value == *(float *)(pr->pvalue);
      break;
    case VIRTUAL_SENSOR:
      break;
    default:
      break;
  }*/
  //return false;
}

// sensor baéir less than
boolean APDRule::apd_rule_sensor_lt(APDRule *pRule) {
	// todo log this when enabled log levels ("SENSOR LT CHK");
  APDRule *pr = (APDRule*)pRule;
  APDSensor *ps = pr->psensor;

  // simplified rule checking : take fvalue from the sensor
  return ((int)ps->fvalue < (int)*((float *)(pr->pvalue)));
}

// sensor value greater than
boolean APDRule::apd_rule_sensor_gt(APDRule *pRule) {
	// todo log this when enabled log levels ("SENSOR GT CHK")
  APDRule *pr = (APDRule*)pRule;
  APDSensor *ps = pr->psensor;

  // simplified rule checking : take fvalue from the sensor
  return ((int)ps->fvalue > (int)*((float *)(pr->pvalue)));
}

// evaluate the expression provided
boolean APDRule::apd_rule_eval_conditions(APDRule *pRule){
	// todo log this when enabled log levels ("EVALUATE CONDITIONS")
  APDRule *pr = (APDRule*)pRule;
  APDEvaluator *ape = new APDEvaluator(pr->psa, pr->pca);
  float fres = ape->feval(pr->config.pszconditions);
  free(ape);
  // todo log this when enabled log levels ("ram free:");  Serial.print(freeMemory());
  return (fres != 0);
}

