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
 * AtlasScientificSensor.cpp
 *
 * todo sensors need testing, code might not be fully working
 *
 * should work with HardwareSerial (pins 19,18 or 17,16 or 15,14)
 * and optional RS-232 DEMUXER attached
 * (set S0,S1, hard pull E to ground)
 *
 * calibration: use the HTTP API to send in the calibration command letters
 * according to the AtlasScientific documentation for the sensor.
 * Use debug log to see if calibration step was ok.
 * Use other Atlas commands to turn off leds when happy and to store in eprom.
 *
 *  Created on: Aug 27, 2012
 *      Author: George Schreiber
 */

#include "AtlasScientificSensor.h"
AtlasScientificSensor::AtlasScientificSensor(SDCONF *sdc, void *assensor)
{
	int edscand = 0;		// extra data parameters scanned
	this->initSensor(sdc);

	this->sensor = (ASSENS*)malloc(sizeof(ASSENS));		// allocate sensor data
	memset(this->sensor,0,sizeof(ASSENS));				// 0 the memory
	this->sensor->channel = NAN;							// channel defaults to NAN (unused)

	ASENC ast;											// temporary buffer
	int cht;												// channel

	// pre-scan (before building sensor) the extra parameters from the config

	// extra data should hold RS-232 port splitter configuration -> Software Serial
	// if present, it should be "<YN>,<S0>,<S1>,<E>", where YN is the split port no. 0-3, S0 and S1 are the pins to select channel and E is the enable pin
	edscand = sscanf(this->config.extra_data, "%d,%d,%d,%d",
			&cht, &(ast.S0), &(ast.S1), &(ast.E));

	// todo we might want to bail out now if 0 < edscand  < 3 because it means the config for SW serial is corrupted

	// start building up the sensor
	if (this->sensor != NULL) {
		if (assensor!=NULL) {			// if reusing another AtlasScientific on this port (should be SW Serial)
			if (((AtlasScientificSensor*)assensor)->is_demux()) {	  // reusable Atlas Sensor MUST be DEMUX
				this->bPrimary = false;
				this->sensor->asenc = ((AtlasScientificSensor*)assensor)->sensor->asenc;
				if (memcmp(&(ast.S0), &(this->sensor->asenc->S0), sizeof(byte)*3) == 0) {
					// TODO add logging, test this branch
					if (cht != ((AtlasScientificSensor*)assensor)->sensor->channel) {
						// TODO add logging check this branch
						this->sensor->channel = cht;	// OK, reusing a softserial, setting own channel
						// todo log this when enabled log levels
						SerPrintP(" channel "); Serial.print(this->sensor->channel);
					} else {
						// todo log this when enabled log levels ERROR same SW serial channel
					}
				} else {
					// todo log this when enabled log levels ERROR different SoftSerial config on same HW pins
				}
			} else {
				// todo log this when enabled log levels ERROR invalid reusable Atlas Sensor (not SW Serial)
			}
		} else {															// PRIMARY SENSOR OBJECT / RS-232
			this->bPrimary = true;
			this->sensor->asenc = (ASENC*)malloc(sizeof(ASENC));
			if (this->sensor->asenc != NULL) {
				memset(this->sensor->asenc,0,sizeof(ASENC));
				this->sensor->asenc->csel = NAN;							// no channel selected (in case DEMUX)
				// get ptr to HW serial or allocate a new SW serial object, according to extra config
				//if (edscand == 0) {	// hardware serial (0 SW serial parameters scanned)
				if (edscand >= 3) {	// we at least have YN,S0,S1 -- IT IS AN RS-232 DEMUXER ATTACHED TO PINS
					if (edscand > 3) {	// if E specified
						// ok
					} else {		// no E spec
						ast.E = 0;		// E should be hard pulled GND in your wiring
					}
					// todo log this when enabled log levels
					char sztmp[32]="";
					sprintf_P(sztmp,PSTR("DMX:%d,%d:%d,%d,%d/%d"),this->config.sensor_pin,this->config.sensor_secondary_pin,ast.S0,ast.S1,ast.E,cht);
					APDDebugLog::log(APDUINO_MSG_ATLAS_CO,sztmp);	// todo this should be DEBUG level

					this->sensor->asenc->S0 = ast.S0;
					this->sensor->asenc->S1 = ast.S1;
					this->sensor->asenc->E = ast.E;
					this->sensor->channel = cht;		// todo, remove, this should have been done already

					pinMode(this->sensor->asenc->S0, OUTPUT);							// S0 is OUTPUT
					pinMode(this->sensor->asenc->S1, OUTPUT);							// S1 is OUTPUT
					if (this->sensor->asenc->E > 0) {		// if E pin specified (not pulled to GND fixed)
						pinMode(this->sensor->asenc->E, OUTPUT);						// E is OUTPUT (LOW to enable, HIGH to disable)
						// now pulling LOW fixed if using SW serial, otherwise should be enabled/disabled before/after read/print operations
						digitalWrite(this->sensor->asenc->E, LOW);
						APDDebugLog::log(APDUINO_MSG_ATLAS_CO,"E");	// todo this should be DEBUG level
					}
					APDDebugLog::log(APDUINO_MSG_ATLAS_CO,NULL);	// todo this should be DEBUG level
				}	// END SETTING UP DEMUXER

				if (this->is_hw_serial()) {	// hardware serial (0 SW serial parameters scanned)
					// todo log this when enabled log levels
					APDDebugLog::log(APDUINO_MSG_ATLAS_HW,"HW");
					if (this->selectHWSerial()) {		// will detect which HW serial to open
						// start the hardware serial port
						((HardwareSerial*)(((ASSENS*)(this->sensor))->asenc->serialport))->begin(ATLAS_BAUD_RATE);
						APDDebugLog::log(APDUINO_MSG_ATLAS_HW,NULL);	// todo this should be DEBUG level
					}	else {
						// todo log this when enabled log levels
						APDDebugLog::log(APDUINO_ERROR_ATLAS_HW,NULL); 				// ERROR selecting HW serial
					}
					// todo halt?
				} else {			// Software Serial (3 or 4 SW serial params scanned)
#ifdef ATLAS_SOFTSERIAL
					if (this->selectSWSerial()) {

						((SoftwareSerial*)(((ASSENS*)(this->sensor))->asenc->serialport))->begin(ATLAS_BAUD_RATE);
						APDDebugLog::log(APDUINO_MSG_ATLAS_SW,NULL);	// todo this should be DEBUG level
					} else
#else
						// TODO log not supported
#endif
					{
						// todo log this when enabled log levels  ERROR selecting sw serial
						APDDebugLog::log(APDUINO_ERROR_ATLAS_SW,NULL);	// todo this should be DEBUG level
					}
				}
				// todo bail out on previous errors otherwise state will look like ready even if error(s) occurred
				this->sensor->asenc->state = STATE_READY;
			} else {
				// todo log this when enabled log levels
				APDDebugLog::log(APDUINO_MSG_ATLAS,"E0");			// ERROR - out of ram
			}
		}

		this->sensor->value = NAN;
		this->fvalue = NAN;

	}

	this->_lm = millis();
	this->_state = STATE_READY;
}

