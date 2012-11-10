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
 * APDControl.cpp
 *
 *  Created on: Mar 26, 2012
 *      Author: George Schreiber
 */

#include "APDControl.h"

APDControl::APDControl() {
  initBlank();
}

/** construct an APDControl class instance
 *
 * \param[in] cdc pointer to the Control Definition Configuration
 * \param[in] preusablecontrol pointer to an APDControl that has a reusable control to be used by this new control (eg. RC-Plugs sharing an underlying RC-Switch object)
 *
 */
APDControl::APDControl(CDCONF *cdc, APDControl *preusablecontrol) {
  initBlank();
  memcpy((void*)&(this->config),(void*)cdc,sizeof(CDCONF));       // copy the structure to config

  // allocate ram for the control object/struct, as needed - depending on the type
  switch (this->config.control_type) {
    case ANALOG_CONTROL:
    	// todo DEBUG when levels enabled SerPrintP("ANALOG CONTROL");
      apd_action_set_value(this,this->config.initial_value);
      break;
    case DIGITAL_CONTROL:
    	// todo DEBUG when levels enabled  SerPrintP("DIGITAL CONTROL");
      apd_action_set_value(this,this->config.initial_value);
      break;
    case RCSWITCH_CONTROL:
    	// todo DEBUG when levels enabled  	SerPrintP("RCSWITCH CONTROL");
      {
				this->psharedclass = (void *)(new RCSwitch());
				RCSwitch *pswitch = (RCSwitch *)this->psharedclass;
				pswitch->enableTransmit(this->config.control_pin);
				// todo DEBUG when levels enabled SerPrintP("Enabled transmission.\n");
				//TODO enable the optional parameters via the extra config
			// Optional set pulse length.
			// pswitch->setPulseLength(320);

			// Optional set protocol (default is 1, will work for most outlets)
			// pswitch->setProtocol(2);

			// Optional set number of transmission repetitions.
			// pswitch->setRepeatTransmit(15);
			}
      break;
    case RCPLUG_CONTROL:
    	// todo DEBUG when levels enabled SerPrintP("RCPLUG CONTROL");
      {
    		if (preusablecontrol != NULL) {	// reuse existing RCSwitch
    			this->primary = false;
    			this->psharedclass = preusablecontrol->psharedclass;
    		} else {
    			this->primary = true;
    			this->psharedclass = (void *)(new RCSwitch());
					RCSwitch *pswitch = (RCSwitch *)this->psharedclass;
					pswitch->enableTransmit(this->config.control_pin);
					// todo DEBUG when levels enabled 	SerPrintP("Enabled transmission.\n");

					//TODO enable the optional parameters via the extra config
				// Optional set pulse length.
				// pswitch->setPulseLength(320);

				// Optional set protocol (default is 1, will work for most outlets)
				// pswitch->setProtocol(2);

				// Optional set number of transmission repetitions.
				// pswitch->setRepeatTransmit(15);
    		}
			}
      break;
    case SOFTWARE_CONTROL:
    	// todo DEBUG when levels enabled  SerPrintP("SOFTWARE CONTROL");
      //TODO implement
      //apd_action_set_value(this,this->config.initial_value);
      break;
    default:
    	APDDebugLog::log(APDUINO_WARN_CTYPEINVALID,NULL);
  }
  // we should have now a pointer to the APDSensor
}

APDControl::~APDControl() {
	// TODO Auto-generated destructor stub
	if (this->psharedclass != NULL) {
	  switch (this->config.control_type) {
		case RCSWITCH_CONTROL:
		case RCPLUG_CONTROL:
			if (this->primary){			// if primary owner of shared class
				RCSwitch *pswitch = (RCSwitch*)this->psharedclass;
				pswitch->disableTransmit();
			  delete(((RCSwitch*)this->psharedclass));
			} else {
				this->psharedclass = NULL;
			}
			break;
		default:
			;
			// todo DEBUG when levels enabled ("Who allocated pextra??!");
		}
	}
}

/** init class instance
 */
void APDControl::initBlank() {
  memset(&config,0,sizeof(CDCONF));
  control = NULL;
  iValue = 0;
  pmetro = NULL;
  pcustfunc = NULL;
  psharedclass = NULL;
}


/** get control value as string (char *)
 *
 * \param[out] strdest pointer to a char array to receive the value
 * \return strdest
 *
 */
