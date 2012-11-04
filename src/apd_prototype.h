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
 * apd_prototype.h
 *
 * a currently unmaintained file with a bunch of default values that has been in use
 * before things like autoprovisioning have been implemented.
 *
 * todo revise this file, the need for it and deprecate if not needed
 *
 *  Created on: Mar 27, 2012
 *      Author: George Schreiber
 */

#ifndef APD_PROTOTYPE_H_
#define APD_PROTOTYPE_H_

#include "APDSensor.h"
#include "APDRule.h"
#include "APDControl.h"

SDCONF APDSENSORS[] = {
        {"TestA1", ANALOG_SENSOR, 0, 0, 5, 1000, 0 },
        {"TestD1", DIGITAL_SENSOR, 0, 0, 5, 5000, 0 }
      };

CDCONF APDCONTROLS[] = {
        {"HBLed1", DIGITAL_CONTROL, 44, 0},                 // led on pin 44
        {"HBLed2", DIGITAL_CONTROL, 45, 1},                 // led on pin 45
        {"LCDBacklight", DIGITAL_CONTROL, 32, 0},           // lcd backlight on pin 32
        {"ButtonALed", DIGITAL_CONTROL, 46, 0},             // a led on pin 46
        {"ButtonBLed", DIGITAL_CONTROL, 47, 0},             // b led on pin 47
        {"PhotoLed", ANALOG_CONTROL, 11, 0}                 // c led on pin 11
      };

RDCONF APDRULES[] = {
  {"HB Blink1", RF_METRO, -1, 1000.0, APDRA_SWITCH_VALUE, APDRA_SWITCH_VALUE, 0, 0, 0, -1 },     // blink the first led that was initialized true
  {"HB Blink2", RF_METRO, -1, 1000.0, APDRA_SWITCH_VALUE, APDRA_SWITCH_VALUE, 1, 0, 1, -1 },        // blink the second led that was initialized false
  {"LCD", RF_IDLE_CHECK, -1, 250.0, APDRA_SET_OFF, APDRA_SET_ON, 2, 0, 1, -1 },            // turn on/off backlight
  {"ButtALed", RF_SENSOR_EQ, 9, HIGH, APDRA_SET_ON, APDRA_SET_OFF, 3, 0, 0, -1 },            // blink led A
  {"ButtAScr", RF_SENSOR_EQ, 9, HIGH, APDRA_VIRT_SCREEN_NEXT, APDRA_NOOP, -1, 0, 0, -1 },    // switch to next screen
  {"ButtBLed", RF_SENSOR_EQ, 10, HIGH, APDRA_SET_ON, APDRA_SET_OFF, 4, 0, 0, -1 }            // blink led B
  // test photoled -> value from sensor
};


// Sensor Screen Configuration ("record") - the following definition should come from file
struct SCRCONF {
  int iSensorIndex;
  char Unit[5];
  int iReserved;      // reserved for later (eg. postprocessing of sensor value)
};

// Sensor Screen Configuration    -- for now a hard-coded number of 6 positions used
// override in 'SENSCR.CFG' on SD card, or fall back to 1st 6 sensors, no units (unpredictable behaviour if <6 sensors!)
// replace this with a pointer to the array later on (when broken LCD replaced) and reimplement it similarly to sensor provisioning
SCRCONF APDSENSORSCREEN[6] = { {0,"",0}, {1,"",0}, {2,"",0}, {3,"",0}, {4,"",0}, {5,"",0} };


byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEF };            //TODO generate random mac, more than 1 APDuino can have collisions
byte ip[] = { 0, 0, 0, 0 };                          // uninitialized address
byte gateway[] = { 0, 0, 0, 0 };                     // uninitialized address
byte subnet[] = { 0, 0, 0, 0 };                      // uninitialized address
byte apduino_server_ip[] = {192,168,1,16};           // apduino.localhost -- test server on LAN
char apduino_server_name[33]="apduino.localhost";    // test APDuino server on LAN
unsigned int apduino_server_port = 80;               // standard HTTP port
unsigned int localPort = 8888;                       // local port to listen for UDP packets (NTP communications)
unsigned int wwwPort = 80;                           // local port to listen on with WWW server


#endif /* APD_PROTOTYPE_H_ */