AtlasScientificSensor::~AtlasScientificSensor() {
	// TODO send stop command to sensor
	if (this->sensor != NULL) {
		if (this->sensor->asenc != NULL) {
			if (this->bPrimary) {
				if (this->sensor->asenc->serialport != NULL) {
					if (this->is_hw_serial()) {
						HardwareSerial *sp = (HardwareSerial *)(this->sensor->asenc->serialport);
						sp->end();
						// HW serial was NOT created just addressed, not freeing mem
					} else {
#ifdef ATLAS_SOFTSERIAL
						SoftwareSerial *sp = (SoftwareSerial *)(this->sensor->asenc->serialport);
						//sp->end();			// SoftwareSerial destructor will end
						delete(sp);
						// todo log this when enabled log levels
						APDDebugLog::log(APDUINO_MSG_ATLAS,"SWD");
#else
						// TODO log not supported
#endif
					}
					this->sensor->asenc->serialport = NULL;			// reset serialport to NULL
				}
				free(this->sensor->asenc);										// free Atlas Scientific Sensor encapsulation struct
				// todo log this when enabled log levels
				APDDebugLog::log(APDUINO_MSG_ATLAS,"OF");
			}
			this->sensor->asenc = NULL;										// reset encapsulation struct ptr to NULL
		}
		free(this->sensor);
		// todo log this when enabled log levels
		APDDebugLog::log(APDUINO_MSG_ATLAS,"SF");
		this->sensor = NULL;
	}
	delete(this->pmetro);
	// todo log this when enabled log levels
	APDDebugLog::log(APDUINO_MSG_ATLAS,"MD");
	this->pmetro = NULL;
}

