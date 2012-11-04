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
 * APDuino.cpp
 *
 *  Created on: Mar 26, 2012
 *      Author: George Schreiber
 */

#include "APDuino.h"
#include "apd_utils.h"
#include <stdlib.h>
#include <avr/pgmspace.h>
//#include <SdFat/Sd2PinMap.h>

#define PPrintP(s) PrintP(PSTR(s));

// TODO add a constructor that passes callback function pointers (that take strings) to allow display refresh

APDuino::APDuino(long baudrate) {
  init(baudrate);
}

APDuino::APDuino() {
  init(9600);
}

APDuino::~APDuino() {
	SdBaseFile::dateTimeCallbackCancel();			// cancel any callback might have been set for storage datetime
  this->bProcessRules = false;
  free(this->pra);
  free(this->pca);
  free(this->psa);

  free(this->pAPDWeb);
  free(pAPDSerial);
}



void APDuino::init(long baudrate) {
  // TODO Auto-generated constructor stub
  // override these by loading from disk
  //start_time = millis();         // store the startup time; will be used later on to calc. uptime

  for (int i=0; i<10; i++) pcustfuncs[i] = NULL;         // reset custom function pointers

  pAPDSerial = NULL;

  pstr_device_id = NULL;
  pstr_APDUINO_API_KEY = (char *)malloc(sizeof(char)*65);
  strcpy(pstr_APDUINO_API_KEY,"");
  pstr_Name= (char *)malloc(sizeof(char)*16);
  strcpy_P(pstr_Name,PSTR("APDuino"));

  pAPDSerial = new APDSerial(baudrate);
  delay(10);                                    // seems a small delay helps

  //psa = NULL;
  // TODO add checks on allocations
  psa = new APDSensorArray();
  pca = new APDControlArray(&pcustfuncs);
  pra = new APDRuleArray(psa,pca,&(this->bfIdle));

  pAPDWeb = NULL;

  pIdleMetro = NULL;
  pLoggingMetro = NULL;

  bFirstLoopDone = false;
  bInitialized = false;
  bProcessRules = false;

  SerPrintP("\n\nAPDuinOS "); SerPrintP(APDUINO_VERSION);   SerPrintP("."); SerPrintP(APDUINO_BUILD);   SerPrintP(" starting up...\n");
  Serial.print( freeMemory(), DEC); SerPrintP(" RAM free.\n");

  //Serial.Print(pstr_Name);
  //SerPrintP(" - APDuino class started... STORAGE:"); 	Serial.print((unsigned int)pAPDStorage,DEC);
  //SerPrintP("API KEY: "); Print(pstr_APDUINO_API_KEY); SerPrintP("\n");

  // TODO we should store the results and switch to an alternative sprintf/printf (once implemented)
  if (!testprintf() || !testscanf()) {
  	APDDebugLog::log(APDUINO_ERROR_SSCANF,NULL);
  } else {

  }

  bAPDuinoConfigured = false;

  // now comes the setup
#ifdef DEBUG
  SerPrintP("Initializing...\n");
#endif
#ifdef ENABLE_DISPLAY
  display_callback("Initializing...");
#endif
  delay(30);		// allow hw to power-up

//  setupWithStorage(iChip,iSpeed);					// set up Time, SD, Ethernet, load SA,CA,RA
	// end of setup / initialization
}