char *APDControl::get_value_str(char *strdest) {
  char *retstr = NULL;
  //sprintf(strdest,"%3.1f",this->iValue);
  itoa(this->iValue, strdest, 10);
  retstr=strdest;
  return retstr;
}


/** action that calls a special callback that will cause selecting layout on display, for non-headless implementations
 *
 * \param[in] pAPDControl pointer
 * \param[in] iLayout layout index
 *
 */
void APDControl::apd_action_request_layout(APDControl *pAPDControl, int iLayout) {
  // TODO reimplement to support DISPLAY
}

void APDControl::apd_action_next_screen(APDControl *pAPDControl, int iReserved) {
  //TODO reimplement to support DISPLAY
}

void APDControl::apd_action_sync_ntp(APDControl *pAPDControl, int iReserved) {
  // TODO reimplement
}

void APDControl::apd_action_noop(APDControl *pAPDControl, int iReserved) {
	// no operation
}


void APDControl::apd_type_spec_control_write(APDControl *pAPDControl, int iNVal) {
  int iPin = pAPDControl->config.control_pin;
  // todo debug when levels enabled SerPrintP("WR CONTROL PIN "); Serial.print(iPin);, control type and so
  int iNewValue = -1;
  switch (pAPDControl->config.control_type) {
    case ANALOG_CONTROL:
      iNewValue = (iNVal<0 ? 0 : (iNVal>255 ? 255 : iNVal));
      if (iNewValue != pAPDControl->iValue) {
        analogWrite(iPin, iNewValue );
        pAPDControl->iValue = iNewValue;
      } else {
      	// not updating if value does not differ
      }
      break;
    case DIGITAL_CONTROL:
      iNewValue = iNVal > 0 ? HIGH : LOW;
      if (iNewValue != pAPDControl->iValue) {
        pinMode(iPin,OUTPUT);
        digitalWrite(iPin, iNewValue);
        pAPDControl->iValue = iNewValue;
      } else  {
      	// - " -
      }
      break;
    case RCPLUG_CONTROL:		// relay this op. to rcplugs
      apd_action_rc_plug_set_value(pAPDControl, iNVal);
      break;
      //todo add SERVO_CONTROL
      //todo add PULSE_CONTROL
  }
  // todo debug "done" when debug levels enabled
}


void APDControl::apd_action_set_on(APDControl *pAPDControl, int iValue) {
	// todo DEBUG when levels enabled SerPrintP("EXEC SET ON");
  apd_type_spec_control_write(pAPDControl, 255);
}

void APDControl::apd_action_set_off(APDControl *pAPDControl, int iValue) {
 // todo DEBUG when levels SerPrintP("EXEC SET OFF");
  apd_type_spec_control_write(pAPDControl, 0);
}

void APDControl::apd_action_switch(APDControl *pAPDControl, int iValue) {
	// todo DEBUG when levels enabled SerPrintP("EXEC SET SWITCH");
  apd_type_spec_control_write(pAPDControl, ((int)!(((APDControl*)pAPDControl)->iValue) * 255));
}

void APDControl::apd_action_set_value(APDControl *pAPDControl, int iValue) {
	// todo log when levels enabled  SerPrintP("EXEC SET VAL");
  apd_type_spec_control_write(pAPDControl, iValue);
}

/** action that calls a custom function
 * custom function must be added to the control prior to calling this function
 *
 * \param[in] pAPDControl pointer to the control object with the custom fuction
 * \param[in] iReserved reserved
 *
 */
void APDControl::apd_action_custom_function(APDControl *pAPDControl, int iReserved) {
	// todo DEBUG when levels enabled  SerPrintP("CALLING CUSTOM FUNCTION\n");
  if (pAPDControl != NULL && pAPDControl->pcustfunc != NULL && pAPDControl->config.control_type == SOFTWARE_CONTROL) {
  	// todo DEBUG when levels enabled   SerPrintP("Calling custfunc @ "); Serial.print((unsigned int)(pAPDControl->pcustfunc),DEC);SerPrintP(".\n");  delay(300);
      (*(pAPDControl->pcustfunc))();            // call the custom function
  }
  // todo DEBUG when levels enabled  SerPrintP("CUSTFUNC DONE.");
}

/** action that causes rc-switch control to send the on signal on a given channel
 *
 * \param[in] pAPDControl pointer to the RC-Plug control object
 * \param[in] iControl group/channel to which the on signal should be sent to
 *
 */