// returns true if using hw serial pins (so hwserial should be used)
bool AtlasScientificSensor::is_hw_serial() {
	int prx = this->config.sensor_pin;
	int ptx = this->config.sensor_secondary_pin;
	return ((prx==19 && ptx==18) ||	// Serial1
					(prx==17 && ptx==16) || // Serial2
					(prx==15 && ptx==14)); // Serial3
}

// is_demux_serial()
// tells if there is an Atlas Scientific RS-232 serial demuxer between
// our sensor pins and the sensor stamp
// in which case we need to select channel on the demuxer
// to redirect our pins (Z-block, cross TX/RX)
// to the stamp pins (Y-block on demuxer, *straight* RX/TX!)
bool AtlasScientificSensor::is_demux() {
	// decision is made if S0 and S1 filled and a valid channel is provided
	return (this->sensor != NULL && this->sensor->channel >= 0 &&
			this->sensor->channel < 4 &&
			this->sensor->asenc->S0 != 0 && this->sensor->asenc->S1 != 0);
}
// selects SoftwareSerial channel via the RS-232
// adopted from the Atlas-Scientific sample code for RS-232
void AtlasScientificSensor::openChannel(short channel) {
	// todo DEBUG log level
	char szt[64]="";
	sprintf_P(szt,PSTR("XC%d=%d,%d,%d"),(int)channel,(int)this->sensor->asenc->S0,(int)this->sensor->asenc->S1,(int)this->sensor->asenc->E);
	APDDebugLog::log(APDUINO_MSG_ATLAS_CO,szt);
	switch (channel) {
	case 0: //open channel Y0
		digitalWrite(this->sensor->asenc->S0, LOW); //S0=0
		digitalWrite(this->sensor->asenc->S1, LOW); //S1=0
		break;
	case 1: //open channel Y1
		digitalWrite(this->sensor->asenc->S0, HIGH); //S0=1
		digitalWrite(this->sensor->asenc->S1, LOW);  //S1=0
		break;
	case 2: //open channel Y2
		digitalWrite(this->sensor->asenc->S0, LOW);  //S0=0
		digitalWrite(this->sensor->asenc->S1, HIGH); //S1=1
		break;
	case 3: //open channel Y3
		digitalWrite(this->sensor->asenc->S0, HIGH); //S0=1
		digitalWrite(this->sensor->asenc->S1, HIGH); //S1=1
		break;
	default:
		APDDebugLog::log(APDUINO_MSG_ATLAS_CO,"DMX_err");	// TODO
	}
	// pull E low (if specified)
	if (this->sensor->asenc->E != 0) {
		digitalWrite(this->sensor->asenc->E, LOW);
		APDDebugLog::log(APDUINO_MSG_ATLAS_CO,"E.");
	}
	APDDebugLog::log(APDUINO_MSG_ATLAS_CO,"DMX.");


	// print('\r');
	// the print cr was put in place to improve stability.
	// sometimes, when switching channels errant data
	// was passed. The print CR clears any incorrect data that was
	// transmitted to atlas scientific device.
	// ok. let's do so...
	if (this->is_hw_serial()) {
		HardwareSerial *sp = (HardwareSerial *)this->sensor->asenc->serialport;
		sp->print('\r');
	} else {
#ifdef ATLAS_SOFTSERIAL
		SoftwareSerial *sp = (SoftwareSerial *)this->sensor->asenc->serialport;
		sp->print('\r');
#else
		// TODO log not supported
#endif
	}
	return;
}

// opens the sensor's SW serial channel (if any)
void AtlasScientificSensor::openChannel() {
	if (this->sensor && this->sensor->channel >=0 && this->sensor->channel < 4	&& // we have sensor and channel spec
			this->sensor->asenc->S0 != 0 && this->sensor->asenc->S1 != 0 ) {		// S1,S1 provided
			if ( this->sensor->asenc->csel != this->sensor->channel ) {				// not the current channel
				this->openChannel(this->sensor->channel);
			}	// this is not an error, already selected
	} else {
		APDDebugLog::log(APDUINO_ERROR_ATLAS_DEMUX,NULL);		// config error
	}
}