boolean APDuino::init_app() {
	// we need storage!
	if (APDStorage::ready()) {
		// enable sending debug log to SD card
		APDLogWriter::begin();							// this starts the DEBUG log writer (not the data log!)
		APDLogWriter::write_debug_log();		// write any buffered messages

		this->setup_timekeeping();
		delay(50);

		this->setup_networking();  // TODO provide details here
		delay(50);

		//bInitialized = (pAPDTime != NULL && pAPDWeb != NULL);
		bInitialized = (pAPDWeb != NULL);

		// TODO - hardcoded NTP address - allow user to set NTP server
		//byte hackts[4] = ;

		//this->pAPDTime->setupNTPSync(8888, DEFAULT_TIMESERVER_IP,1,1);
		APDTime::setup_ntp_sync(8888, DEFAULT_TIMESERVER_IP,1,1);
		this->check_timekeeping();
	#ifdef DEBUG
		SerPrintP("\ninit sensors\n");
	#endif
		//this->setupSensors();
		this->psa->loadSensors();
		//SerPrintP("APD Sensors - ok.\n");
		//GLCD.Puts(".");
	#ifdef DEBUG
		//setup_apd_controls();
		SerPrintP("\ninit controls\n");
	#endif
		//this->setupControls();
		this->pca->load_controls();
		//SerPrintP("APD Controls - ok.\n");
		//GLCD.Puts(".");
	#ifdef DEBUG
		//setup_apd_rules();
		SerPrintP("init rules\n");
	#endif
		//this->setupRules();
		this->pra->load_rules();
		//SerPrintP("APD Rules - ok.\n");

		// enable "real-time" rule evaluation
		this->psa->enableRuleEvaluation(&(APDRuleArray::evaluate_sensor_rules),(void *)this->pra);

		// FIXME check if we are initialized, set it in bConfigured
		// TODO revise what is obligatory. for now, 1 sensor or control is enough to be considered as configured
		this->bAPDuinoConfigured =  APDStorage::ready() && (this->psa->iSensorCount > 0  || this->pca->iControlCount > 0); // && this->pra->iRuleCount > 0;
		//(this->iRuleCount > 0 && this->iSensorCount > 0 && this->iControlCount > 0 && this->pAPDStorage->ready());       // TODO

		// this was done outside before
#ifdef ENABLE_DISPLAY
		display_callback(".");
#endif

	// adding custom functions should be done outside
		//SerPrintP("APD: WWWSRV...");
		if (this->start_webserver()) {
//				SerPrintP("OK.\n");
//		} else {
//				SerPrintP("FAIL.\n");
		}
#ifdef ENABLE_DISPLAY
		display_callback(".");
		delay(500);
#endif

#ifdef DEBUG
		SerPrintP("APD: WEBLOG TO: ");
		SerDumpIP(serverip);
		SerPrintP("\n");
#endif

		//SerPrintP("APD: APDUINO ONLINE...\n");
		if (this->pAPDWeb->setupAPDuinoOnline()) { //  && this->bConfigured()
//				SerPrintP("OK.");
//		} else {
//				SerPrintP("ERR.");
		}
		//SerPrintP("APD: COSM...\n");
		if (this->pAPDWeb->setupCosmLogging()){
		//		SerPrintP("OK.");
		//} else {
		//		SerPrintP("ERR.");
		}
		//SerPrintP("APD: THINGSPEAK...\n");
		if (this->pAPDWeb->setupThingSpeakLogging()){
//				SerPrintP("OK.");
//		} else {
//				SerPrintP("ERR.");
		}
	//}
#ifdef ENABLE_DISPLAY
		display_callback(".");
		delay(500);
#endif

		if (bConfigured()) {
			//SerPrintP("APD: LOGGING...");
			if (this->start_logging(DEFAULT_ONLINE_LOG_FREQ)) {		// TODO revise, this should be configurable
//					SerPrintP("OK.\n");
//			} else {
//					SerPrintP("FAIL.\n");
			}
		}

		delay(100);
		//pAPD->startIdling(uIdleDuration);           // used for idling device
	}
}

void APDuino::setup_with_storage(int iChip, int iSpeed) {
  delay(250);    // give the hw some time, we're probably just powering on
  // check if storage was initialized with success
  // check we have APDWeb allocated
  if (setup_storage(SS_PIN,iChip,iSpeed)) {
			init_app();
  } else {
  	APDDebugLog::log(APDUINO_ERROR_STORAGENOTSETUP,NULL);
	}
}

unsigned long APDuino::getUpTime() {
  // TODO nullptrchk
  return APDTime::getUpTime();
}

char *APDuino::get_uptime_str(char *psz_uptime) {
  // TODO nullptr chk
  return APDTime::get_uptime_str(psz_uptime);
}

