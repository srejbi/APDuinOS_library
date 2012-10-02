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

  this->bProcessRules = false;
  free(this->pra);
  free(this->pca);
  free(this->psa);

  free(this->pAPDWeb);
  free(this->pAPDStorage);
  free(this->pAPDTime);

  free(pAPDSerial);
  // TODO Auto-generated destructor stub
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

  pAPDStorage = NULL;          // will be the storage

  pAPDTime = NULL;

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
  	Serial.println(APDUINO_ERROR_SSCANF);				// TODO replace this with future error handler
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


boolean APDuino::initApplication() {
	// we need storage!
	if (this->pAPDStorage != NULL && this->pAPDStorage->ready()) {
		this->setupTimeKeeping();
		delay(50);

		this->setupNetworking();  // TODO provide details here
		delay(50);

		bInitialized = (pAPDTime != NULL && pAPDWeb != NULL);

		// TODO - hardcoded NTP address - allow user to set NTP server
		//byte hackts[4] = ;

		this->pAPDTime->setupNTPSync(8888, DEFAULT_TIMESERVER_IP,1,1);
		this->checkTimeKeeping();
	#ifdef DEBUG
		SerPrintP("\ninit sensors\n");
	#endif
		//this->setupSensors();
		this->psa->loadSensors(this->pAPDStorage);
		//SerPrintP("APD Sensors - ok.\n");
		//GLCD.Puts(".");
	#ifdef DEBUG
		//setup_apd_controls();
		SerPrintP("\ninit controls\n");
	#endif
		//this->setupControls();
		this->pca->loadControls(this->pAPDStorage);
		//SerPrintP("APD Controls - ok.\n");
		//GLCD.Puts(".");
	#ifdef DEBUG
		//setup_apd_rules();
		SerPrintP("init rules\n");
	#endif
		//this->setupRules();
		this->pra->loadRules(this->pAPDStorage);
		//SerPrintP("APD Rules - ok.\n");

		// enable "real-time" rule evaluation
		this->psa->enableRuleEvaluation(&(APDRuleArray::evaluateSensorRules),(void *)this->pra);

		// FIXME check if we are initialized, set it in bConfigured
		// TODO revise what is obligatory. for now, 1 sensor or control is enough to be considered as configured
		this->bAPDuinoConfigured =  this->pAPDStorage->ready() && (this->psa->iSensorCount > 0  || this->pca->iControlCount > 0); // && this->pra->iRuleCount > 0;
		//(this->iRuleCount > 0 && this->iSensorCount > 0 && this->iControlCount > 0 && this->pAPDStorage->ready());       // TODO

		// this was done outside before
#ifdef ENABLE_DISPLAY
		display_callback(".");
#endif

	// adding custom functions should be done outside
		//SerPrintP("APD: WWWSRV...");
		if (this->startWebServer()) {
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
			if (this->startLogging(DEFAULT_ONLINE_LOG_FREQ)) {		// TODO revise, this should be configurable
//					SerPrintP("OK.\n");
//			} else {
//					SerPrintP("FAIL.\n");
			}
		}

		delay(100);
		//pAPD->startIdling(uIdleDuration);           // used for idling device
	}
}

void APDuino::setupWithStorage(int iChip, int iSpeed) {
  delay(250);                                   // give the hw some time, we're probably just powering on
#ifdef DEBUG
  // debugging initialization
  if (this->pAPDStorage == NULL) {
  	SerPrintP("no storage yet...");
  }else{
  	SerPrintP("already have storage?");
  }
  // end debugging initialization

  if (setupStorage(SS_PIN,iChip,iSpeed) != NULL) {
  	// debugging initialization
  	SerPrintP("Seems storage was started...");
  }else {
  	SerPrintP("Seems could not start storage...");
  	// end debugging initialization
  }

  // we should now have storage
  if (this->pAPDStorage == NULL) {
  	// debugging initialization
    	SerPrintP("still no storage!");
    }else{
    	SerPrintP("have storage.");
    	// end debugging initialization
    }
#endif

  // check if storage was initialized with success
  // check we have APDWeb allocated
  if (setupStorage(SS_PIN,iChip,iSpeed) != NULL && this->pAPDStorage != NULL) {
		if (this->pAPDStorage->ready() ) {

			initApplication();

			// from outside

		} else {
			Serial.println(APDUINO_ERROR_STORAGENOTREADY);
			//SerPrintP("STORAGE NOT READY.\n");
		}
  } else {
  		Serial.println(APDUINO_ERROR_STORAGENOTSETUP);
    	//SerPrintP("STORAGE SETUP ERROR.\n");
    }
}