// opens the hardware serial port specified by the sensor pin
bool AtlasScientificSensor::selectHWSerial()
{
	bool bRetCode = false;
	if (this->sensor != NULL && ((ASSENS*)(this->sensor))->asenc->serialport == NULL) {
		HardwareSerial* pser = NULL;
		if (this->config.sensor_pin - this->config.sensor_secondary_pin == 1) {	// hardware serial pins must be neighbours, TX RX order
			switch (this->config.sensor_pin) {
			case 19:	// TX 18 RX 19 = Serial1
				pser = &Serial1;
				APDDebugLog::log(APDUINO_MSG_ATLAS_HW,"1");				// todo DEBUG level
				break;
			case 17:	// TX 16 RX 17 = Serial2
				pser = &Serial2;
				APDDebugLog::log(APDUINO_MSG_ATLAS_HW,"2");				// todo DEBUG level
				break;
			case 15:	// TX 14 RX 15 = Serial3
				pser = &Serial3;
				APDDebugLog::log(APDUINO_MSG_ATLAS_HW,"3");				// todo DEBUG level
				break;
			default:
				APDDebugLog::log(APDUINO_ERROR_ATLAS_HW,NULL);				// todo DEBUG level
				// todo log this when enabled log levels SerPrintP("E");			// ERROR invalid RX PIN for Hardware Serial
				;;
			}
		} else {
			// todo log this when enabled log levels SerPrintP("E");			// ERROR invalid PIN sequence for Hardware Serial
		}
		// todo check serial state
		((ASSENS*)(this->sensor))->asenc->serialport = (void *)pser;		// STORE pointer to serial port
		if (pser) {// && this->is_demux()) {
			this->openChannel();
		}
		bRetCode = (pser != NULL);			// return true if a HW serial was selected
	} else {
		// todo log this when enabled log levels SerPrintP("E");				// ERROR, invalid object
	}
	return bRetCode;
}

#ifdef ATLAS_SOFTSERIAL
// opens the sw serial port specified by the sensor pin
bool AtlasScientificSensor::selectSWSerial()
{
	bool bRetCode = false;
	APDDebugLog::log(APDUINO_MSG_ATLAS_SW,NULL);				// todo DEBUG level
	if (this->sensor != NULL && ((ASSENS*)(this->sensor))->asenc->serialport == NULL) {
		// allocate a new SoftwareSerial
		SoftwareSerial *pser = new SoftwareSerial(this->config.sensor_pin, this->config.sensor_secondary_pin );
		if (pser != NULL) {
			((ASSENS*)(this->sensor))->asenc->serialport = (void *)pser;		// STORE pointer to serial port
			if (pser) {// && this->is_demux()) {
				this->openChannel();
			}
			// todo log this when enabled log levels
			APDDebugLog::log(APDUINO_MSG_ATLAS_SW,NULL);
			bRetCode = true;
		} // else?
	} else {
		// todo log this when enabled log levels
		APDDebugLog::log(APDUINO_ERROR_ATLAS_SW,NULL);
	}
	return bRetCode;
}
#endif

boolean AtlasScientificSensor::perform_check()
{
	float nv = this->as_sensor_read();
	// todo revise the special values (-50,-100) below and change sensors to hold validity bit in _state (indicating if reading was valid) instead of altering value with custom "invalid values"
	if (this->_state == STATE_READY && nv != NAN) this->fvalue = nv;
	return (nv != NAN);
}