boolean APDuino::storage_ready() {
	APDStorage::ready();
}


void APDuino::setup_timekeeping() {
#ifdef DEBUG
  SerPrintP("Time...");
#endif
//  if (this->pAPDTime == NULL) {
    if (!APDTime::started()) {
//      this->pAPDTime = new APDTime(true);       // try with RTC
  		APDTime::begin(true);       // try with RTC
  } else {
  	APDDebugLog::log(APDUINO_WARNING_TIMEALREADYSETUP,NULL);
  }
}

void APDuino::check_timekeeping() {
#ifdef DEBUG
  SerPrintP("Time check...");
#endif
  //if (this->pAPDTime != NULL) {
  if (APDTime::started()) {
      // TODO add NTP switch
      APDTime::sync_to_ntp();
      SerPrintP("check@:"); Serial.print((unsigned long)APDTime::now().unixtime(),DEC);
      char tbuf[20] = "1970/01/01 00:00:00";
      SerPrintP("now: "); Serial.print(APDTime::nowS(tbuf)); SerPrintP("...");
  } else {
  	APDDebugLog::log(APDUINO_ERROR_NOTIMEOBJECT,NULL);
  }
}

DateTime APDuino::timeNow() {
  return APDTime::now();
}



void APDuino::print(char *string){
 if (pAPDSerial != NULL) {
   pAPDSerial->print(string);
 }
}
void APDuino::printP(void *Pstring) {
  if (pAPDSerial != NULL) {
     pAPDSerial->printP(Pstring);
   }
}
void APDuino::debug(char *string, int iMsgLevel) {
  if (pAPDSerial != NULL && iMsgLevel <= iDebugLevel) {
     pAPDSerial->printP(string);
   }
}
void APDuino::debugP(void *Pstring, int iMsgLevel) {
  if (pAPDSerial != NULL && iMsgLevel <= iDebugLevel) {
     pAPDSerial->printP(Pstring);
   }
}





void APDuino::loop() {
  this->loop_core();            // runs also in Service Mode

  if (this->bAPDuinoConfigured) {             // if configured
#ifdef DEBUG
  	SerPrintP("-");
#endif
    this->loop_operations();                          // run the real ops
#ifdef DEBUG
    SerPrintP(">");
#endif

    // this following hack is to enable rules only after we should have read sensors
    if (this->bFirstLoopDone == false ) { //&& bProcessRules == false) {
    	APDDebugLog::log(APDUINO_MSG_ENABLERULEPROC,NULL);

    	this->bFirstLoopDone = true;
    	this->bProcessRules = true;
    } //else {
     //SerPrintP("FR:"); Serial.print(this->bFirstLoopDone); SerPrintP("PR:"); Serial.print(this->bProcessRules); SerPrintP("---");
    //}
#ifdef DEBUG
    else {
    	if (this->bProcessRules) {
    		SerPrintP("PROC RULES");
    	}
    	if (this->bFirstLoopDone) {
				SerPrintP("NOTFIRSTRUN");
			} else
    	delay(100);
    }
#endif
    delay(1);
  } else {
#ifdef DEBUG
  	//SerPrintP("UNCONF.");
#endif
  	delay(1);
  }
}

void APDuino::loop_core() {
//  sercom();           // loop serial control
	APDLogWriter::write_debug_log();			// write out any messages
	// TODO : periodically rotate log files if size exceeded max
	// APDStorage::rotate_file(APDLogWriter::szlogfname,MAX_LOG_SIZE);		// rotate debug log if needed
	// APDStorage::rotate_file("APDUINO.LOG",MAX_LOG_SIZE);		// rotate debug log if needed


  if (pAPDWeb != NULL) {

    pAPDWeb->loop();          // loop www services

    if (pAPDWeb->dispatched_requests) {					// process dispatched request buffer
    	int request = pAPDWeb->dispatched_requests;			// copy the request from the buffer
    	pAPDWeb->dispatched_requests = DREQ_NOOP;				// reset the buffer to avoid reexec (by any mistake later)
    	switch (request) {												// execute the request
    	case DREQ_RECONF:														// reload configuration
    		this->reconfigure();
    		break;
    	case DREQ_RESET:
    		soft_reset();														// soft-reset is an improper way to restart (does not reset hardware)
    		break;
    	case DREQ_RELOADRULES:
    		this->reload_rules();
    		break;
    	default:
    		APDDebugLog::log(APDUINO_WARN_UNKNOWNREQUEST,NULL);
    	}
    }
  }
  delay(1);
}


