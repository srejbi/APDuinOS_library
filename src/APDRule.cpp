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

  memcpy((void*)&(this->config),(void*)rdc,sizeof(RDCONF));       // copy the structure to config

  this->psa = pSA;
  this->pca = pCA;

// NOW's the time to do some tests
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
      SerPrintP("RULE: '"); Serial.print( this->config.label); SerPrintP("'");
      this->pcsensorvalue = NULL;               // done in init
      this->pmetro = NULL;                      // done in init
      // set rule evaluation sensor / value
      if (this->config.rf_sensor_idx > -1 ) {              // TODO reimplement sensor indexing : && this->config.rf_sensor_idx < iSensorCount) {

          //TODO - this has to be done outside of the class - in APDUINO
          //this->psensor = &(pAPDSensors[this->config.rf_sensor_idx]);



//            this->pvalue = &(pAPDSensors[this->config.rf_sensor_idx].fvalue);
        SerPrintP(" IN SENSOR(id");Serial.print(this->config.rf_sensor_idx);
        SerPrintP(" - ");

        // TODO THIS PART IS CRUCIAL - outside
        //Serial.print(pAPDSensors[this->config.rf_sensor_idx].config.label);
        SerPrintP(") ");
      } else {
        this->psensor = NULL;
//            this->pvalue = &(this->finternal);      // use the provided static value
//            SerPrintP(" evals Static Value=");Serial.print(this->finternal); SerPrintP(" ");
        SerPrintP(" IS VIRTUAL");
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
          SerPrintP(" SENSORVAL "); //Serial.print(pAPDSensors[this->config.ra_sensor_idx].config.label);
          Serial.print(this->config.ra_sensor_idx,DEC);
        } else {
          // TODO check the config.value_mapping
          SerPrintP(" STATIC VALUE ");
          this->cvalue = this->config.ra_value;      // use the provided static value

          //TODO reimplement outside
          //this->cvalue = (pAPDControls[this->config.rule_control_idx].iValue);
        }
        SerPrintP(" CONTROL "); Serial.print(this->config.rule_control_idx); SerPrintP(" -\" name can't be known yet. postproc needed...");
        //; Serial.print(this->pcontrol->config.label);SerPrintP("\" ");                // TODO dont access pcontrol, ptr not set!
      } else {
        SerPrintP(" NULL/INVALID CONTROL.");
        this->pcontrol = NULL;
        // todo revise this
        //this->cvalue = &(this->finternal);      // use the provided static value
        this->cvalue = 0;      // use the provided static value
        //this->cvalue = this->config.ra_value;      // use the provided static value
      }
      if (this->config.conditions[0]!=0) {
      	SerPrintP(" EVALS: \"");Serial.print(this->config.conditions);SerPrintP("\" --");
      }

      // select control functions
      SerPrintP(" T: ");
      this->ptcontrolfunc= get_rule_action_ptr(this->config.rule_true_action);

      SerPrintP(" F: ");
      this->pfcontrolfunc= get_rule_action_ptr(this->config.rule_false_action);

      // choose the evaluation function

      SerPrintP(" depending on -");
      /* link in the rule evaluation funtion */
      this->prulefunc = NULL;    // initialize as nullptr
      switch (this->config.rule_definition) {
        case RF_FALSE:
          SerPrintP("FALSE Rule");
          this->prulefunc = (&apd_rule_false);
          break;
        case RF_TRUE:
          SerPrintP("TRUE Rule");
          this->prulefunc = (&apd_rule_true);
          break;
        case RF_METRO:
          SerPrintP("METRO Rule");
          this->prulefunc = (&apd_rule_metro);
          this->pmetro = new Metro(this->config.rf_value);      // the correct metro value is the sensor's test val
if  (this->pmetro)  {            SerPrintP("Rule Metro allocated"); }else {SerPrintP("Failed to allocate rule metro."); }
          break;
        case RF_RTC_PASSED:
          SerPrintP("RTC Rule");
          this->prulefunc = (&apd_rule_rtc_passed);
          break;
        case RF_IDLE_CHECK:
          SerPrintP("IDLE Rule");
          this->prulefunc = (&apd_rule_idle_check);
          break;
        case RF_RAM_CHECK:
          SerPrintP("RAM Rule");
          this->prulefunc = (&apd_rule_ram_check);
          break;
        case RF_SENSOR_GT:
			SerPrintP("SENSOR GT VAL Rule");
			this->prulefunc = (&apd_rule_sensor_gt);
			break;
        case RF_SENSOR_GTE:
        	break;
        case RF_SENSOR_LT:
            SerPrintP("SENSOR LT VAL Rule");
            this->prulefunc = (&apd_rule_sensor_lt);
            break;
        case RF_SENSOR_LTE:
          SerPrintP("NOT SUPPORTED");
          break;
        case RF_SENSOR_EQ:
            SerPrintP("SENSOR EQ VAL Rule");
            this->prulefunc = (&apd_rule_sensor_equ);
            break;
        case RF_SENSOR_GT_SENSOR:
        case RF_SENSOR_GTE_SENSOR:
        case RF_SENSOR_LT_SENSOR:
        case RF_SENSOR_LTE_SENSOR:
        case RF_SENSOR_EQ_SENSOR:
          SerPrintP("NOT SUPPORTED");
          break;
        case RF_EVALUATE:
					SerPrintP("EVALUATE CONDITIONS");
					this->prulefunc = (&apd_rule_eval_conditions);
					break;
        default:
          SerPrintP("Invalid rule definition."); Serial.print(this->config.rule_definition);
      }
}