float AtlasScientificSensor::as_sensor_read()
{
	float sensorval = NAN;

	if (this->sensor->asenc == NULL || this->sensor->asenc->serialport == NULL ) {
		APDDebugLog::log(APDUINO_MSG_ATLAS,"cferr");	// todo log this when enabled log SerPrintP("E");			// error, no sensor port etc.
		return NAN;
	}

	if (this->_state == STATE_READY) {			// attempt to trigger reading if ready
		APDDebugLog::log(APDUINO_MSG_ATLAS,"rstart");	// todo log this when enabled log SerPrintP("E");			// error, no sensor port etc.
		if (this->sensor->asenc->state == STATE_READY) {		// if serial port is ready
			this->sensor->asenc->state = STATE_BUSY;				// set the AS serial port flag as busy (reused on the same pin)
			this->_state = STATE_WRITE;							// set this class state as write

			this->openChannel();			// this opens channel if we're using a demuxer

			// instruct reading sensor
			this->print("R\r");				// AtlasScientific read command (valid for all classes /pH,EC,DO,ORP/)
			//this->print('\r');				// command confirmed by carriage return

			// and "come back" in 1100 ms for results
			this->pmetro->interval(1100);							// reschedule checking this sensor in 1100 ms (will go to the STATE_WRITE branch)
			char sztmp[32]="";						// extra log info
			sprintf_P(sztmp,PSTR("cmds@%l"),(millis() - this->_lm));	// fill out extra log info
			APDDebugLog::log(APDUINO_MSG_ATLAS_TX,sztmp);	// todo this should be DEBUG level
			this->_lm = millis();
		} else {												// the serial port is occupied (by another instance using the shared serial port)
			//SerPrintP(".");
			this->pmetro->interval(10);												// set a short interval so we time out quickly for retry
		}
	} else if (this->_state == STATE_WRITE) {		// if ready to fetch data
		APDDebugLog::log(APDUINO_MSG_ATLAS,"F");	// todo log this when enabled log SerPrintP("E");			// error, no sensor port etc.
		// no need to this->openChannel(); we assume states were respected and nobody changed channel if we're using a demuxer
		// we assume nobody else is using the shared object in this state, should be STATE_BUSY (by this sensor)
		int databytes = 0;		// bytes to read
		char sz_rx[32] = "";		// normally Atlas should send up to 12 chars, this is for the future
		databytes = this->fetch(sz_rx);
		// todo log this when enabled log
		char sztmp[32]="";
		sprintf_P(sztmp,PSTR("%d b: %s"),databytes,sz_rx);
		APDDebugLog::log(APDUINO_MSG_ATLAS_RX,sztmp);	// todo this should be DEBUG level

		float sensorval = 0;
		if (sscanf(sz_rx, "%f", &sensorval) == 0) {
			APDDebugLog::log(APDUINO_ERROR_ATLAS_DATA,sz_rx);	// todo this should be DEBUG level
		} else {
			this->fvalue = sensorval;
			// logging : todo DEBUG level
			sprintf_P(sztmp,PSTR("%f"),this->fvalue);
			APDDebugLog::log(APDUINO_MSG_ATLAS_STO,sztmp);	// todo this should be DEBUG level
		}

		// todo log this when enabled log DEBUG
		sprintf_P(sztmp,PSTR("%s(%d):%f(in %l)"),this->config.label,this->config.sensor_pin,this->fvalue,(millis() - this->_lm));
		APDDebugLog::log(APDUINO_MSG_ATLAS_RX,sztmp);	// todo this should be DEBUG level
		this->_lm = millis();

		// set ready states
		this->_state = STATE_READY;
		this->sensor->asenc->state = STATE_READY;
		this->pmetro->interval(this->config.sensor_freq);						// reset normal poll time
	} else {
		// todo log this when enabled log levels ("Unknown state. Slowing sensor polling.\n");
	  APDDebugLog::log(APDUINO_MSG_ATLAS,"SD");	// todo this should be WARNING level
		this->pmetro->interval(this->config.sensor_freq*10);
	}
	return sensorval;
}

void AtlasScientificSensor::command(const void *cmd) {
	char sztmp[RCV_BUFSIZ] ="";
	const char *pszcmd = (const char*)cmd;
	if (this->_state == STATE_READY) {			// attempt to trigger reading if ready
		this->_state == STATE_BUSY;
		this->sensor->asenc->state = STATE_BUSY;
		// todo check if valid command? -> no. user is responsible to read Atlas docs. before sending commands.
		this->openChannel();			// will open the channel if using demuxer
		APDDebugLog::log(APDUINO_MSG_ATLAS_TX,pszcmd);	// todo this should be DEBUG level
		this->print(pszcmd); this->print("\r");	// confirm command
		APDDebugLog::log(APDUINO_MSG_ATLAS_TX,"ok");	// todo this should be DEBUG level
		// lets wait for a response in this version...
		int i=0;
		while (!this->available() && i<500) {		// wait for incoming bytes for about 5sec max
			delay(10);
			i++;
		}
		i=0;			// from now we use it to count received bytes

		while (this->available()&& i<RCV_BUFSIZ-1) {			// if there are bytes
			char c = (char)this->read();		// read
			//Serial.print(c);					// and display on serial
			sztmp[i]=c;
			i++;
		}
		sztmp[i]=0;		// terminate string
		// print whatever in buffer
    APDDebugLog::log(APDUINO_MSG_ATLAS_RX,sztmp);	// todo this should be DEBUG level
		this->_lm = millis();

		// set ready states
		this->_state = STATE_READY;
		this->sensor->asenc->state = STATE_READY;
		this->pmetro->interval(this->config.sensor_freq);						// reset normal poll time
	} else {
		APDDebugLog::log(APDUINO_MSG_ATLAS_BUSY,NULL);
	}

}