void APDuino::loop_operations() {
  unsigned long ms=millis();

  // TODO selectLayout (callback)
  //select_layout(); // will check if selection must be made (set selectLayout to the desired layout, or bRepaint for repaint)

  // TODO screen_update (callback)
  //screen_update();

  if (pIdleMetro != NULL && pIdleMetro->check()) {
#ifdef DEBUG
    SerPrintP("IDLING...\n");
#endif
    this->idle_device();
  }

  //if (pAPDSensors != NULL ) {  // do we really need a metro here? or only the individual metros ... && pSensorMetro != NULL && pSensorMetro->check()) {
  if (psa != NULL ) {  // do we really need a metro here? or only the individual metros ... && pSensorMetro != NULL && pSensorMetro->check()) {
    //SerPrintP("SENSORCHECK -> ");
    //delay(20);
#ifdef DEBUG
    char tbuf[11] = "";
    sprintf(tbuf,"SENS(%d)",iNextSensor);
    Serial.println(tbuf);
#endif
    //glcd_debug_update(tbuf);
    this->psa->pollSensors(this->bProcessRules);
//    SerPrintP("SENSORCHKDONE");
  }// else
  delay(1);
  //if (pAPDRules != NULL && iRuleCount > 0)  loop_apd_rules();
  if (this->pra != NULL && this->bProcessRules == true) {
  	//SerPrintP("RULEEVAL");
  	this->pra->loop_rules();
  }
#ifdef DEBUG
  else {
  	SerPrintP("NOT LOOPING RULES!");
  }
#endif

  if (pLoggingMetro != NULL && pLoggingMetro->check()) {
    //glcd_debug_update("LOGGING");
    this->log_data();
  }
}

//APDStorage *APDuino::setupStorage(int iSS, int iChip, int iSpeed) {
bool APDuino::setup_storage(int iSS, int iChip, int iSpeed) {
	boolean bret = false;
	SerPrintP("Storage ");
	if (bret = APDStorage::begin(iSS,iChip,iSpeed)) {
		// set time callback for SDFatLib
		SdBaseFile::dateTimeCallback( &(APDTime::SdDateTimeCallback) );
	} else {
			SerPrintP("Not ");
	}
	SerPrintP("Ready.\n");
	return bret;
}

// return true if APDuino is configured for normal operations
// the actual value returned is not dynamically calculated
// but read from bAPDuinoConfigured that is set during conf/setup ops.
boolean APDuino::bConfigured() {
  return bAPDuinoConfigured;
}

// adds a custom function pointer (external code) to the custom functions array to a given pos.
// APDControls then can call these 'software controls'
//FIXME direct references to controls
int APDuino::add_custom_function(int iPos, void (*pcf)()){
	// todo log this when enabled log levels ("SET CUSTFUNC @IDX "); Serial.print(iPos);
  if (this->pcustfuncs[iPos] == NULL) {
  	// todo log this when enabled log levels ("NULL, SET: "); Serial.print((unsigned int)pcf,DEC);

      this->pcustfuncs[iPos] = pcf;
      // loop though controls, see if any of them is referring to the given index
      for (int i=0; i<this->pca->iControlCount; i++) {
        APDControl *pc = this->pca->pAPDControls[i];
        if (pc != NULL && pc->config.control_type == SOFTWARE_CONTROL ) {
        	// todo log this when enabled log levels ("SW CTRL @ IDX");Serial.print(i);SerPrintP(" - ");

          if (pc->config.control_pin == iPos) {
            if (this->pcustfuncs[pc->config.control_pin] != NULL) {
              // assign the custom function pointer to the control; take control's PIN as IDX
            	// todo log this when enabled log levels ("SET CUSTFUNC IDX "); Serial.print(pc->config.control_pin); SerPrintP(" CTRLIDX"); Serial.print(i); SerPrintP(".\n");

            	pc->pcustfunc = this->pcustfuncs[pc->config.control_pin];      // cvalue must hold the cfunc idx
            } else {
            	APDDebugLog::log(APDUINO_WARN_NOCUSTFUNCATIDX,NULL);
            }
          } else {
          	APDDebugLog::log(APDUINO_WARN_CUSTFUNCMISMATCH,NULL);
          }
        }
      }
      return iPos;
  }
  return -1;
}