unsigned long APDuino::getUpTime() {
  // TODO nullptrchk
  return this->pAPDTime->getUpTime();
}

char *APDuino::getUpTimeS(char *psz_uptime) {
  // TODO nullptr chk
  return this->pAPDTime->getUpTimeS(psz_uptime);
}

boolean APDuino::storage_ready() {
	return (this->pAPDStorage!=NULL ? this->pAPDStorage->ready() : false);
}


void APDuino::setupTimeKeeping() {
#ifdef DEBUG
  SerPrintP("Time...");
#endif
  if (this->pAPDTime == NULL) {
      this->pAPDTime = new APDTime(true);       // try with RTC
  }
//#ifdef DEBUG
  else {
  	Serial.println(APDUINO_WARNING_TIMEALREADYSETUP);
      //SerPrintP("already done.\n");
  }
//#endif
}

void APDuino::checkTimeKeeping() {
#ifdef DEBUG
  SerPrintP("Time check...");
#endif
  if (this->pAPDTime != NULL) {
      // TODO add NTP switch
      pAPDTime->ntpSync();
      SerPrintP("check@:"); Serial.print((unsigned long)this->pAPDTime,DEC);
      char tbuf[20] = "1970/01/01 00:00:00";
      SerPrintP("now: "); Serial.print(this->pAPDTime->nowS(tbuf)); SerPrintP("...");
  } else {
  	Serial.println(APDUINO_ERROR_NOTIMEOBJECT);
      //SerPrintP("Nothing to check?");
  }
}

DateTime APDuino::timeNow() {
  return pAPDTime->now();
}