void APDControl::apd_action_rc_switch_on(APDControl *pAPDControl, int iControl) {
	if (pAPDControl->config.control_type == RCSWITCH_CONTROL) {
	  RCSwitch *pswitch = (RCSwitch *)pAPDControl->psharedclass;
	  if (pswitch != NULL) {
	  	// todo DEBUG when levels enabled  SerPrintP("Switch On :");
			uint8_t uac = highByte(iControl);
			uint8_t ucc = lowByte(iControl);
			pswitch->switchOn(uac,ucc);
			// todo DEBUG when levels enabled Serial.print((int)uac, DEC); SerPrintP("-"); Serial.print((int)ucc,DEC); SerPrintP("\n");
	  } else {
	  	// todo DEBUG when levels enabled  SerPrintP("Missing RC-Switch object!\n");
	  }
	} else {
		// todo DEBUG when levels enabled SerPrintP("Wrong control type.\n");
	}
}

/** action that causes rc-switch control to send the off signal on a given channel
 *
 * \param[in] pAPDControl pointer to the RC-Plug control object
 * \param[in] iControl group/channel to which the off signal should be sent to
 *
 */
void APDControl::apd_action_rc_switch_off(APDControl *pAPDControl, int iControl) {
	if (pAPDControl->config.control_type == RCSWITCH_CONTROL) {
	  RCSwitch *pswitch = (RCSwitch *)pAPDControl->psharedclass;
	  if (pswitch != NULL) {
	  	// todo DEBUG when levels enabled SerPrintP("Switch Off: ");
			uint8_t uac = highByte(iControl);
			uint8_t ucc = lowByte(iControl);

			pswitch->switchOff(uac,ucc);
			// todo DEBUG when levels enabled Serial.print((int)uac, DEC); SerPrintP("-"); Serial.print((int)ucc,DEC); SerPrintP("\n");
		} else {
			// todo DEBUG when levels enabled SerPrintP("Missing RC-Switch object!\n");
	  }
	} else {
		// todo DEBUG when levels enabled SerPrintP("Wrong control type.\n");
	}
}

/** action that turns an rc-plug on or off
 *
 * \param[in] pAPDControl pointer to the RC-Plug control object
 * \param[in] iValue value to be set
 *
 */
void APDControl::apd_action_rc_plug_set_value(APDControl *pAPDControl, int iValue) {
	if (pAPDControl->config.control_type == RCPLUG_CONTROL) {
	  RCSwitch *pswitch = (RCSwitch *)pAPDControl->psharedclass;
	  int iControl = 0;
	  int iscand = sscanf(pAPDControl->config.extra_data,"%d",&iControl);
	  if (iscand) {
			uint8_t uac = highByte(iControl);	// extract rc-group id
			uint8_t ucc = lowByte(iControl);	// extract rc-control id

			if (pswitch != NULL) {

				if (iValue == 0) {
					// todo VERBOSE with level SerPrintP("Switch Off :");
					pswitch->switchOff(uac,ucc);
				} else {
					// todo VERBOSE with level SerPrintP("Switch On :");
					pswitch->switchOn(uac,ucc);
				}
				// todo verbose with level Serial.println(pAPDControl->iValue);
				pAPDControl->iValue = (int)(iValue != 0);
				// todo VERBOSE	Serial.print((int)uac, DEC); SerPrintP("-"); Serial.print((int)ucc,DEC); SerPrintP("\n");
			} else {
				// todo VERBOSE	SerPrintP("Missing RC-Switch object!\n");
			}
	  } else {
			// todo VERBOSE	SerPrintP("Missing RC-Plug data!\n")
	  }
	} else {
		// todo VERBOSE SerPrintP("Wrong control type.\n");
	}
}

/** action that turns an rc-plug on
 *
 * \param[in] pAPDControl pointer to the RC-Plug control object
 * \param[in] iReserved reserved
 *
 */
void APDControl::apd_action_rc_plug_on(APDControl *pAPDControl, int iReserved) {
	apd_action_rc_plug_set_value(pAPDControl, 1);
}

/** action that turns an rc-plug off
 *
 * \param[in] pAPDControl pointer to the RC-Plug control object
 * \param[in] iReserved reserved
 *
 */
void APDControl::apd_action_rc_plug_off(APDControl *pAPDControl, int iReserved) {
	apd_action_rc_plug_set_value(pAPDControl, 0);
}