// turn on/off rule processing
boolean APDuino::toggleRuleProcessing() {
  bProcessRules = (this->pra->pAPDRules != NULL ? !bProcessRules : false);
  return bProcessRules;
}

// enable execution of rules
boolean APDuino::enableRuleProcessing() {
  bProcessRules = true;
  return bProcessRules;
}

// disable execution of rules
boolean APDuino::disableRuleProcessing() {
  bProcessRules = false;
  return bProcessRules;
}

boolean APDuino::reconfigure() {
	boolean retcode = false;
	// todo log this when enabled log levels ("\nRECONREQ...\n");
  delay(100);

  // put APDWeb in maintenance mode to PREVENT ACCESS TO sensors, controls, rules
  if (this->pAPDWeb->pause_service()) {
  	boolean bProcRulesOld = bProcessRules;			// store old rule processing state
    bProcessRules = false;
    unsigned long ulRam = freeMemory();

    // todo log this when enabled log levels ("Reconfiguring Arrays!\n");	Serial.print( ulRam, DEC); SerPrintP(" RAM free.\n");
		this->iNextSensor = -1;				// invalidate next sensor index

		delete(this->pra);
		// todo log this when enabled log levels ("Deleted Rule Array.\n");	Serial.print( freeMemory(), DEC); SerPrintP(" RAM free.\n");
		delete(this->pca);
		// todo log this when enabled log levels ("Deleted Control Array.\n");	Serial.print( freeMemory(), DEC); SerPrintP(" RAM free.\n");
		delete(this->psa);
		// todo log this when enabled log levels ("Deleted Sensor Array.\n");	Serial.print( freeMemory(), DEC); SerPrintP(" RAM free.\n");	\
							SerPrintP("Deleted Arrays!\n"); \
							Serial.print( freeMemory(), DEC); SerPrintP(" RAM free.\n");


		psa = new APDSensorArray();
		pca = new APDControlArray(&pcustfuncs);
		pra = new APDRuleArray(psa,pca,&(this->bfIdle));
		// todo log this when enabled log levels ("Reallocated Arrays!\n");  ( freeMemory(), DEC) (" RAM free.\n"); ("\ninit sensors\n");
  	this->psa->loadSensors();
  	// todo log this when enabled log levels ("APD Sensors - ok.\n");
		//GLCD.Puts(".");

  	// todo log this when enabled log levels("\ninit controls\n");

		this->pca->load_controls();
		//SerPrintP("APD Controls - ok.\n");
		//GLCD.Puts(".");

		// todo log this when enabled log levels ("init rules\n"); delay(10);

	  this->pra->load_rules();
	  // todo log this when enabled log levels ("APD Rules - ok.\n");

		// Update pointers in APDWeb
		this->pAPDWeb->pAPDRules = this->pra->pAPDRules;
		this->pAPDWeb->iRuleCount = this->pra->iRuleCount;
		this->pAPDWeb->pAPDControls = this->pca->pAPDControls;
		this->pAPDWeb->iControlCount = this->pca->iControlCount;
		this->pAPDWeb->pAPDSensors = this->psa->pAPDSensors;
		this->pAPDWeb->iSensorCount = this->psa->iSensorCount;

#ifdef DEBUG
	  	SerPrintP("Reconfigured Arrays!\n");
	  	Serial.print( freeMemory(), DEC); SerPrintP(" RAM free.\n");
	  	delay(10);

	  	if (ulRam == freeMemory()) {
	  		SerPrintP("No memory leak detected.\n");
	  	} else {
	  		SerPrintP("\nMEMORY LEAK DETECTED!\n");
	  	}
#endif
		// enable "real-time" rule evaluation
		this->psa->enableRuleEvaluation(&(APDRuleArray::evaluate_sensor_rules),(void *)this->pra);

		// FIXME check if we are initialized, set it in bConfigured
		// TODO revise what is obligatory. for now, 1 sensor or control is enough to be considered as configured
		this->bAPDuinoConfigured =  APDStorage::ready() && (this->psa->iSensorCount > 0  || this->pca->iControlCount > 0); // && this->pra->iRuleCount > 0;

		this->bFirstLoopDone = false;									// we have not yet looped with the new config (no sensor values)
		bProcessRules = bProcRulesOld;								// restore old rule processing state
		retcode = this->pAPDWeb->continue_service();	// return if web server continues processing
  } else {
  	APDDebugLog::log(APDUINO_ERROR_COULDNOTPAUSEWWW,NULL);
  }

  return retcode;
}