// print a string to the serial port to the atlas stamp
// if using a demuxer, should open channel first
size_t AtlasScientificSensor::print(const char *psz) {
	if (this->is_hw_serial()) {
		APDDebugLog::log(APDUINO_MSG_ATLAS_HWP,psz);		// TODO DEBUG LEVEL
		HardwareSerial *sp = (HardwareSerial *)this->sensor->asenc->serialport;
		return sp->print(psz);
	} else {
#ifdef ATLAS_SOFTSERIAL
		APDDebugLog::log(APDUINO_MSG_ATLAS_SWP,psz);		// TODO DEBUG LEVEL
		SoftwareSerial *sp = (SoftwareSerial *)this->sensor->asenc->serialport;
		return sp->print(psz);
#else
		// TODO log not supported
#endif
	}
}

// print a char to the serial port to the atlas stamp
// if using a demuxer, shoudl open channel first
size_t AtlasScientificSensor::print(const char pc) {
	char szlog[2]=" ";
	szlog[0]=pc;
	if (this->is_hw_serial()) {
		APDDebugLog::log(APDUINO_MSG_ATLAS_HWPC,szlog);		// TODO DEBUG LEVEL
		HardwareSerial *sp = (HardwareSerial *)this->sensor->asenc->serialport;
		return sp->print(pc);
	} else {
#ifdef ATLAS_SOFTSERIAL
		APDDebugLog::log(APDUINO_MSG_ATLAS_SWPC,szlog);		// TODO DEBUG LEVEL
		SoftwareSerial *sp = (SoftwareSerial *)this->sensor->asenc->serialport;
		return sp->print(pc);
#else
		// TODO log not supported
#endif
	}
}

// read bytes if available
// stop on no more bytes or \r
// ... open channel if demuxed
// todo provide bufsize to avoid writing to arbitrary places...
int AtlasScientificSensor::fetch(char *psz_rx) {
	int bytes_avail = this->available();
	int i = 0;
	//char *pc = psz_rx;			// char pointer
	if (bytes_avail > 0) {		// AtlasScientific example was looking for 3+ bytes, why? (\r?)
		for (i = 0; i < bytes_avail ; i++) {
			psz_rx[i] = (char)this->read();			// this will be slow (each time checks if SW/HW) TODO consider using 2 separate iterations
			if (psz_rx[i] == '\r') {					// if end of command (CR)
				//psz_rx[i] = 0;							// replace \r with \0
				break;									// get outta here
			}
		}
		psz_rx[i] = 0;							// replace \r with \0, as well as terminate if no more bytes
	}
	return i;
}

// returns number of available bytes on HW/SW serial, -1 on error
int AtlasScientificSensor::available() {
	int bytes_available = -1;
	if (this->sensor && this->sensor->asenc && this->sensor->asenc->serialport) {
		bytes_available = ( this->is_hw_serial() ?
				(((HardwareSerial *)this->sensor->asenc->serialport)->available()) :
#ifdef ATLAS_SOFTSERIAL
				(((SoftwareSerial *)this->sensor->asenc->serialport)->available()) );
#else
		    NAN);		// Software serial not supported
#endif
	}
	return bytes_available;
}


// returns a character from HW/SW serial, -1 on error
// open channel first if using a demuxer
int AtlasScientificSensor::read() {
	int iread = 0;
	if (this->sensor && this->sensor->asenc && this->sensor->asenc->serialport) {
		iread = this->is_hw_serial() ?
				((HardwareSerial *)this->sensor->asenc->serialport)->read() :
#ifdef ATLAS_SOFTSERIAL
				((SoftwareSerial *)this->sensor->asenc->serialport)->read();
#else
				NAN;		// Software Serial not supported
#endif
	}
	return iread;
}