void APDuino::Print(char *string){
 if (pAPDSerial != NULL) {
   pAPDSerial->print(string);
 }
}
void APDuino::PrintP(void *Pstring) {
  if (pAPDSerial != NULL) {
     pAPDSerial->printP(Pstring);
   }
}
void APDuino::Debug(char *string, int iMsgLevel) {
  if (pAPDSerial != NULL && iMsgLevel <= iDebugLevel) {
     pAPDSerial->printP(string);
   }
}
void APDuino::DebugP(void *Pstring, int iMsgLevel) {
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
    	Serial.println(APDUINO_MSG_ENABLERULEPROC);		// TODO replace with future message handler
    	//SerPrintP("Enabling Rule Processing...\n");
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
    	default:
    		Serial.println(APDUINO_WARN_UNKNOWNREQUEST);
    		//SerPrintP("W");														// WARNING unknown request, ignoring
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
  	this->pra->loopRules();
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




APDStorage *APDuino::setupStorage(int iSS, int iChip, int iSpeed) {
  if (this->pAPDStorage == NULL) {
      this->pAPDStorage = new APDStorage(iSS,iChip,iSpeed);
      if (this->pAPDStorage != NULL) {
#ifdef DEBUG
      	SerPrintP("ATTEMPTING TO START STORAGE...\n");
#endif
          pAPDStorage->start();
          SerPrintP("Storage ");
          if (pAPDStorage->ready()) {
              ;
          } else {
              SerPrintP("Not ");
          }
          SerPrintP("Ready.\n");
      } else {
      	Serial.println(APDUINO_ERROR_STORAGEALLOC);
          //SerPrintP("ERR: S02\n");		//S02 - Storage allocation error
      }
  } else {
  	Serial.println(APDUINO_ERROR_STORAGEALLOCALREADY);
      //SerPrintP("ERR: S01.\n");				// S01 - Storage already allocated
  }
  return this->pAPDStorage;
}

//boolean APDuino::startWebLogging(unsigned long ulWebLoggingFreq) {
//  boolean bLogging = false;
//  if (bAPDuinoConfigured && pAPDWeb != NULL && pAPDWeb->bEthConfigured ) {    // check registration status
//      bLogging = pAPDWeb->startWebLogging(ulWebLoggingFreq);
//  }
//  return bLogging;
//}

boolean APDuino::startLogging(unsigned long ulLoggingFreq) {
  boolean bLogging = false;
  if (bAPDuinoConfigured && pAPDStorage != NULL && pAPDStorage->ready() ) {    // check storage status
      if (pAPDStorage->logrotate() >= 0) {
      	Serial.println(APDUINO_MSG_SDLOGOK);
          //SerPrintP("Log2SD ok.\n");
      } else {
      	Serial.println(APDUINO_ERROR_LOGUNKNOWN);
          //SerPrintP("ERR: L01\n");			// L01 - Unknown error related to SD logrotate
      }
      if (this->pLoggingMetro == NULL) {
         this->pLoggingMetro = new Metro(ulLoggingFreq,true);           // ignore missed events
      } else {
#ifdef DEBUG
          SerPrintP("Reschedule logging...")
#endif
          this->pLoggingMetro->interval(ulLoggingFreq);
          this->pLoggingMetro->reset();
      }
      bLogging = (this->pLoggingMetro != NULL);
  }
  return bLogging;
}

void APDuino::startIdling(unsigned long uIdleDuration) {
  if (this->pIdleMetro == NULL) {
      this->pIdleMetro = new Metro(uIdleDuration,true);                // used for idling device
  } else {
      this->pIdleMetro->interval(uIdleDuration);
  }
}



boolean APDuino::bConfigured() {
  return bAPDuinoConfigured;
}
//FIXME direct references to controls
int APDuino::AddCustomFunction(int iPos, void (*pcf)()){
#ifdef DEBUG
  SerPrintP("SET CUSTFUNC @IDX "); Serial.print(iPos);
#endif
  if (this->pcustfuncs[iPos] == NULL) {
#ifdef DEBUG
      SerPrintP("NULL, SET: "); Serial.print((unsigned int)pcf,DEC);
#endif
      this->pcustfuncs[iPos] = pcf;
      // loop though controls, see if any of them is referring to the given index
      for (int i=0; i<this->pca->iControlCount; i++) {
        APDControl *pc = this->pca->pAPDControls[i];
        if (pc != NULL && pc->config.control_type == SOFTWARE_CONTROL ) {
#ifdef DEBUG
            SerPrintP("SW CTRL @ IDX");Serial.print(i);SerPrintP(" - ");
#endif
          if (pc->config.control_pin == iPos) {
            if (this->pcustfuncs[pc->config.control_pin] != NULL) {
              // assign the custom function pointer to the control; take control's PIN as IDX
#ifdef DEBUG
              SerPrintP("SET CUSTFUNC IDX "); Serial.print(pc->config.control_pin);
              SerPrintP(" CTRLIDX"); Serial.print(i); SerPrintP(".\n");
#endif
              pc->pcustfunc = this->pcustfuncs[pc->config.control_pin];      // cvalue must hold the cfunc idx
            } else {
            	Serial.println(APDUINO_WARN_NOCUSTFUNCATIDX);
#ifdef DEBUG
                SerPrintP("NO CUSTFUNCPTR @ SPEC IDX!\n");
#endif
            }
          } else {
          	Serial.println(APDUINO_WARN_CUSTFUNCMISMATCH);
#ifdef DEBUG
              SerPrintP("CTRL NOT REFERRING TO THIS CUSTFUNC, SKIP.\n")
#endif
          }
        }
      }
      return iPos;
  }
  return -1;
}


boolean APDuino::toggleRuleProcessing() {
  bProcessRules = (this->pra->pAPDRules != NULL ? !bProcessRules : false);
  return bProcessRules;
}

boolean APDuino::enableRuleProcessing() {
  bProcessRules = true;
  return bProcessRules;
}

boolean APDuino::disableRuleProcessing() {
  bProcessRules = false;
  return bProcessRules;
}

boolean APDuino::reconfigure() {
	boolean retcode = false;
#ifdef DEBUG
  SerPrintP("\nRECONREQ...\n");
#endif
  delay(100);

  // put APDWeb in maintenance mode to PREVENT ACCESS TO sensors, controls, rules
  if (this->pAPDWeb->pause_service()) {
  	boolean bProcRulesOld = bProcessRules;			// store old rule processing state
    bProcessRules = false;
    unsigned long ulRam = freeMemory();

#ifdef DEBUG
  	SerPrintP("Reconfiguring Arrays!\n");
  	delay(10);
  	Serial.print( ulRam, DEC); SerPrintP(" RAM free.\n");
  	delay(100);
#endif

		this->iNextSensor = -1;				// invalidate next sensor index

		delete(this->pra);
#ifdef DEBUG
		SerPrintP("Deleted Rule Array.\n");	Serial.print( freeMemory(), DEC); SerPrintP(" RAM free.\n");	delay(100);
#endif
		delete(this->pca);
#ifdef DEBUG
		SerPrintP("Deleted Control Array.\n");	Serial.print( freeMemory(), DEC); SerPrintP(" RAM free.\n");	delay(100);
#endif
		delete(this->psa);
#ifdef DEBUG
		SerPrintP("Deleted Sensor Array.\n");	Serial.print( freeMemory(), DEC); SerPrintP(" RAM free.\n");	delay(100);
  	SerPrintP("Deleted Arrays!\n");
  	Serial.print( freeMemory(), DEC); SerPrintP(" RAM free.\n");
  	delay(100);
#endif

		psa = new APDSensorArray();
		pca = new APDControlArray(&pcustfuncs);
		pra = new APDRuleArray(psa,pca,&(this->bfIdle));
#ifdef DEBUG
  	SerPrintP("Reallocated Arrays!\n"); delay(10);
  	Serial.print( freeMemory(), DEC); SerPrintP(" RAM free.\n");
  	delay(10);

  	SerPrintP("\ninit sensors\n"); delay(10);
#endif
		this->psa->loadSensors(this->pAPDStorage);
		//SerPrintP("APD Sensors - ok.\n");
		//GLCD.Puts(".");

#ifdef DEBUG
		SerPrintP("\ninit controls\n"); delay(10);
#endif
		this->pca->loadControls(this->pAPDStorage);
		//SerPrintP("APD Controls - ok.\n");
		//GLCD.Puts(".");

#ifdef DEBUG
			SerPrintP("init rules\n"); delay(10);
#endif
		this->pra->loadRules(this->pAPDStorage);
		//SerPrintP("APD Rules - ok.\n");

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
		this->psa->enableRuleEvaluation(&(APDRuleArray::evaluateSensorRules),(void *)this->pra);

		// FIXME check if we are initialized, set it in bConfigured
		// TODO revise what is obligatory. for now, 1 sensor or control is enough to be considered as configured
		this->bAPDuinoConfigured =  this->pAPDStorage->ready() && (this->psa->iSensorCount > 0  || this->pca->iControlCount > 0); // && this->pra->iRuleCount > 0;

		this->bFirstLoopDone = false;									// we have not yet looped with the new config (no sensor values)
		bProcessRules = bProcRulesOld;								// restore old rule processing state
		retcode = this->pAPDWeb->continue_service();	// return if web server continues processing
  } else {
  	Serial.println(APDUINO_ERROR_COULDNOTPAUSEWWW);
  	//SerPrintP("E");					// ERROR could not pause web service
  }

  return retcode;
}

void APDuino::new_ethconf_parser(void *pAPD, int iline, char *psz) {
  NETCONF nc;
#ifdef DEBUG
  Serial.print("NETCONF READ: "); Serial.print(psz);
#endif
  int ab;
  //            mac6              |IP4         |MASK4       |GW4         |UDPPort,WWWPort
  //            DE AF DA DB AD ED ,c0 a8 01 ea, ff ff ff 00 ,c0 a8 01 fe ,8888,80
  //char myconf[]="DEAFDADBADED c0a801ea ffffff00 c0a801fe 8888 80";
  int ips = sscanf( psz, "%2x%2x%2x%2x%2x%2x %2x%2x%2x%2x %2x%2x%2x%2x %2x%2x%2x%2x %2x%2x%2x%2x %d %d",
  //int ips = sscanf( psz, "%2x%2x%2x%2x%2x%2x,%2x%2x%2x%2x,%2x%2x%2x%2x,%2x%2x%2x%2x,%d,%d",
  //int ips = sscanf( psz, "%2x%2x%2x%2x%2x%2x %d.%d.%d.%d %d.%d.%d.%d %d.%d.%d.%d %d %d",
      &(nc.mac[0]),&(nc.mac[1]),&(nc.mac[2]),&(nc.mac[3]),&(nc.mac[4]),&(nc.mac[5]),
      &(nc.ip[0]),&(nc.ip[1]),&(nc.ip[2]),&(nc.ip[3]),
      &(ab),&(nc.subnet[1]),&(nc.subnet[2]),&(nc.subnet[3]),
      &(nc.gateway[0]),&(nc.gateway[1]),&(nc.gateway[2]),&(nc.gateway[3]),
      &(nc.pridns[0]),&(nc.pridns[1]),&(nc.pridns[2]),&(nc.pridns[3]),
      &(nc.localPort),
      &(nc.wwwPort));
  (nc.subnet[0]) = ab;
#ifdef DEBUG
  SerPrintP("Parsed config:\n");
  // print configuration
  SerPrintP("IP: ");
	SerDumpIP(nc.ip);
	SerPrintP(" MASK: ");
	SerDumpIP(nc.subnet);
	SerPrintP(" DNS: ");
	SerDumpIP(nc.pridns);
	SerPrintP(" GW: ");
	SerDumpIP(nc.gateway);
	SerPrintP(" UDP: ");
	Serial.println(nc.localPort);
	SerPrintP(" WWW: ");
	Serial.println(nc.wwwPort);
#endif
  if (ips < 24) {
  	Serial.println(APDUINO_ERROR_BADNETCONFIG);
      //SerPrintP("\nBad config. Please reprovision from APDuino Online.\n");
  }
#ifdef DEBUG
  SerPrintP("\nIP config parsed "); Serial.print(ips); SerPrintP(" entities.\n");
#endif
  //TODO add compatibility code (so other options can follow, even if not parsed)
  if (((APDuino *)pAPD)->pAPDWeb == NULL ) {
  	Serial.println(APDUINO_MSG_ETHERNETFROMCONF);
#ifdef DEBUG
      SerPrintP("Init net with loaded config... \n");
#endif
      ((APDuino *)pAPD)->pAPDWeb = new APDWeb(&nc,((APDuino*)pAPD)->pAPDTime);
  }


  //TODO check for errors and use an internal (class) index to keep track of the next rule to be populated

  // now do something with the values parsed...
}



void APDuino::setupNetworking() {
	Serial.println(APDUINO_MSG_NETINIT);
  //SerPrintP("Net init...");
  delay(250);                   // probably just starting up, adding a little delay, 1/4s
  if (pAPDWeb == NULL) {      // replace with check if IP config is present
#ifdef DEBUG
    SerPrintP("trying to load config...");
#endif
    if (pAPDStorage != NULL) {
        if (pAPDStorage->readFileWithParser("ETHERNET.CFG",&new_ethconf_parser,(void*)this) > 0) {;
#ifdef DEBUG
          SerPrintP("done.\n");
#endif
        } else {
        	Serial.println(APDUINO_ERROR_BADNETCONFIG);
            //SerPrintP("No/Bad netconfig.\n");
        }
    } else {
    	Serial.println(APDUINO_ERROR_SUSPECTSTORAGEERR);
       //SerPrintP("STORAGE ERR?\n");
    }
    if (pAPDWeb == NULL) {
    		Serial.println(APDUINO_MSG_DHCPFALLBACK);
        //SerPrintP("DHCP fallback...\n");
        pAPDWeb = new APDWeb(this->pAPDTime);
    }
  } else {
  	Serial.println(APDUINO_ERROR_NETALREADYSTARTED);
      //SerPrintP("Net already started?\n");
  }
}

boolean APDuino::startWebServer() {
  boolean retcode = false;
#ifdef DEBUG
  SerPrintP("WWWS...");
#endif
  if (pAPDWeb != NULL) {
#ifdef DEBUG
      SerPrintP("starting ...");
#endif
      pAPDWeb->startWebServer(this->psa->pAPDSensors,this->psa->iSensorCount,this->pca->pAPDControls,this->pca->iControlCount,this->pra->pAPDRules,this->pra->iRuleCount,pAPDStorage);
      retcode = (pAPDWeb != NULL && pAPDWeb->pwwwserver != NULL && pAPDWeb->pwwwclient !=NULL);
  } else {
  	Serial.println(APDUINO_ERROR_NONETFORWWW);
      //SerPrintP("NONET\n");
  }
  return retcode;
}

//boolean APDuino::setupAPDuinoOnline(char *hostname, IPAddress *phostip, int port) {
//#ifdef DEBUG
//  SerPrintP("relaying apduino online setup\n");
//#endif
//  if (pAPDWeb != NULL && pAPDWeb->bEthConfigured) {
//      for (int i=0; i<4;i++)     {Serial.print((*phostip)[i], DEC); SerPrintP("."); }
//    pAPDWeb->setupAPDuinoOnline(hostname,phostip,port);
//  } else {
//      return false;
//  }
//  return true;
//}




void APDuino::unidle_device() {
#ifdef DEBUG
  SerPrintP("WAKE UP!\n");
#endif
  bfIdle = false;
  if (pIdleMetro != NULL) pIdleMetro->reset();    // UI is not idle
}

void APDuino::idle_device() {
  bfIdle = true;
  if (pIdleMetro != NULL) pIdleMetro->reset();
}



void APDuino::log_data() {
  char logString[128]="";
  char *plog = logString;
  char dataString[16]="";                // make a string for assembling the data to log:
  if (!pAPDStorage->p_sd->exists("APDLOG.TXT")) {        // if new file
    // FIXME reimplement log_header();                          // start with column names
  }
#ifdef DEBUG
  SerPrintP("Assembling log...");
#endif
  char ts[] = "1970/01/01 00:00:00";        // string used for timestamp
  if (this->pAPDTime != NULL) {
    this->pAPDTime->nowS(ts);
  } else {
    this->getUpTimeS(ts);
  }
  strcpy(plog,ts);
  plog += strlen(ts);

  // TODO should check if not printing out of the string!
  for (int i=0;i<this->psa->iSensorCount;i++) {
    if (this->psa->pAPDSensors[i]->config.sensor_log) {                    // if to be logged
        *plog = ',';
        plog++;
        strcpy(plog,this->psa->valueS(i,dataString));
        plog += strlen(dataString);
    }
  }
  *plog = 0;  // terminating 0
#ifdef DEBUG
  Serial.print(logString);
  SerPrintP("--------------------\n");
#endif
  pAPDStorage->write_log_line(logString);
}