boolean APDuino::reload_rules() {
	boolean retcode = false;
	// todo log this when enabled log levels ("\nRECONREQ...\n");
  delay(100);

  // put APDWeb in maintenance mode to PREVENT ACCESS TO sensors, controls, rules
  if (this->pAPDWeb->pause_service()) {
  	boolean bProcRulesOld = bProcessRules;			// store old rule processing state
    bProcessRules = false;
    unsigned long ulRam = freeMemory();

    // todo log this when enabled log levels("Reconfiguring Arrays!\n") ( ulRam, DEC); SerPrintP(" RAM free.\n");
		this->iNextSensor = -1;				// invalidate next sensor index

		delete(this->pra);
		pra = new APDRuleArray(psa,pca,&(this->bfIdle));
	  this->pra->load_rules();
	  // todo log this when enabled log levels ("APD Rules - ok.\n");

		// Update pointers in APDWeb
		this->pAPDWeb->pAPDRules = this->pra->pAPDRules;
		this->pAPDWeb->iRuleCount = this->pra->iRuleCount;

		// todo log this when enabled log levels ("Reconfigured Arrays!\n");	Serial.print( freeMemory(), DEC); SerPrintP(" RAM free.\n"); \
	  	if (ulRam == freeMemory()) { \
	  		SerPrintP("No memory leak detected.\n"); \
	  	} else { \
	  		SerPrintP("\nMEMORY LEAK DETECTED!\n"); \
	  	}

		// enable "real-time" rule evaluation
		this->psa->enableRuleEvaluation(&(APDRuleArray::evaluate_sensor_rules),(void *)this->pra);

		// FIXME check if we are initialized, set it in bConfigured
		// TODO revise what is obligatory. for now, 1 sensor or control is enough to be considered as configured
		this->bAPDuinoConfigured =  APDStorage::ready() && (this->psa->iSensorCount > 0  || this->pca->iControlCount > 0); // && this->pra->iRuleCount > 0;

		this->bFirstLoopDone = false;									// we have not yet looped with the new config (no sensor values)
		bProcessRules = bProcRulesOld;								// restore old rule processing state
		retcode = this->pAPDWeb->continue_service();	// return if web server continues processing
  } else {
  	APDDebugLog::log(APDUINO_ERROR_COULDNOTPAUSEWWW,NULL);
  }

  return retcode;
}