// initialize with string containing RDCONF
APDRule::APDRule(char *psz_rdc) {
  // TODO implement this
  initBlank();
}

APDRule::~APDRule()
{
  // TODO Auto-generated destructor stub
	if (this->pmetro) delete(this->pmetro);		// delete metro if any
	initBlank();
}

void APDRule::initBlank() {
  memset(&config,0,sizeof(RDCONF));
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



void APDRule::evaluateRule() {
#ifdef DEBUG
    SerPrintP("EVAL RULE "); Serial.print(this->config.label); //delay(10);
#endif
    boolean bRuleOn = (*(this->prulefunc))(this);
    if (bRuleOn != this->bLastState ||
    		(bRuleOn && (this->config.reexec & REEXEC_TRUE)) ||
    		(!bRuleOn && (this->config.reexec & REEXEC_FALSE))
    		) {
    	this->bLastState = bRuleOn;
		  int iControlValue = 0;
		  if (bRuleOn) {                           // if the rule evaluated as TRUE
	#ifdef DEBUG
			SerPrintP(" EVALUATES TRUE -> ");
	#endif
			if (this->pcsensorvalue != NULL) {    // if a sensor is specified for control value
			  //iControlValue = *((float*)(this->pcsensorvalue));      // take the sensor value
				iControlValue = map(*((float*)(this->pcsensorvalue)), 0, 1024, 0, 255);
#ifdef DEBUG
			  SerPrintP(" sensorval ");
#endif
			} else {                                                    // else
			  iControlValue = this->cvalue;                        // the value provided as control value
#ifdef DEBUG
			  SerPrintP(" static ");
#endif
			}
#ifdef DEBUG
			Serial.print(iControlValue); SerPrintP(" will be passed ");
#endif

			(*(this->ptcontrolfunc))(this->pcontrol, iControlValue);      // call the TRUE control function with the selected control value
		  } else {                                 // else; do as described above, except the control function called is the FALSE control function
#ifdef DEBUG
			SerPrintP(" EVALUATES FALSE ->");
#endif
			if (this->pcsensorvalue != NULL) {                    // ...as above...
#ifdef DEBUG
			  SerPrintP(" sensorval ");
#endif
			  //iControlValue = *((float*)(this->pcsensorvalue));
			  iControlValue = map(*((float*)(this->pcsensorvalue)), 0, 1024, 0, 255);
			} else {
#ifdef DEBUG
			  SerPrintP(" static ");
#endif
			  iControlValue = this->cvalue;
			}
#ifdef DEBUG
			Serial.print(iControlValue); SerPrintP(" will be passed ");
#endif
			(*(this->pfcontrolfunc))(this->pcontrol, iControlValue);      // call the FALSE control function
		  }
#ifdef DEBUG
		  SerPrintP(".\n");
#endif
	//    }  // if virtual sensor rule
		// other rules should be evaluated on value change detected
  } // else no state change
}







// get the desired action's function ptr
void (*APDRule::get_rule_action_ptr(int rule_action))(APDControl *,int) {
  //void *pfunc = NULL;
  void (*pfunc)(APDControl *,int) = NULL;

  switch (rule_action) {
    case APDRA_SET_OFF:
      SerPrintP("Off Control");
      pfunc= (&APDControl::apd_action_set_off);
      break;
    case APDRA_SET_ON:
      SerPrintP("On Control");
      pfunc= (&APDControl::apd_action_set_on);
      break;
    case APDRA_SWITCH_VALUE:
//#ifdef DEBUG
    SerPrintP("Switch Control");
//#endif
      pfunc= (&APDControl::apd_action_switch);
      break;
    case APDRA_SET_VALUE:
//#ifdef DEBUG
    SerPrintP("Set Value Control");
//#endif
      pfunc= (&APDControl::apd_action_set_value);
      break;
    case APDRA_RCSWITCH_ON:
    //#ifdef DEBUG
        SerPrintP("RC-Switch On");
    //#endif
          pfunc= (&APDControl::apd_action_rc_switch_on);
          break;
    case APDRA_RCSWITCH_OFF:
    //#ifdef DEBUG
        SerPrintP("RC-Switch Off");
    //#endif
          pfunc= (&APDControl::apd_action_rc_switch_off);
          break;
    case APDRA_RCPLUG_SET_VALUE:
    //#ifdef DEBUG
        SerPrintP("RC-Plug Set Value");
    //#endif
          pfunc= (&APDControl::apd_action_rc_plug_set_value);
          break;
    case APDRA_RCPLUG_ON:
    //#ifdef DEBUG
        SerPrintP("RC-Plug On");
    //#endif
          pfunc= (&APDControl::apd_action_rc_plug_on);
          break;
    case APDRA_RCPLUG_OFF:
    //#ifdef DEBUG
        SerPrintP("RC-Plug Off");
    //#endif
          pfunc= (&APDControl::apd_action_rc_plug_off);
          break;
    case APDRA_VIRT_CUST_FUNC:
//#ifdef DEBUG
    SerPrintP("Custom Function Control");
//#endif
      pfunc= (&APDControl::apd_action_custom_function);
      break;
    case APDRA_VIRT_SCREEN_NEXT:
//#ifdef DEBUG
    SerPrintP("Next Screen Control");
//#endif
      pfunc= (&APDControl::apd_action_next_screen);
      break;
    case APDRA_VIRT_SYNCNTP:
//#ifdef DEBUG
    SerPrintP("NTPSync Control");
//#endif
      pfunc= (&APDControl::apd_action_sync_ntp);
      break;
    case APDRA_NOOP:
//#ifdef DEBUG
    SerPrintP("NOOP Control");
//#endif
      pfunc= (&APDControl::apd_action_noop);
      break;
    default:
      SerPrintP("Invalid Control definition."); Serial.print(rule_action);
  }
  return pfunc;
}













boolean APDRule::apd_rule_idle_check(APDRule *pRule) {
#ifdef DEBUG
  SerPrintP("IDLE CHK");
#endif
  boolean bIdle = (*(pRule->pcsensorvalue) == 1);
#ifdef DEBUG
  Serial.print(bIdle, DEC); SerPrintP("\n");
#endif
  return bIdle;
}

boolean APDRule::apd_rule_metro(APDRule *pRule) {
#ifdef DEBUG
  SerPrintP("METRO CHK");
#endif
  boolean retcode = (((APDRule*)pRule)->pmetro != NULL ? (((APDRule*)pRule)->pmetro)->check() : false);
  if (retcode) {
#ifdef DEBUG
      SerPrintP("METRO INTERVAL"); Serial.print(pRule->config.);
#endif
      pRule->pmetro->reset();          // reset metro if passed
  }
  return retcode;
}

boolean APDRule::apd_rule_rtc_passed(APDRule *pRule) {
  //TODO: implement checking the cvalue as time against rtc - use UNIX time
  return false;
}

//boolean apd_rule_rtc_metro(void *pRule) {
//  Metro* pMyMetro = (Metro *)(((APD_RULE*)pRule)->pmetro);
//  return (pMyMetro!=NULL ? pMyMetro->check() : false);
//}

boolean APDRule::apd_rule_true(APDRule *pRule) {
#ifdef DEBUG
  SerPrintP("TRUE CHK");
#endif
  return true;
}


boolean APDRule::apd_rule_false(APDRule *pRule) {
#ifdef DEBUG
  SerPrintP("FALSE CHK");
#endif
  return false;
}

boolean APDRule::apd_rule_ram_check(APDRule *pRule) {
#ifdef DEBUG
  SerPrintP("RAM CHK");
#endif
  return (((APDRule*)pRule)->cvalue > freeMemory());
}


boolean APDRule::apd_rule_sensor_equ(APDRule *pRule) {
#ifdef DEBUG
  SerPrintP("SENSOR EQ CHK");
#endif
  APDRule *pr = (APDRule*)pRule;
  APDSensor *ps = pr->psensor;

  // simplified rule checking : take fvalue from the sensor
  return ((int)ps->fvalue == (int)*((float *)(pr->pvalue)));

/*  switch(ps->config.sensor_type) {
    case ANALOG_SENSOR:
    case DIGITAL_SENSOR:
#ifdef DEBUG
      SerPrintP("ADCK: "); Serial.print(ps->config.label); SerPrintP(" : "); Serial.print(ps->fvalue,DEC); SerPrintP("(S) ==(?) "); Serial.print((int)*(float *)(pr->pvalue),DEC); SerPrintP("(R) ? ");
      delay(100);
#endif
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

boolean APDRule::apd_rule_sensor_lt(APDRule *pRule) {
#ifdef DEBUG
  SerPrintP("SENSOR LT CHK");
#endif
  APDRule *pr = (APDRule*)pRule;
  APDSensor *ps = pr->psensor;

  // simplified rule checking : take fvalue from the sensor
  return ((int)ps->fvalue < (int)*((float *)(pr->pvalue)));
}

boolean APDRule::apd_rule_sensor_gt(APDRule *pRule) {
#ifdef DEBUG
  SerPrintP("SENSOR GT CHK");
#endif
  APDRule *pr = (APDRule*)pRule;
  APDSensor *ps = pr->psensor;

  // simplified rule checking : take fvalue from the sensor
  return ((int)ps->fvalue > (int)*((float *)(pr->pvalue)));
}


boolean APDRule::apd_rule_eval_conditions(APDRule *pRule){
#ifdef DEBUG
  SerPrintP("EVALUATE CONDITIONS);
#endif
  APDRule *pr = (APDRule*)pRule;
  APDEvaluator *ape = new APDEvaluator(pr->psa, pr->pca);
  float fres = ape->feval(pr->config.conditions);
  free(ape);
#ifdef DEBUG
  SerPrintP("ram free:");  Serial.print(freeMemory()); SerPrintP("\n");
#endif
  // simplified rule checking : take fvalue from the sensor
  return (fres != 0);
}