void APDuino::new_ethconf_parser(void *pAPD, int iline, char *psz) {
  NETCONF nc;
  // todo log this when enabled log levels ("NETCONF READ: "); Serial.print(psz);
  int ab;
  //            mac6              |IP4         |MASK4       |GW4         |UDPPort,WWWPort
  //            DE AF DA DB AD ED ,c0 a8 01 ea, ff ff ff 00 ,c0 a8 01 fe ,8888,80
  //char myconf[]="DEAFDADBADED c0a801ea ffffff00 c0a801fe 8888 80";
  int ips = sscanf_P( psz, PSTR("%2x%2x%2x%2x%2x%2x %2x%2x%2x%2x %2x%2x%2x%2x %2x%2x%2x%2x %2x%2x%2x%2x %d %d"),
      &(nc.mac[0]),&(nc.mac[1]),&(nc.mac[2]),&(nc.mac[3]),&(nc.mac[4]),&(nc.mac[5]),
      &(nc.ip[0]),&(nc.ip[1]),&(nc.ip[2]),&(nc.ip[3]),
      &(ab),&(nc.subnet[1]),&(nc.subnet[2]),&(nc.subnet[3]),
      &(nc.gateway[0]),&(nc.gateway[1]),&(nc.gateway[2]),&(nc.gateway[3]),
      &(nc.pridns[0]),&(nc.pridns[1]),&(nc.pridns[2]),&(nc.pridns[3]),
      &(nc.localPort),
      &(nc.wwwPort));
  (nc.subnet[0]) = ab;
  // todo log this when enabled log levels ("Parsed config:\n"); \
					SerPrintP("IP: ");	SerDumpIP(nc.ip); \
					SerPrintP(" MASK: "); SerDumpIP(nc.subnet); \
					SerPrintP(" DNS: "); 	SerDumpIP(nc.pridns); \
					SerPrintP(" GW: "); 	SerDumpIP(nc.gateway); \
					SerPrintP(" UDP: "); 	Serial.println(nc.localPort); \
					SerPrintP(" WWW: "); 	Serial.println(nc.wwwPort);

  if (ips < 24) {			// if did not parse the complete config
  	APDDebugLog::log(APDUINO_ERROR_BADNETCONFIG,NULL);
  }
  //TODO add compatibility code (so other options can follow, even if not parsed)
  if (((APDuino *)pAPD)->pAPDWeb == NULL ) {
  	APDDebugLog::log(APDUINO_MSG_ETHERNETFROMCONF,NULL);
    ((APDuino *)pAPD)->pAPDWeb = new APDWeb(&nc);
  }
}



void APDuino::setup_networking() {
	APDDebugLog::log(APDUINO_MSG_NETINIT,NULL);
  delay(250);                   // probably just starting up, adding a little delay, 1/4s
  if (pAPDWeb == NULL) {      // replace with check if IP config is present
  	// todo log this when enabled log levels ("trying to load config...");
    if (APDStorage::ready()) {
        if ((APDStorage::read_file_with_parser("ETHERNET.CFG",&new_ethconf_parser,(void*)this)) <= 0) {	// check for errors (0 - no lines, -1 - error)
        	APDDebugLog::log(APDUINO_ERROR_BADNETCONFIG,NULL);
        }
    } else {
    	APDDebugLog::log(APDUINO_ERROR_SUSPECTSTORAGEERR,NULL);
    }
    if (pAPDWeb == NULL) {
    		APDDebugLog::log(APDUINO_MSG_DHCPFALLBACK,NULL);
    		pAPDWeb = new APDWeb();
    }
  } else {
  	APDDebugLog::log(APDUINO_ERROR_NETALREADYSTARTED,NULL);
  }
}

boolean APDuino::start_webserver() {
  boolean retcode = false;
  // todo log this when enabled log levels ("WWWS...");
  if (pAPDWeb != NULL) {
  	// todo log this when enabled log levels ("starting ...");
      pAPDWeb->startWebServer(this->psa->pAPDSensors,this->psa->iSensorCount,this->pca->pAPDControls,this->pca->iControlCount,this->pra->pAPDRules,this->pra->iRuleCount);
      retcode = (pAPDWeb != NULL && pAPDWeb->pwwwserver != NULL && pAPDWeb->pwwwclient !=NULL);
  } else {
  	APDDebugLog::log(APDUINO_ERROR_NONETFORWWW,NULL);
  }
  return retcode;
}


/* Idling - for devices with physical UI */

// used for setting a timeout to make device idle after it expires.
// this is for devices with a physical interface typically to dim/shutdown LCD.
// the application should call startIdling once, upon setup to set the idle timeout.
// uIdleDuration is the value of the timeout in milliseconds
// this is for devices with a physical interface typically to dim/shutdown LCD.
// the application should call startIdling upon completion of any UI action (eg. user pressed a button)
// uIdleDuration is the value of the timeout in milliseconds
void APDuino::start_idling(unsigned long uIdleDuration) {
  if (this->pIdleMetro == NULL) {
      this->pIdleMetro = new Metro(uIdleDuration,true);                // used for idling device
  } else {
      this->pIdleMetro->interval(uIdleDuration);
  }
}


// function to reset idle status and idle metro
// call this on every
void APDuino::unidle_device() {
	// todo log this when enabled log levels ("WAKE UP!\n");
  bfIdle = false;
  if (pIdleMetro != NULL) pIdleMetro->reset();    // UI is not idle
}

void APDuino::idle_device() {
  bfIdle = true;
  if (pIdleMetro != NULL) pIdleMetro->reset();
}

/* End of Idling - for devices with physical UI */


/* Logging - SD card logging */

// internal - call this when ready to log on SD
// ulLoggingFreq - logging frequency in millisecs
// returns true if logging was started, false otherwise
boolean APDuino::start_logging(unsigned long ulLoggingFreq) {
  boolean bLogging = false;
  if (bAPDuinoConfigured && APDStorage::ready() ) {    // check storage status
  	// APDLogWriter::enable_sync_writes(); // enabled by begin(), just to remember enable/disable after/before SD ops
  	if (APDStorage::rotate_file("APDLOG.TXT", MAX_LOG_SIZE) >= 0) {
  			APDDebugLog::log(APDUINO_MSG_SDLOGOK,NULL);
      } else {
      	APDDebugLog::log(APDUINO_ERROR_LOGUNKNOWN,NULL);
      }
      if (this->pLoggingMetro == NULL) {
         this->pLoggingMetro = new Metro(ulLoggingFreq,true);           // ignore missed events
      } else {
      	APDDebugLog::log(APDUINO_MSG_SDLOGRESCHED,NULL);
				this->pLoggingMetro->interval(ulLoggingFreq);
				this->pLoggingMetro->reset();
      }
      bLogging = (this->pLoggingMetro != NULL);
  }
  return bLogging;
}


// logging to SD card in CSV format
// the function reads through all sensors and outputs value if logging is enabled for sensor
// resulting a comma separated line of sensor values that is appended to the log file on SD card
void APDuino::log_data() {
  char logString[128]="";			// TODO increase buffer for controls logging
  char *plog = logString;
  char dataString[20]="";                // make a string for assembling the data to log:
  if (!APDStorage::p_sd->exists("APDLOG.TXT")) {        // if it will be new file
    // FIXME reimplement log_header();                          // start with column names
  }
  // todo log this when enabled log levels ("Assembling log...");
  strcpy_P(dataString,PSTR("1970/01/01 00:00:00"));
  strcpy(plog,APDTime::nowS(dataString));
  plog += strlen(dataString);

  // TODO should check if not printing out of the string!
  for (int i=0;i<this->psa->iSensorCount;i++) {
    if (this->psa->pAPDSensors[i]->config.sensor_log) {                    // if to be logged
        *plog = ',';
        plog++;
        strcpy(plog,this->psa->valueS(i,dataString));
        plog += strlen(dataString);
    }
  }

  // TODO add controls logging
  *plog = 0;  // terminating \0

  APDDebugLog::log(APDUINO_MSG_SDLOGGING,ultoa(strlen(logString),dataString,10));					// debug the number of bytes to be written
  APDStorage::write_log_line("APDLOG.TXT",logString);
  APDDebugLog::log(APDUINO_MSG_SDLOGGINGOK,NULL);					// debug
}
/* End of Logging - SD card logging */
