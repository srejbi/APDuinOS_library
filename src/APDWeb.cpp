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
 * APDWeb.cpp
 *
 *  Created on: Mar 28, 2012
 *      Author: George Schreiber
 */

#include "APDWeb.h"

#define WCPrintP(pc,s) myCPrintP(pc,(PSTR(s)));

// initilize without config (DHCP)
APDWeb::APDWeb()
{
	initBlank();
	if (start()) {
		APDDebugLog::log(APDUINO_MSG_ETHSTARTED,NULL);
	} else {	// Failed to configure Ethernet.
	  				// Fix DHCP on your LAN or provide a valid static config on SD and reset.
		APDDebugLog::log(APDUINO_ERROR_ETHCONF,NULL);
	}
}

APDWeb::~APDWeb()
{
	// todo should close any open connections first (?) - for now we don't care
	free(pwwwclient);
	pwwwclient = NULL;
	free(pwwwserver);
	pwwwserver = NULL;
}

// initialize with a network configuration
// (may be static or DHCP (set pnc->ip='0.0.0.0'))
APDWeb::APDWeb(NETCONF *pnc)
{
	initBlank();
	memcpy(&net,pnc,sizeof(NETCONF));             // copy the provided config
	if (start()) {
		APDDebugLog::log(APDUINO_MSG_CONFETHSTARTED,NULL);
	} else {
		APDDebugLog::log(APDUINO_WARN_NETCONFDHCPFALLBACK,NULL);

		initBlank();			// reinit
		if (start()) {
			APDDebugLog::log(APDUINO_MSG_NETOK,NULL);
		} else {
			APDDebugLog::log(APDUINO_ERROR_DHCPFAILED,NULL);
		}
	}
}



void APDWeb::initBlank()
{
	// todo log this when enabled log levels
	// TODO generate mac randomness ?
	memcpy(&net.mac,(byte []){ 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED },6);            //TODO generate random mac, more than 1 APDuino can have collisions
	memset(&net.ip,0,4*sizeof(byte));                          // uninitialized address
	memset(&net.gateway,0,4*sizeof(byte));                     // uninitialized address
	memset(&net.subnet,0,4*sizeof(byte));                      // uninitialized address
	memset(&net.pridns,0,4*sizeof(byte));                      // uninitialized address

	memcpy(&apduino_server_ip,APDUINO_SERVER_IP,4*sizeof(byte));  // set APDuino Online IP
	strcpy_P(apduino_server_name,APDUINO_SERVER);                 // APDuino Online hostname
	apduino_server_port = 80;                 										// standard HTTP port
	apduino_logging_freq = DEFAULT_ONLINE_LOG_FREQ;

	memcpy(&cosm_server_ip,COSM_SERVER_IP,4*sizeof(byte));           // set cosm ip
	strcpy_P(cosm_server_name, COSM_SERVER );												// set cosm hostname
	cosm_server_port = 80;     // standard HTTP port
	cosm_feed_id = 0;         // TODO reset feed id
	cosm_logging_freq = DEFAULT_ONLINE_LOG_FREQ;    // 1 min

	memcpy(&thingspeak_server_ip,(byte[]){184, 106, 153, 149},4*sizeof(byte)); // api.thingspeak.com  184.106.153.149
	strcpy_P(thingspeak_server_name, PSTR("api.thingspeak.com") );
	thingspeak_server_port = 80;     // standard HTTP port
	thingspeak_logging_freq = DEFAULT_ONLINE_LOG_FREQ;    // 1 min

	net.localPort = 8888;                         // local port to listen for UDP packets (NTP communications)
	net.wwwPort = 80;
	bEthConfigured = false;
	pwwwserver = NULL;
	pwwwclient = NULL;
	uCCount = 0;
	pwwwcp = NULL;           // pointer to the actual web client processor (depending on what request was made, a reader can be assigned to process server resp. if we care)
	bWebClient = false;
	/*pAPDSensors = NULL;
	pAPDControls = NULL;
	pAPDRules = NULL;
	iSensorCount = -1;
	iControlCount = -1;
	iRuleCount = -1;*/
	this->psa = NULL;
	this->pca = NULL;
	this->pra = NULL;

	memset(szAPDUINO_API_KEY, 0, sizeof(szAPDUINO_API_KEY) );
	memset(szCOSM_API_KEY, 0, sizeof(szCOSM_API_KEY) );
	memset(szTHINGSPEAK_API_KEY, 0, sizeof(szTHINGSPEAK_API_KEY) );
	pmetro = NULL;
	phmetro = NULL;
	tsmetro = NULL;
	bDHCP = true;

	iFailureCount = 0;
	iRestartCount = 0;
	bRestart = false;
	iBusyCounter = 0;
	wcb_millis = 0;

	operational_state = 0;
	dispatched_requests = 0;
}


// start the networking service(s) as configured (or not configured)
// returns true on success, false otherwise (check logs)
boolean APDWeb::start() {
	// todo log this when enabled log levels

	//net.ip[0] = 0;
	if ((bRestart && !this->bDHCP) || (!bRestart && net.ip[0] != 0)) {                 // if an IP seems to be provided
		APDDebugLog::log(APDUINO_MSG_TRYINGSTATICIP,NULL);			// TODO print IP in string and push to debug log
		// todo log this with the net params
		Ethernet.begin(net.mac, net.ip, net.pridns, net.gateway, net.subnet);
		bEthConfigured = true;
		operational_state = OPSTATE_CONFIGURED | OPSTATE_STARTED;
		this->bDHCP = false;
		// todo read on howto check state
	} else {                              // go for DHCP
		APDDebugLog::log(APDUINO_MSG_TRYINGDHCPIP,NULL);

		// trying DHCP
		if (Ethernet.begin(net.mac) == 0) {
			APDDebugLog::log(APDUINO_ERROR_DHCPSTARTFAIL,NULL);
			operational_state = OPSTATE_BLANK | OPSTATE_ERROR;
			return false;
		}
		// we should have a lease now
		APDDebugLog::log(APDUINO_MSG_DHCPLEASED,NULL);			// TODO print IP to string and put to log message
		// todo log this when enabled log levels

		// we should have a lease now, save data
		uint32_t ipaddr = (uint32_t)Ethernet.localIP();
		memcpy((void *)(byte *)net.ip, (void *)&ipaddr, sizeof(byte)*4);
		ipaddr = (uint32_t)Ethernet.subnetMask();
		memcpy((void *)(byte *)net.subnet, (void *)&ipaddr, sizeof(byte)*4);
		ipaddr = (uint32_t)Ethernet.gatewayIP();
		memcpy((void *)(byte *)net.gateway, (void *)&ipaddr, sizeof(byte)*4);
		// TODO what about DNS? (not using right now, resolving IP's manually in config)
#if (ARDUINO >= 101)
		ipaddr = Ethernet.dnsServerIP();
		memcpy((void *)(byte *)net.pridns, (void *)&ipaddr, sizeof(byte)*4);
#endif
		// TODO log this with net.ip, net.subnet, net.gateway, net.pridns

		bEthConfigured = true;
		operational_state = OPSTATE_CONFIGURED | OPSTATE_STARTED;
	}

	iFailureCount = 0;
	iRestartCount += 1;
	bRestart = false;
	iBusyCounter = 0;
	wcb_millis = 0;

	return true;
}

// a fake restart, it is only stopping clients, not the server
// works around connection issues..
// TODO: investigate server stopping
boolean APDWeb::restart() {
	APDDebugLog::log(APDUINO_LOG_NETRESTART,NULL);
	// if running in a multithreading environment, we should lock the APDWeb with a semaphore here...
	if (bEthConfigured) {
		if (pwwwclient) {
			pwwwclient->stop();
			delay(1000);
		}

		bEthConfigured = false;
		operational_state = OPSTATE_BLANK;
		if (this->start()) {
			this->self_register();
			APDDebugLog::log(APDUINO_LOG_NETRESTARTED,NULL);
		} else {
			operational_state |= OPSTATE_ERROR;
			this->failure();
			APDDebugLog::log(APDUINO_ERROR_NETRESTARTFAIL,NULL);
		}
	} else {
		return false;
	}
	return bEthConfigured;
}

void APDWeb::failure() {
	if (++this->iFailureCount >= MAX_NET_FAILURES) {	// TODO replace 3 with a variable
		// todo log this with "->FC=" iFailureCount when enabled log levels
		this->bRestart = true;		// request a restart (will be done in loop)
		APDDebugLog::log(APDUINO_MSG_NETFAILSRESTART,NULL);
	}
}


// registers the moment a response was first expected
// subsequent calls increase busy counter (as still no response)
// upon MAX_WEBCLIENT_BUSY_LOOPS checks
// or WEBCLIENT_BUSY_TIMEOUT_MS millisecs from first check (whichever occurs first)
// it increases failure counter (that may lead to network restart)
void APDWeb::wc_busy() {
	unsigned long nowms = millis();
	if (wcb_millis == 0 || iBusyCounter == 0) {
		wcb_millis = nowms;
	}
	if (++this->iBusyCounter >= MAX_WEBCLIENT_BUSY_LOOPS  || (nowms - wcb_millis > WEBCLIENT_BUSY_TIMEOUT_MS)) {	// TODO replace 50 with a variable
		this->failure();
		this->iBusyCounter = 0;
		this->wcb_millis = 0;
	}
}

boolean APDWeb::setupAPDuinoOnline() {
	boolean retcode = false;
	// todo log this when enabled log levels
	szAPDUINO_API_KEY[0] = 0;
	if (bEthConfigured && APDStorage::ready()) {
		// todo log this when enabled log levels
		if (APDStorage::read_file_with_parser("ONLINE.CFG",&new_apduinoconf_parser,(void*)this) > 0) {
		} else {
			// use defaults
			memcpy(&apduino_server_ip,APDUINO_SERVER_IP,4*sizeof(byte));           // apduino.localhost -- test server on LAN
			strcpy_P(apduino_server_name,APDUINO_SERVER);                   // test APDuino server on LAN
			apduino_server_port = 80;
			apduino_logging_freq = DEFAULT_ONLINE_LOG_FREQ;
		}
		// todo log this with apduino_server_name apduino_server_ip when enabled log levels

		if (apduino_server_ip[0]>0) {                            // if we have a server name
			loadAPIkey(szAPDUINO_API_KEY,"APIKEY.CFG");             // load api key for apduino.com
			// todo log this when enabled log levels
			self_register();								// this will get a server-generated key

			startWebLogging(apduino_logging_freq);
			retcode = true;
		} else {
			APDDebugLog::log(APDUINO_ERROR_APDUINOONLINEIP,NULL);
		}

		retcode = true;
	} else {
		APDDebugLog::log(APDUINO_ERROR_STORAGEERRORAO,NULL);
	}
	return retcode;
}

// load api key, basically reads a line from the given file, if found
// szAPIKey - pointer to a string that receives the line (should be large enough)
// szAPIFile - path to the file
boolean APDWeb::loadAPIkey(char *szAPIKey, const char *szAPIFile) {
	if (APDStorage::ready()) {
		int i=0;
		char line[RCV_BUFSIZ]="";
		int bread=0;
		SdFile dataFile(szAPIFile, O_RDONLY );
		if (dataFile.isOpen()) {
			while (bread=dataFile.fgets(line, sizeof(line))) {      // get the next line
				if (char *nl=strstr_P(line,PSTR("\n")) ) {
					*nl = '\0';
				}
				if (strlen(line)) {
					strcpy(szAPIKey,line);
				}
				i++;
			}
		}
		return (i>0 && strlen(szAPIKey)>0);
	}
	return false;
}

// stores an API key in a keyfile (a file containing a string with API key)
// szAPIKey - character buffer with API key
// szAPIFile - path to the file
void APDWeb::saveAPIkey(const char *szAPIKey, const char *szAPIFile)
{
	if (APDStorage::ready()) {
		SdFile dataFile(szAPIFile, O_WRITE | O_CREAT);
		if (dataFile.isOpen()) {
			dataFile.println(szAPIKey);
			dataFile.close();
		} else {
			APDDebugLog::log(APDUINO_ERROR_AKSAVEIOERR,NULL);
		}
	} else {
		APDDebugLog::log(APDUINO_ERROR_AKSAVESTORAGE,NULL);
	}
}

// start periodic logging to APDuino Online
boolean APDWeb::startWebLogging(unsigned long uWWWLoggingFreq) {
	boolean retcode = false;
	if (this->pmetro == NULL) {
		this->pmetro = new Metro(uWWWLoggingFreq, true);
	} else {
		this->pmetro->interval(uWWWLoggingFreq);
	}
	this->pmetro->reset();
	retcode = (this->pmetro != NULL);
	APDDebugLog::log(APDUINO_MSG_AOLOGSTARTED, apduino_server_name); // todo include IP in log -- SerDumpIP(apduino_server_ip);
	return retcode;
}

// setup lohhomh to Cosm
boolean APDWeb::setupCosmLogging() {
	boolean retcode = false;
	szCOSM_API_KEY[0] = 0;
	if (bEthConfigured && APDStorage::ready() && this->phmetro == NULL) {
		if (APDStorage::read_file_with_parser("PACHUBE.CFG",&new_cosmconf_parser,(void*)this) > 0) {
			// todo log this when enabled log levels
			if (cosm_server_ip[0]>0) {                            // if we have a server name
				loadAPIkey(szCOSM_API_KEY,"PACHUBE.KEY");             // TODO -> load api key for ; allow multiple keys for different services
				Serial.print(szCOSM_API_KEY);
				delay(20);

			} else {
				// todo log this when enabled log levels
			}
			this->phmetro = new Metro(cosm_logging_freq, true);                     // TODO check this
			APDDebugLog::log(APDUINO_MSG_COSMLOGSTARTED, cosm_server_name); // todo include IP in log -- SerDumpIP(cosm_server_ip);
			retcode = true;

		} else {
			// todo log this when enabled log levels
		}

	} else {
		// todo log this when enabled log levels
	}
	return retcode;
}


boolean APDWeb::setupThingSpeakLogging() {
	boolean retcode = false;
	// todo log this
	szTHINGSPEAK_API_KEY[0] = 0;
	if (bEthConfigured && APDStorage::ready() && this->tsmetro == NULL) {
		if (APDStorage::read_file_with_parser("THINGSPK.CFG",&new_thingspeakconf_parser,(void*)this) > 0) {
			// todo log this w/ server name
			if (thingspeak_server_ip[0]>0) {                            // if we have a server name
				loadAPIkey(szTHINGSPEAK_API_KEY,"THINGSPK.KEY");
				// todo log this with szTHINGSPEAK_API_KEY
			} else {
				// todo log this
			}
			// todo log this
			this->tsmetro = new Metro(thingspeak_logging_freq, true);                     // TODO check this
			retcode = true;
		} else {
			// todo log this
		}
	} else {
		// todo log this
	}
	return retcode;
}


//void APDWeb::startWebServer(APDSensor **pSensors, int iSensorCount, APDControl **pControls, int iControlCount, APDRule **pRules, int iRuleCount)
void APDWeb::startWebServer(const APDSensorArray *pSA, const APDControlArray *pCA, const APDRuleArray *pRA) {
	// todo log this
	if (pwwwserver == NULL) {
		// todo log this
		pwwwserver = new EthernetServer(net.wwwPort);
		pwwwserver->begin();
		/*this->pAPDSensors = pSensors;
		this->pAPDControls = pControls;
		this->pAPDRules = pRules;
		this->iSensorCount = iSensorCount;
		this->iControlCount = iControlCount;
		this->iRuleCount = iRuleCount;*/
		this->psa = (APDSensorArray *)pSA;
		this->pca = (APDControlArray *)pCA;
		this->pra = (APDRuleArray *)pRA;

		this->setup_webclient();
	} else {
		// todo log this
	}
}

// start a html page using image, css resources on the APDuino Online server
// todo this code is being deprecated (1st minimized) in favor of the JSON data + www server solution
void APDWeb::web_startpage(EthernetClient *pClient, char *title,int refresh=0) {
	WCPrintP(pClient,"<html>\n");
	WCPrintP(pClient,"<head><title>APDUINO - "); pClient->print(title); WCPrintP(pClient,"</title>\n");
	WCPrintP(pClient,"<link rel=\"shortcut icon\" type=\"image/x-icon\" href=\"http://"); pClient->print(apduino_server_name); WCPrintP(pClient,"/images/apduino_icon.png\"/>\n");
	if (refresh) {
		WCPrintP(pClient,"<meta http-equiv=\"refresh\" content=\""); pClient->print(refresh); WCPrintP(pClient,"\"/>\n");
	}
	WCPrintP(pClient,"</head><body>\n");
}

// closes the HTML body and html tags
void APDWeb::web_endpage(EthernetClient *pClient) {
	if (*pClient) {
		WCPrintP(pClient,"</body></html>");
	} else {
		char sztmp[24]="";
		APDDebugLog::log(APDUINO_ERROR_WWWNOCLIENT,strcpy_P(sztmp,PSTR("web_endpage")));
	}
}

// generates HTML code with a table line for status
void APDWeb::webstatus_table_item(EthernetClient *pClient, const char *group, const int index, const char *name, const char *value, const char *logged ) {
/*	deprecating
	WCPrintP(pClient,"<tr class=\"line\">"
										"<td class=\""); pClient->print(group); WCPrintP(pClient,"_name\">");
	pClient->print(index); WCPrintP(pClient," : ");
	pClient->print(name);
	WCPrintP(pClient,"</td><td class=\""); pClient->print(group); WCPrintP(pClient,"_value\">");
	pClient->print(value);
	WCPrintP(pClient,"</td><td");
	if (strcmp_P(logged,PSTR("1"))) {
		WCPrintP(pClient," class=\""); pClient->print(group); WCPrintP(pClient,"-chart\" id=\"chart-"); pClient->print(name); WCPrintP(pClient,"\"");
	}
	WCPrintP(pClient,"></td></tr>");		// end row
	*/
}


// list sensors on a html page
// todo deprecate this in favor of the JSON api
void APDWeb::web_status(EthernetClient *pClient) {
	/* deprecating
	if (*pClient) {
		char tbuf[20] ="";
		WCPrintP(pClient,"<div class=\"status\">"
				"<div class=\"sensors\"><h1>Sensors</h1>"
				"<table id=\"sensors_table\">");

		//    SerPrintP("Output sensors...");
		for (int i = 0; i < this->psa->iSensorCount; i++) {
			webstatus_table_item(pClient, "sensor", i, this->psa->pAPDSensors[i]->config.label, this->psa->pAPDSensors[i]->get_value_str(tbuf), (this->psa->pAPDSensors[i]->config.sensor_log ? "1" : "0") );
		}

		WCPrintP(pClient,"</table>"
										"</div>");		// sensors div
		// CONTROLS OUTPUT
		WCPrintP(pClient,"<div class=\"controls\"><h1>Controls</h1>"
				"<table id=\"controls_table\">");

		for (int i = 0; i < this->pca->iControlCount; i++) {
			webstatus_table_item(pClient, "control", i, this->pca->pAPDControls[i]->config.label, this->pca->pAPDControls[i]->get_value_str(tbuf), (this->pca->pAPDControls[i]->config.control_log ? "1" : "0") );
		}

		WCPrintP(pClient,"</table>"
										"</div>\n");		// controls div

		//    RULES OUTPUT
		WCPrintP(pClient,"<div class=\"rules\"><h1>Rules</h1>"
				"<table id=\"rules_table\">");

		for (int i = 0; i < this->pra->iRuleCount; i++) {
			webstatus_table_item(pClient, "rule", i, this->pra->pAPDRules[i]->config.label, this->pra->pAPDRules[i]->get_value_str(tbuf), "0" );
		}

		WCPrintP(pClient,"</table>"
											"</div>\n");		// rules div

		WCPrintP(pClient,"</div>\n");		// status div
		// give the web browser time to receive the data
		delay(1);
	} else {
		char sztmp[24] = "";
		APDDebugLog::log(APDUINO_ERROR_WWWNOCLIENT,strcpy_P(sztmp,PSTR("web_status")));
	}
	*/
}


// return HTTP 503 - Service unavailable
void APDWeb::web_maintenance(EthernetClient *pClient) {
	WCPrintP(pClient, "HTTP/1.1 503 (Service unavailable)\n"
			"Content-Type: text/html\n\n"
			"<h2>Temporarily unavailable. Please try again later.</h2>\n");
}

// return HTTP 404 - Not found
void APDWeb::web_notfound(EthernetClient *pClient) {
	WCPrintP(pClient, "HTTP/1.1 404 Not Found\n"
			"Content-Type: text/html\n\n"
			"<h2>File Not Found!</h2>\n");
}

// generate a HTML snippet with <a href /> tag hyperlinking to the APDuino Online server
// allowing to associate device with APDuino Online account
void APDWeb::claim_device_link(EthernetClient *pClient) {
	if (*pClient) {
		WCPrintP(pClient,"<a href=\"http://"); pClient->print(apduino_server_name); WCPrintP(pClient,"/devices/claim_device?api_key=");
		pClient->print(szAPDUINO_API_KEY); WCPrintP(pClient,"\">Claim your APDuino!</a>");
		WCPrintP(pClient,"<hr/>");
		// give the web browser time to receive the data
		delay(1);
	} else {
		char sztmp[24]="";
		APDDebugLog::log(APDUINO_ERROR_WWWNOCLIENT,strcpy_P(sztmp,PSTR("claim_device_link")));
	}
}

// serve a file from SD card
// client - the ethernet client to write to
// szPath - path to the file
// return true if file was found and served, false on error (not found)
bool APDWeb::ServeFile(EthernetClient client, const char *szPath) {
	bool retcode = false;
	APDDebugLog::log(APDUINO_MSG_WWWFILEACCESS, szPath);
	SdFile file;
	if (file.open(APDStorage::p_root, szPath, O_READ)) {
		retcode = true;
		if ( file.isFile() ) {
			if (strstr_P(szPath, PSTR(".htm")) != 0 || strstr_P(szPath, PSTR(".html")) != 0 ) {
				header(&client,CONTENT_TYPE_HTML);
			} else if (strstr_P(szPath, PSTR(".js")) != 0) {
				header(&client,CONTENT_TYPE_JAVASCRIPT);
			} else {
				header(&client,CONTENT_TYPE_TEXT);
			}	// todo add more content types with an extension-CT mapping file (provisioned)

			int16_t c;
			while ((c = file.read()) > -1) {
				// uncomment the serial to debug (slow!)
				//Serial.print((char)c);
				client.print((char)c);
			}
		} else if (file.isDir()) {
			// send a standard http response header (todo handle json later)
			header(&client,CONTENT_TYPE_HTML);
			// print all the files, use a helper to keep it 'clean'
			web_startpage(&client,"files");
			ListFiles(client, szPath, LS_DATE | LS_SIZE);		// list path
			web_endpage(&client);
		}
		file.close();
	} else {
		// let the caller handle 404 //web_notfound(&client);
	}
	return retcode;
}

// generate html list of a directory
// client - Ethernet Client to print to
// szPath - the directory to list
// flags - listing flags
// todo implement listing as json
// originally used some SdFat example here featured by a webserver example
void APDWeb::ListFiles(EthernetClient client, const char *szPath, uint8_t flags) {
	SdFile *proot = APDStorage::p_root;
	dir_t p;
	if (APDStorage::ready() && APDStorage::p_root ) {
		proot->rewind();
		if (szPath != NULL && szPath[0] != 0) {
			proot = new SdFile(szPath, O_RDONLY);
		}
		WCPrintP(&client,"<div class=\"files\">\n<ul>\n");
		if (proot != APDStorage::p_root) {		// link to upper level if not at root
			WCPrintP(&client,"<li><a href=\"../\">..</a></li>\n");
		}
		while (proot->readDir(&p) > 0 && p.name[0] != DIR_NAME_FREE) {
			// done if past last used entry
			//if (p.name[0] == DIR_NAME_FREE) break;
			// skip deleted entry and entries for '.'
			if (p.name[0] == DIR_NAME_DELETED || (p.name[0] == '.' && p.name[1] != '.')) continue;
			//
			if (DIR_IS_FILE_OR_SUBDIR(&p)) {
				// print any indent spaces
				WCPrintP(&client,"<li><a href=\"");
				for (uint8_t i = 0; i < 11; i++) {
					if (p.name[i] == ' ') continue;
					if (i == 8) {
						WCPrintP(&client,".");
					}
					client.print((char)p.name[i]);
#ifdef DEBUG
					Serial.print((char)p.name[i]);
#endif
				}
				if (DIR_IS_SUBDIR(&p)) {
					WCPrintP(&client,"/");
				}
#ifdef DEBUG
				Serial.println("");
#endif
				WCPrintP(&client,"\">");

				// print file name with possible blank fill
				for (uint8_t i = 0; i < 11; i++) {
					if (p.name[i] == ' ') continue;
					if (i == 8) {
						WCPrintP(&client,".");
					}
					client.print((char)p.name[i]);
				}

				WCPrintP(&client,"</a>");

				if (DIR_IS_SUBDIR(&p)) {
					WCPrintP(&client,"/");
				}

				// print modify date/time if requested
				if (flags & LS_DATE) {
					char sztmp[32]="";
				  sprintf_P(sztmp,PSTR("%02d-%02d-%02d %02d:%02d:%02d"),
				  									FAT_YEAR(p.lastWriteDate), FAT_MONTH(p.lastWriteDate), FAT_DAY(p.lastWriteDate),
				  	  							FAT_HOUR(p.lastWriteTime), FAT_MINUTE(p.lastWriteTime), FAT_SECOND(p.lastWriteTime));
					client.print(sztmp);
				}
				// print size if requested
				if (!DIR_IS_SUBDIR(&p) && (flags & LS_SIZE)) {
					WCPrintP(&client," ");
					client.print(p.fileSize);
				}
				WCPrintP(&client,"</li>\n");
			}
		}
		WCPrintP(&client,"</ul>\n</div>\n");
	} else {
		APDDebugLog::log(APDUINO_ERROR_WWWFSNOSTORAGE,NULL);
		//todo redirect this error also to the www client
	}
	if (proot != APDStorage::p_root) {
		free(proot);
	}
}

// loop the server, process incoming requests
// APDuino calls this reasonably often to have a smooth web service
void APDWeb::loop_server()
{
	if (pwwwserver != NULL) {      // if server is instantiated
		char clientline[RCV_BUFSIZ+1];
		clientline[RCV_BUFSIZ-1]=0;      // a terminating 0 at the last position
		int index = 0;
		char *pc = clientline;		// pc is a temporary char pointer that may be used  \
																	for finding places in the actual (execution point) \
																	do not use it across the routine! do not write to the address pointed

		EthernetClient client = pwwwserver->available();
		if (client) {
			uCCount++;                              // one more web client
			boolean current_line_is_blank = true;      // an http request ends with a blank line
			index = 0;                              // reset the input buffer
			while (client.connected()) {            // as long as the client is connected
				if (!(this->operational_state & OPSTATE_PAUSED)) {
					if (client.available()) {             // if there are bytes to read
						char c = client.read();                // read 1 character

						if (c != '\n' && c != '\r') {         // If it isn't a new line, add the character to the buffer
							clientline[index] = c;
							index++;

							// are we too big for the buffer? shift left
							if (index >= RCV_BUFSIZ-1) {
								// low ram footprint chosen for shifting (vs. high footprint memcpy)
								for (int i=0; i < RCV_BUFSIZ-1; i++) clientline[i] = clientline[i+1];
								index = RCV_BUFSIZ -2;
							}
							//if (index >= RCV_BUFSIZ)           // are we too big for the buffer? start tossing out data
							//	index = RCV_BUFSIZ -1;							// todo this is bs, SHIFT char array is the proper solution (both result data loss, but this even corrupts the data integrity)
							continue;           							 // continue to read more data!
						}
						// got a \n or \r new line, which means the string is done
						clientline[index] = 0;

						// Look for substring such as a request to get the root file
						if (strstr_P(clientline, PSTR("GET /status.json")) != 0) {
							if (basicAuthorize(&client)) {
								header(&client,CONTENT_TYPE_JSON);
								json_status(&client);
							}
						} else if (strstr_P(clientline, PSTR("POST /controls/")) != 0) {
							procControlReq(&client,clientline);		// shoult take care of all, including auth.
						} else if (strstr_P(clientline, PSTR("POST /sensors/")) != 0) {
							procSensorReq(&client,clientline);		// shoult take care of all, including auth.
						} else if ((pc = strstr_P(clientline, PSTR("POST /systems/"))) != 0) {
							procSystemReq(&client,clientline);		// -- " " --
						} else if (strstr_P(clientline, PSTR("GET /sd/ ")) != 0) { // print all the files, use a helper to keep it clean
							if (basicAuthorize(&client)) {
								header(&client,CONTENT_TYPE_HTML);	// send a standard http response header for html page
								web_startpage(&client,"files");
								ListFiles(client, NULL, LS_DATE | LS_SIZE);		// list root
								web_endpage(&client);
							}
						} else if (strstr_P(clientline, PSTR("GET /sd/")) != 0) {		// no space after the /, so a filename is expected to follow
							// retrieve the filename before authentication (as that would throw our buffer away)
							char *pfile_path;		// will point to file path
							pfile_path = clientline + 8; // pointer to string following "GET /sd/"
							(strstr_P(clientline, PSTR(" HTTP")))[0] = 0;       // terminate string just before proto string
							char file_path[128] = "";		// a temp. buffer hopefully large enough for any files (overkill for the current dir. layout)
							file_path[127] = 0;					// make sure the string will be null terminated
							strncpy(file_path,pfile_path,127);	// strncpy would not null terminate if reaching max. (but null pads if < max)
							// todo log file access ATTEMPT
							if (basicAuthorize(&client)) {		// now (that we have the file) Authenticate
								// todo log this
								if (!ServeFile(client,file_path)) web_notfound(&client);		// server file or 404 if serving returns false
							}
						} else if (strstr_P(clientline, PSTR("GET /status ")) != 0 ||		// /status
												strstr_P(clientline, PSTR("GET / ")) != 0) {				// also for www root
							if (basicAuthorize(&client)) {
								if (!ServeFile(client,"/index.htm")) {
										// todo log this when enabled log levels
										header(&client,CONTENT_TYPE_HTML);
										web_startpage(&client,"status",20);
										WCPrintP(&client,"Get an UI from APDuino Online.");
										web_endpage(&client);
										// todo log this when enabled log levels
								}
							}
						} else if (strstr_P(clientline,PSTR("GET /reconfigure")) != 0) {
							if (basicAuthorize(&client)) {
								header(&client,CONTENT_TYPE_HTML);
								// deprecating fancy responses, just an ok status
								//web_startpage(&client,"reconfigure",0);
								//WCPrintP(&client,"OK.");		// TODO
								//web_endpage(&client);
								this->dispatched_requests = DREQ_RECONF;		// APDuino should read it
							}
						} else if (strstr_P(clientline,PSTR("GET /reset")) != 0) {
							if (basicAuthorize(&client)) {
								header(&client,CONTENT_TYPE_HTML);
								// deprecating fancy responses, just an ok status
								//web_startpage(&client,"reset",0);
								//WCPrintP(&client,"OK.");		// TODO return all stuff as JSON + codes ref. apd_msg_codes.h - javascript can easily make the conversion
								//web_endpage(&client);
								this->dispatched_requests = DREQ_RESET;		// APDuino should read it
							}
						}  else if (strstr_P(clientline,PSTR("GET /reloadrules")) != 0) {
							if (basicAuthorize(&client)) {
								header(&client,CONTENT_TYPE_HTML);
								// deprecating fancy responses, just an ok status
								//web_startpage(&client,"reload_rules",0);
								//WCPrintP(&client,"OK.");
								//web_endpage(&client);
								this->dispatched_requests = DREQ_RELOADRULES;		// APDuino should read it
							}
						} else if (strstr_P(clientline,PSTR("GET /diags")) != 0) {
							if (basicAuthorize(&client)) {
								header(&client,CONTENT_TYPE_HTML);
								// deprecating fancy responses, just an ok status
								//web_startpage(&client,"diagnostics",0);
								//WCPrintP(&client,"OK.");
								//web_endpage(&client);
								this->dispatched_requests = DREQ_DIAGNOSTICS;		// APDuino should read it
							}
						} else if (strstr_P(clientline,PSTR("GET /claimdevice")) != 0) {
							if (basicAuthorize(&client)) {
								header(&client,CONTENT_TYPE_HTML);
								web_startpage(&client,"claimdevice",0);
								claim_device_link(&client);
								web_endpage(&client);
							}
					// TODO investigate/research why the Reset_AVR messes up the device
					/* } else if (strstr_P(filename,PSTR("reset")) != 0) {
							if (basicAuthorize(&client)) {
								// todo log this
								Reset_AVR();
							}*/
						} else if (strstr_P(clientline, PSTR("POST /provisioning")) != 0) {
							if (basicAuthorize(&client)) {
								if (!(this->operational_state & OPSTATE_PAUSED)) {
									this->processProvisioningRequest(&client, true);
								} else {
									web_maintenance(&client);
								}
							}
						} else {
							// todo log this when enabled log levels
							// everything else is a 404
							if (basicAuthorize(&client)) {
								if (strstr_P(clientline, PSTR("GET / ")) == 0) {		// if not root url
									web_notfound(&client);
								} else {
									// 'GET / ' served already
								}
							}	// authorized
						}
						break;
					}
				} else {			// OPSTATE_PAUSED (not 'likely' in a single threaded env. , eh...:))
					web_maintenance(&client);
				}
			}

			delay(1);					// give the web browser time to receive the data
			client.stop();		// disconnect the client

		}
	}
}

// attempts to locate Basic Authorization HTTP header, if found, checks if base64 encoded value
// matches with the one stored in LOCAL.KEY (if not found than base64 encoded "admin:admin")
// returns true if authorized, false otherwise
// Note: when unauthorized, it will call sendAuthRequest to send the request
// so the caller can just close the connection
// if true, the caller can proceed serving the protected content
// todo multiuser, multirealm, etc.
boolean APDWeb::basicAuthorize(EthernetClient *pclient) {
	boolean bretcode = false;		// unauth
	char sztmp[RCV_BUFSIZ] = "";
	char szmark[24] = "";
	forwardToMarker(pclient,sztmp,strcpy_P(szmark, PSTR("Authorization: Basic ")));
	if (strstr(sztmp, szmark)) {
		sztmp[0] = 0;
		int index = 0;
		while (pclient->available() && !bretcode) {    // if there are bytes to read and still not authorized
			char c = pclient->read();                // read 1 character
			if (c != '\n' && c != '\r') {         // If it isn't a new line, add the character to the buffer
				sztmp[index] = c;
				index++;

				if (index >= RCV_BUFSIZ)           // are we too big for the buffer? start tossing out data (messes up the decoding of u/p
					index = RCV_BUFSIZ -1;
				continue;           							 // continue to read more data!
			}
			sztmp[index] = 0;			// terminating string at index
			char szencstr[128] = "";
			if (!loadAPIkey(szencstr,"LOCAL.KEY")) {
				// todo remove deprecated code block
				// deprecated implementation with Base64: base64 encode u/p and compare
				// for first implementation will go with admin:admin
				//char szliteral[] = "";
				//strcpy_P(szliteral,PSTR("admin:admin"));
				//int encoded = base64_encode(szencstr,szliteral,11);

				// same thing using pre-encoded default pw (smaller footprint)
				strcpy_P(szencstr,PSTR("YWRtaW46YWRtaW4="));	// "admin:admin"
			}
			// todo log access when enabled log levels
			//APDDebugLog::log(APDUINO_MSG_WWWAUTHOK,NULL);
			if (strcmp(sztmp,szencstr)==0) {
				bretcode = true;
			} else break;
		}
		if (!bretcode) {
			APDDebugLog::log(APDUINO_ERROR_WWWAUTHFAIL,sztmp);
			sendAuthRequest(pclient,"apduino");
		}
	} else {
		Serial.print(sztmp);
		// Unauthorized, send auth req.
		APDDebugLog::log(APDUINO_ERROR_WWWUNAUTH,NULL);
		sendAuthRequest(pclient,"apduino");
	}
	return bretcode;
}

void APDWeb::sendAuthRequest(EthernetClient *pclient, const char *szrealm) {
	WCPrintP(pclient,"HTTP/1.1 401 Not Authorized\n"
				"WWW-Authenticate: Basic realm=\"");
	pclient->print(szrealm);
	WCPrintP(pclient,"\"\n");
	WCPrintP(pclient,"\n\n");
	delay(1);
}

void APDWeb::procControlReq(EthernetClient *pclient, char *clientline) {
	bool ok = false;
	char *pc = strstr_P(clientline, PSTR("POST /controls/"));

	pc+=15;				      //skip to post 'POST /control/'
	int ic = atoi(pc);	// get the control id -> from pc
	APDDebugLog::log(APDUINO_LOG_WWWCONTROLACCESS,clientline);
	if (basicAuthorize(pclient)) {
		// return OK if done, // resource not available if invalid,// busy if in automatic mode
		//header(&client,CONTENT_TYPE_JSON);
		if (ic >= 0 && ic <= this->pca->iControlCount) {
			//	pc += 6; // skip "value="
			if (pc = forwardToMarker(pclient,clientline,"value=")) {
				int iv = NAN;
				if (pc = forwardToMarker(pclient,clientline,"&")) {
					*pc = 0; // terminate string at '&'
				}
				iv = atoi(clientline);			// now we should have an integer /no error handling/
				if (iv != NAN && (!this->pra->bProcRules ||											// if rule proc inactive,
					(forwardToMarker(pclient,clientline,"force=")&&pclient->read()=='1'))) {// or force=1 flag given
					// we can write the control
					ok = true;
					APDControl::apd_action_set_value(this->pca->find_control_by_index(ic), iv);

					// log action
					char sztmp[16]="";
					sprintf_P(sztmp,PSTR("%d,%d"),ic,iv);
					APDDebugLog::log(APDUINO_LOG_WWWCONTROLSET,sztmp);
					// done
				} else {
					// should not write, todo log with levels enabled
				}
			}
		}
		if (ok) {
			header(pclient,CONTENT_TYPE_HTML);
			json_status(pclient);
		} else {
			web_notfound(pclient);
		}
	} // authorized
}


void APDWeb::procSensorReq(EthernetClient *pclient, char *clientline) {
	bool ok = false;
	char *pc = strstr_P(clientline, PSTR("POST /sensors/"));
	pc+=14;				      	//skip to post 'POST /sensors/'
	int ic = atoi(pc);		// get the control id -> from pc
	APDDebugLog::log(APDUINO_LOG_WWWSENSORACCESS,clientline);
	if (basicAuthorize(pclient)) {
		// return OK if done, // resource not available if invalid,// busy if in automatic mode
		if (ic >= 0 && ic <= this->psa->iSensorCount) {
			APDDebugLog::log(APDUINO_LOG_WWWSENSORACCESS,NULL);
			if (pc = forwardToMarker(pclient,clientline,"cmd=")) {
				APDDebugLog::log(APDUINO_LOG_WWWSENSORACCESS,pc);
				if (pc = forwardToMarker(pclient,clientline,"&")) {
					*pc = 0; // terminate string at '&'
				}
				char *pcmd = clientline;			// now we should have an integer /no error handling/
				APDDebugLog::log(APDUINO_LOG_WWWSENSORACCESS,pcmd);
				if (strlen(pcmd) && (!this->pra->bProcRules ||											// if rule proc inactive,
					(forwardToMarker(pclient,clientline,"force=")&&pclient->read()=='1'))) {// or force=1 flag given
					// we "can" write the sensor
					ok = true;
					this->psa->pAPDSensors[ic]->command(pcmd);
					// log action
					char sztmp[16]="";
					sprintf_P(sztmp,PSTR("s%d:%s."),ic,pcmd);
					APDDebugLog::log(APDUINO_LOG_WWWSENSORACCESS,sztmp);		// TODO change code
					// done
				} else {
					APDDebugLog::log(APDUINO_LOG_WWWSENSORACCESS,"RBLOCK");	// should not write, todo log with levels enabled
				}
			}
		}
		if (ok) {
			header(pclient,CONTENT_TYPE_HTML);
			json_status(pclient);
		} else {
			web_notfound(pclient);
		}
	} // authorized
}

void APDWeb::procSystemReq(EthernetClient *pclient, char *clientline) {
	char *pc = strstr_P(clientline, PSTR("POST /systems/"));
	pc+=14;								    //skip to post 'POST /systems/'
	int ic = atoi(pc);				// get the system ctrl id
	APDDebugLog::log(APDUINO_LOG_WWWSYSACCESS,clientline);
	if (basicAuthorize(pclient)) {
		// todo imlement a simple set value on system controls
		APDDebugLog::log(APDUINO_LOG_WWWSYSACCESS,clientline);
		switch (ic) {
		case 0: 				// loglevel
			if (pc = forwardToMarker(pclient,clientline,"value=")) {
				int iv = NAN;
				if (pc = forwardToMarker(pclient,clientline,"&")) {
					*pc = 0; // terminate string at '&'
				}
				iv = atoi(clientline);			// now we should have an integer /no error handling/
				if (iv >= 0 && iv <= LOG_LEVEL_QUIET) {
					APDDebugLog::set_loglevel(iv);
					APDDebugLog::log(APDUINO_LOG_WWWDEBUGLEVEL,clientline);
				}
			}
			header(pclient,CONTENT_TYPE_JSON);		// let's just send status ok and nothing
			break;
		case 8:					// for now a hardcoded 8 represents rule processing
										// and we simply toggle, whatever the value param is
			this->pra->toggle_processing();				// for now just toggle processing
			header(pclient,CONTENT_TYPE_JSON);		// let's just send status ok and nothing
			json_status(pclient);// todo write back the curr. val?
			break;
		default:
			web_notfound(pclient);	// simply not found for now
		}
		// todo later implement force option to inject value into control even if there is a running ruleset
	}	// authorized
}


// forwardToMarker
// reads the input stream of the client into a character buffer (should be big enough)
// szBuf - the character buffer (allocated by caller) that will be used to hold data read
// todo: pass size of buffer to allow shifting, now a size >=RCV_BUFSIZ is ASSUMED
// stops when marker found or when no more data from client
// returns ptr to marker if found, NULL otherwise
// Note: szBuf will hold whatever was read last. if NULL was returned, szBuf should contain
// the last portion of data from the client, up to RCV_BUFSIZ chars (well: tail)
// if marker found, than usually (unless marker was immediately read) marker at the end
// and some data (up to RCV_BUFSIZ-marker length chars) that preceded the marker
// (just in case... one should work with the return value, normally.)
char *APDWeb::forwardToMarker(EthernetClient *pclient, char *szBuf, char *szMarker) {
	//SerPrintP("FORWARD TO"); Serial.print(szMarker);
	char *pszmark = NULL;			// will return a ptr to the marker if found
	szBuf[0]=0;
	int index = 0;
	// forward to the data portion
	while (pclient->available() && ((pszmark = strstr(szBuf, szMarker))==0)) {
		char c = pclient->read();
		szBuf[index] = c;
		index++;
		szBuf[index]=0;
		// are we too big for the buffer? shift left
		if (index >= RCV_BUFSIZ-1) {
			for (int i=0; i < RCV_BUFSIZ-1; i++) szBuf[i] = szBuf[i+1];
			index = RCV_BUFSIZ -2;
		}
#ifdef DEBUG
		Serial.print(c);		// dump to serial
		pclient->print(c);	// dump to ethernet
		// DO NOT EVER SEND (A BUNCH OF) INDIVIDUAL CHARS TO APDDebug::log!
#endif
	}
#ifdef DEBUG
	SerPrintP("szBuf:"); Serial.println(szBuf);
#endif
}

void APDWeb::processProvisioningRequest(EthernetClient *pclient, boolean brespond) {
	uint8_t uProv=0;
	unsigned int bytesProv = 0;
	unsigned int bytesProvSaved = 0;
	if (pclient != NULL && pclient->available()) {
		APDDebugLog::log(APDUINO_MSG_PROCPROVREQ,NULL);

		char clientline[RCV_BUFSIZ];
		char provfile[32]="";			            // todo unify name lengths
		strcpy_P(provfile,PSTR("PROV.TMP"));
		char destfile[32]="";
		int index = 0;
		clientline[index]=0;
		clientline[RCV_BUFSIZ-1]=0;

		if (brespond) web_startpage(pclient,"provisioning");
#ifdef DEBUG
		if (brespond) {
			// send back what we received - for debug
			WCPrintP(pclient, "<hr />\n");
			client.println(clientline);
			WCPrintP(pclient, "<hr />\n");
		}
#endif

		while (pclient->available()) {

			if(forwardToMarker(pclient,clientline,"destination=")) {		// TODO use PROGMEM
				if (forwardToMarker(pclient,clientline,"&")) {
					// clientline should now contain the value following 'destination='
					*strstr(clientline, "&") = 0;			// 'cut' string
#ifdef DEBUG
					if (brespond) {
						WCPrintP(pclient, "<hr/><b>DESTINATION</b>=\n");
						pclient->println(clientline);
						WCPrintP(pclient, "<hr/>\n");
					}
#endif
					strcpy(destfile,clientline);										// save the dest file name
				}

				// forward to the data portion
				if (forwardToMarker(pclient,clientline,"data=")) {
					// todo log this ("Processing Data") when enabled log levels
					// remove temp file if exists
					if (APDStorage::p_sd->exists(provfile)) APDStorage::p_sd->remove(provfile);
					SdFile tempFile(provfile, O_WRITE | O_CREAT );

					clientline[0]=0;
					index = 0;
					while (pclient->available() && strstr(clientline,"&")==0) {
						char c = pclient->read();
						if (c=='+') c=' ';        // + to space
						if (c == '%' && pclient->available() >= 2) { // hexcodes...
							char hexcode[3] = "";
							hexcode[2]=0;
							hexcode[0] = pclient->read();
							hexcode[1] = pclient->read();
							if (isHexadecimalDigit(hexcode[0]) && isHexadecimalDigit(hexcode[1])) {
								int cc = read_hex_byte(hexcode);
								char newc = cc;
								if (newc == '&') {
									bytesProv++;
									if (tempFile.isOpen()) {
										tempFile.write(newc);      // we print this character early to file
										bytesProvSaved++;
									}
#ifdef DEBUG
									Serial.print(c);
									if (brespond) pclient->print(c);
#endif

									newc = 1;      // hack / & would stop the checks/
								}
								c = newc;
							} else {		// incomplete hex code??!
								APDDebugLog::log(APDUINO_ERROR_BROKENHEXCODE,NULL);
							}
						}

						if (c != 1 && c!= '&') {      // if not & /does not belong to data anymore/ or ascii(1) /we made it so, no other nonprintables expected/
							bytesProv++;
							if (tempFile.isOpen()) {
								tempFile.write(c);    // write the char to the tempfile
								bytesProvSaved++;
							}
#ifdef DEBUG
							Serial.print(c);
							pclient->print(c);
#endif
						}
						clientline[index] = c;
						index++;
						clientline[index]=0;
						// are we too big for the buffer? shift left
						if (index >= RCV_BUFSIZ-1) {
							for (int i=0; i < RCV_BUFSIZ -1; i++) clientline[i] = clientline[i+1];
							index = RCV_BUFSIZ -2;
							delay(1);				// todo large files provision unreliably
						}

						// TEMPFILE SHOULD BE VALIDATED and RENAMED to DESTINATION
#ifdef DEBUG
						// continue to read more data!
						Serial.print(c);
						if (brespond) pclient->print(c);
						//continue;
#endif
					}

					if (tempFile.isOpen()) tempFile.close(); // either no more bytes or & reached, anyways, file is to be closed at this point
					if (strlen(destfile) > 0) {
						APDDebugLog::log(APDUINO_MSG_PROVFILE,destfile);

						if (APDStorage::p_sd->exists(destfile)) {
							APDStorage::rotate_file(destfile,MAX_PROVISION_BACKUP_COUNT,0);		// make backup // TODO make MAX_PROVISION_BACKUP_COUNT user configurable later on
							APDStorage::p_sd->remove(destfile);
						}
						APDStorage::p_sd->rename(provfile,destfile);
						// todo log this when enabled log levels
						uProv++;
					}

					if (strstr(clientline, "&")) {
						*strstr(clientline, "&") = 0;
						// todo log this with (clientline) when enabled log levels
					}
				}

			} else {	// invalid request or end of provisioning
				if (uProv<1) { if (brespond)  WCPrintP(pclient, "INVALID REQUEST\n"); }
				// TODO ADD ERROR LOG HERE
			}

		} 	// while client has available bytes

		if (brespond) {
			WCPrintP(pclient, "<hr />\n"
												"Received "); pclient->print(bytesProv);
			WCPrintP(pclient, " bytes, provisioned "); pclient->print(bytesProv);
			WCPrintP(pclient," bytes into "); pclient->print(uProv); WCPrintP(pclient," files.\n");
		}
		if (bytesProv != bytesProvSaved) {
			if (brespond) WCPrintP(pclient, "<div class=\"error\">Corrupted provisioning!?</div>");
			// todo log this when enabled log levels
		}
		// TODO a callback to apduino online would be nice
		// TODO or a redirect
		// TODO and handling joined reconfig/reloadrules/reboot instructions
		if (brespond) web_endpage(pclient);
	}
}


// process APDuino Online server response in return to a registration request
// possible scenarios:
// * response for new registration -> we receive an API_KEY, we must store and repeat the request with the new key
// * response for confirmed registration -> we receive confirmation
//
// TODO: server might send auto-configuration data (eg. when a firmware upgrade is detected and the new firmware requires updated configuration files) - in this case this data should be processed and stored
void APDWeb::registration_response(APDWeb *pAPDWeb){
	APDDebugLog::log(APDUINO_MSG_AOSRRESPPROC,NULL); // log this when enabled log levels "SR: Processing server response..."
	bool bReReg = false;
	delay(500);    // debug
	int content_length = -1;
	boolean apikeyconfirmed=false;
	char new_api_key[65]="";
	if (pAPDWeb->pwwwclient != NULL && pAPDWeb->pwwwclient->available()) {
		APDDebugLog::log(APDUINO_MSG_AOSRRESPPROC,NULL); // log this when enabled log levels "SR: Processing server response..."
		while (pAPDWeb->pwwwclient->available()) {    // with bytes to read
			char www_respline[RCV_BUFSIZ] ="";
			int index = 0;
			boolean bProcessingBody = false;
			int iStatusCode = -1;
			while (pAPDWeb->pwwwclient->available()) {
				while (pAPDWeb->pwwwclient->available() && strstr(www_respline, "\n")==0) {
					char c = pAPDWeb->pwwwclient->read();
					if (c!='\r') { //ignore \r
						www_respline[index] = c;
						index++;
						www_respline[index]=0;
						if (index >= RCV_BUFSIZ-1) {           // are we too big for the buffer? shift left
							for (int i=0; i < RCV_BUFSIZ-1; i++) www_respline[i] = www_respline[i+1];
							index = RCV_BUFSIZ -2;
						}
					}
				}

				// a line in www_respline
				//APDDebugLog::log(APDUINO_MSG_AOSRRESPPROC, www_respline); // todo log this when enabled log levels (www_respline);
				if (!bProcessingBody) {      // if fetching lines of the HTTP header
					if (iStatusCode < 0) {          // if no status code yet
						sscanf_P(www_respline,PSTR("Status: %d"),&iStatusCode);    // scan for status
					} else {                       // if status already found
						// todo log this when enabled log levels (iStatusCode);
						if (www_respline[0] == '\n') {    // if blank line (separating header and body)
							bProcessingBody = true;            // now comes the body part
						}
					}
				} else {                  // if fetching lines from the HTTP body
					//TODO reinspect this part of the code, it seems to fail to read the first line (too sleepy now)
					if (content_length < 0) {  // we are in the body but no length yet (1st row)
						content_length = atoi(www_respline);
						// todo log this when enabled log levels(content_length);
					} else { 									// we are in the body somewhere
						if (new_api_key[0]==0) {	// no api key found yet (in the server response body)
							if (sscanf_P(www_respline,PSTR("REG_API_KEY=%s"),new_api_key)) {	// scan if the line specifies api key, scan it in
								if (pAPDWeb->szAPDUINO_API_KEY[0]==0) {										// if we have no API key stored locally
									strcpy(pAPDWeb->szAPDUINO_API_KEY,new_api_key);						// copy the scanned key to the local API key

									pAPDWeb->saveAPIkey(pAPDWeb->szAPDUINO_API_KEY, "APIKEY.CFG");	// store the new key on SD
									bReReg = true;					// signal internally for repeating registration (to confirm new API KEY)

									// done with reception of a new api key
									char sztmp[128] = "";
									sprintf_P(sztmp,PSTR("Registered on APDuino Online.\nClaim device @ http://%s/devices/claim_device?api_key=%s"), pAPDWeb->apduino_server_name, pAPDWeb->szAPDUINO_API_KEY);
									APDDebugLog::log(APDUINO_MSG_AOSRRESPPROC, sztmp);
								} // end if no local API KEY
								else {
									// todo log this when enabled log levels ("API key present already.\n");
								}

							} else {
									// the response line does not contain API KEY - skip
							}
						} else {
								//
						}
					}
					// in any case (whether we had or not an API key) we just passed api key in response
				  // TODO process any provisioning data
					if (pAPDWeb->pwwwclient->available() && pAPDWeb->szAPDUINO_API_KEY[0]!=0) {
						if (strstr(www_respline,pAPDWeb->szAPDUINO_API_KEY)) {	// check for api key
							apikeyconfirmed = true;
							// todo log this when enabled log levels "APDuino Online confirms device.")
						} else if(apikeyconfirmed) {		// only accept provisioning if API key was confirmed
							char sztmp[16] = "";
							APDDebugLog::log(APDUINO_MSG_AOSRRESPJOINTPROV,itoa(pAPDWeb->pwwwclient->available(),sztmp,10));
							pAPDWeb->processProvisioningRequest(pAPDWeb->pwwwclient, false);  // process any provisioning data without rendering a response
							APDDebugLog::log(APDUINO_MSG_AOSRRESPJOINTPROV,NULL);             // log this when enabled log levels "Should be done"
						}
					}
				}
				// reset for the next line
				www_respline[0]=0;
				index = 0;
			}
		}
		pAPDWeb->pwwwclient->flush();
		pAPDWeb->pwwwclient->stop();

		pAPDWeb->pwwwcp = NULL;			// reset the www parser callback
		if (bReReg) {				// if it was detected as a response to a new registration, we must confirm
			// todo log this when enabled log levels "Confirming registration."
			pAPDWeb->self_register();          // re-run registration with the new API key so server creates the device
		}
	}
}

// TODO  add 'claim device' functionality:
// -> once registerdevice but no deviceid, offer link with API_KEY to claim device. it should result a deviceid

// register to APDuino Online
// send API KEY if present; if not, a new one will be received in the server response, store and repeat request to confirm key
// tell LAN IP and APDuinOS version in the request
// SERVER RESPONSE IS PROCESSED BY CALLBACK: registration_response
boolean APDWeb::self_register() {
	char www_postdata[96];
	if ( pwwwclient!=NULL ) {
		if ( !pwwwclient->connected() ) {
			//sprintf_P(www_postdata,PSTR("lan_ip=%d.%d.%d.%d&v=%s.%s"),net.ip[0],net.ip[1],net.ip[2],net.ip[3],APDUINO_VERSION,APDUINO_BUILD);
			char sztmp[20]="";
			sprintf_P(www_postdata,PSTR("lan_ip=%d.%d.%d.%d&v=%s"),net.ip[0],net.ip[1],net.ip[2],net.ip[3],apduino_fullversion(sztmp));
			APDDebugLog::log(APDUINO_LOG_AOSELFREG,www_postdata);
			if( pwwwclient->connect(apduino_server_ip, apduino_server_port) ) {
				// send the HTTP PUT request:
				WCPrintP(pwwwclient,"PUT /devices/self_register HTTP/1.1\n");
				WCPrintP(pwwwclient,"Host: ");    pwwwclient->println(apduino_server_name);
				WCPrintP(pwwwclient,"X-APDuinoApiKey: ");   pwwwclient->println(szAPDUINO_API_KEY);
				WCPrintP(pwwwclient,"User-Agent: ");
				pwwwclient->println(USERAGENT);
				WCPrintP(pwwwclient,"Content-Type: application/x-www-form-urlencoded\n");
				WCPrintP(pwwwclient,"Content-Length: ");

				// calculate the length of the sensor reading in bytes:
				// 8 bytes for "sensor1," + number of digits of the data:
				int thisLength = strlen(www_postdata);
				pwwwclient->println(thisLength);

				// last pieces of the HTTP PUT request:
				WCPrintP(pwwwclient,"Connection: close\n\n");

				// here's the actual content of the PUT request:
				pwwwclient->println(www_postdata);
				if (pwwwcp ==NULL) {
					pwwwcp = (&registration_response);      // set reader 'callback'
					APDDebugLog::log(APDUINO_LOG_AOSELFREGREQSENT,this->szAPDUINO_API_KEY);
				} else {
					APDDebugLog::log(APDUINO_ERROR_WWWCLIENTOCCUPIED,NULL);
				}
				APDDebugLog::log(APDUINO_LOG_AOSELFREGOK,NULL);
			} else {
				// TODO check this branch out
				APDDebugLog::log(APDUINO_ERROR_AOSELFREG,NULL); // could not connect
				pwwwclient->stop();
			}
		} else {
			APDDebugLog::log(APDUINO_ERROR_WWWCLIENTBUSY,NULL);
			this->wc_busy();
		}
		bWebClient = (pwwwclient!=0) && pwwwclient->connected();
		// todo log this when enabled log levels
	} else {
		APDDebugLog::log(APDUINO_ERROR_AONOWEBCLIENT,NULL);
		this->failure();
	}
}


// requires Ethernet connection to be started already
boolean APDWeb::setup_webclient() {
	if (pwwwclient==0) {
		pwwwclient = new EthernetClient;
	}
	return (pwwwclient != 0);
}


// requires Ethernet connection to be started already
void APDWeb::loop() {
	/*if (this->bEthConfigured && this->bDHCP) {
      Ethernet.maintain();
  }*/
	if (pwwwclient != NULL) {
		loop_webclient();
		if (!pwwwclient->connected()) {           // if no web client is active
			if ((this->operational_state & OPSTATE_STARTED) && !(this->operational_state & OPSTATE_PAUSED)) {
				iBusyCounter = 0;
				if (this->psa->iSensorCount>0 || this->pca->iControlCount>0) {	// TODO fix this quick hack to see properly if there is anything to log
					if (this->pmetro != NULL && this->pmetro->check()) {
						this->log_to_ApduinoOnline();
						this->pmetro->reset();
					} else if ( this->phmetro != NULL && this->phmetro->check()) {
						this->log_to_Cosm();
						delay(10);
						this->phmetro->reset();
					} else if ( this->tsmetro != NULL && this->tsmetro->check()) {
						this->log_to_ThingSpeak();
						this->tsmetro->reset();
					}
				}
			}
		} else {
			//SerPrintP("WC busy...\n");
			this->wc_busy();
		}
	} else {
		//SerPrintP("No webclient class?!\n");		// ERROR CODE?
		this->failure();
	}
	if (pwwwserver != NULL) {
		//SerPrintP("WSLOOP\n");
		loop_server();
	}
	if (bRestart) {
		//SerPrintP("RESTARTREQ\n");
		this->restart();
	}

}

bool APDWeb::pause_service() {
	// TODO wait/kill any connected clients
	bool retcode = false;

	if ((this->operational_state & OPSTATE_CONFIGURED) &&
			(this->operational_state & OPSTATE_STARTED) &&
			!(this->operational_state & OPSTATE_PAUSED)){
		this->operational_state |= OPSTATE_PAUSED;
		retcode = true;
	}
	return retcode;
}

bool APDWeb::continue_service() {
	bool retcode = false;
	if ((this->operational_state & OPSTATE_CONFIGURED) &&
			(this->operational_state & OPSTATE_PAUSED) &&
			(this->operational_state & OPSTATE_STARTED)) {
		this->operational_state &= (~OPSTATE_PAUSED);
		retcode = true;
	}
	return retcode;
}


// TODO: add size control, avoid writing to random places
void APDWeb::get_lastlog_string(char *szLogBuf) {
	char ts[20] = "";
	strcpy_P(ts,PSTR("1970/01/01 00:00:00"));        // string used for timestamp
	APDTime::nowS(ts);                 // pull correct time
	char *pcLog = szLogBuf;
	char dataString[16]="";                // make a string for assembling the data to log:

	strcpy_P(pcLog,PSTR("datarow="));
	pcLog+=8;
	strcpy(pcLog,ts);
	pcLog+=strlen(ts);
	for (int i=0;i<this->psa->iSensorCount;i++) {
		if (this->psa->pAPDSensors[i]->config.sensor_log && this->psa->pAPDSensors[i]->fvalue != NAN) {          // if sensor to be logged
			//strcpy(pcLog,this->psa->pAPDSensors[i]->config.label);
			//pcLog+=strlen(this->psa->pAPDSensors[i]->config.label);
			*pcLog=','; pcLog++;// *pcLog = '\0';
			this->psa->pAPDSensors[i]->get_value_str(dataString);
			strcpy(pcLog,dataString);
			pcLog+=strlen(dataString);
		}
	}
	*pcLog='\n'; pcLog++; *pcLog='\0'; // \n\0
}


// TODO: add size control, avoid writing to random places
void APDWeb::get_cosmlog_string(char *szLogBuf) {
	char *pcLog = szLogBuf;				// write pointer on the buf
	char dataString[32]="";                // make a string for assembling the data to log:

	for (int i=0;i<this->psa->iSensorCount;i++) {
		if (this->psa->pAPDSensors[i]->config.sensor_log && this->psa->pAPDSensors[i]->fvalue != NAN ) {          // if sensor to be logged & has a valid value (todo use sensor states)
			strcpy(pcLog,this->psa->pAPDSensors[i]->config.label);
			pcLog+=strlen(this->psa->pAPDSensors[i]->config.label);
			*pcLog=','; pcLog++;// *pcLog = '\0';
			this->psa->pAPDSensors[i]->get_value_str(dataString);
			strcpy(pcLog,dataString);
			pcLog+=strlen(dataString);
			*pcLog='\n'; pcLog++;
		}
	}
	for (int i=0;i<this->pca->iControlCount;i++) {
		if (this->pca->pAPDControls[i]->config.control_log) {          // if control to be logged
			strcpy(pcLog,this->pca->pAPDControls[i]->config.label);
			pcLog+=strlen(this->pca->pAPDControls[i]->config.label);
			*pcLog=','; pcLog++;// *pcLog = '\0';
			this->pca->pAPDControls[i]->get_value_str(dataString);
			strcpy(pcLog,dataString);
			pcLog+=strlen(dataString);
			*pcLog='\n'; pcLog++;
		}
	}
	*pcLog='\n'; pcLog++; *pcLog='\0'; // \n\0
}

// TODO: add size control, avoid writing to random places
void APDWeb::get_thingspeaklog_string(char *szLogBuf) {
	uint8_t uSens=0;
	char *pcLog = szLogBuf;
	char dataString[16]="";                // make a string for assembling the data to log:

	for (int i=0;i<this->psa->iSensorCount;i++) {
		if (this->psa->pAPDSensors[i]->config.sensor_log && this->psa->pAPDSensors[i]->fvalue != NAN) {          // if sensor to be logged
			char szFN[16]="";
			uSens++;
			sprintf_P(szFN,PSTR("field%d"),uSens);
			if (pcLog > szLogBuf) {
				*pcLog='&'; pcLog++;
			}

			strcpy(pcLog,szFN);
		  pcLog+=strlen(szFN);
			*pcLog='='; pcLog++;// *pcLog = '\0';
			this->psa->pAPDSensors[i]->get_value_str(dataString);
			strcpy(pcLog,dataString);
			pcLog+=strlen(dataString);
			//*pcLog='\n'; pcLog++;
		}
	}
	*pcLog='\n'; pcLog++; *pcLog='\0'; // \n\0
}


// requires Ethernet connection to be started already
void APDWeb::log_to_ApduinoOnline() {
	APDDebugLog::log(APDUINO_DEBUG_AOLOGCALLED,NULL);
	//APDDebugLog::disable_sync_writes();
	char www_logdata[256];
	if ( pwwwclient ) {           // TODO check if we're registered
		if ( !pwwwclient->connected() ) {
			get_lastlog_string(www_logdata);

			char sztmp[11] = "";
			APDDebugLog::log(APDUINO_DEBUG_AOLOGGING,ultoa(strlen(www_logdata),sztmp,10));		// logdata has \n
			// todo log this when enabled log levels (this->pstr_APDUINO_API_KEY) (www_logdata)

			if( pwwwclient->connect(apduino_server_ip, apduino_server_port) ) {
				// todo log this when enabled log levels
				// send the HTTP PUT request:
				WCPrintP(pwwwclient,"PUT "); pwwwclient->print(WEBLOG_URI); WCPrintP(pwwwclient," HTTP/1.1\n");
				WCPrintP(pwwwclient,"Host: ");    pwwwclient->println(apduino_server_name);
				// TODO fix api key
				WCPrintP(pwwwclient,"X-APDuinoApiKey: ");   pwwwclient->println(szAPDUINO_API_KEY);
				WCPrintP(pwwwclient,"User-Agent: "); pwwwclient->println(USERAGENT);
				WCPrintP(pwwwclient,"Content-Type: application/x-www-form-urlencoded\n");
				WCPrintP(pwwwclient,"Content-Length: ");

				// calculate the length of the sensor reading in bytes:
				// 8 bytes for "sensor1," + number of digits of the data:
				unsigned long thisLength = strlen(www_logdata);
				pwwwclient->println(thisLength);

				// last pieces of the HTTP PUT request:
				WCPrintP(pwwwclient,"Connection: close\n\n");

				// here's the actual content of the PUT request:
				pwwwclient->println(www_logdata);
				APDDebugLog::log(APDUINO_MSG_AOLOGDONE,NULL);		// debug
			} else {
				APDDebugLog::log(APDUINO_ERROR_WWWCANTCONNECTAO,NULL);		// debug

				pwwwclient->stop();          // stop client now
				this->failure();
			}
		}
	}	else {
		APDDebugLog::log(APDUINO_ERROR_AOLOGNOWEBCLIENT,NULL);
		this->failure();
	}
	//APDDebugLog::enable_sync_writes();
	bWebClient = (pwwwclient!=0) && pwwwclient->connected();
}

void APDWeb::log_to_Cosm() {
	APDDebugLog::log(APDUINO_DEBUG_COSMLOGCALLED,NULL);
	//APDDebugLog::disable_sync_writes();
	if ( pwwwclient ) {           // TODO check if we're registered
		if ( !pwwwclient->connected() ) {
			char feedUrl[64] = "";
			char www_logdata[640]="";
			get_cosmlog_string(www_logdata);

			char sztmp[11] = "";
			APDDebugLog::log(APDUINO_DEBUG_COSMLOGGING,ultoa(strlen(www_logdata),sztmp,10));		// logdata has \n

			sprintf_P(feedUrl,PSTR("/v2/feeds/%lu.csv"),cosm_feed_id);
			// todo log this when enabled log levels with (cosm_server_name) (feedUrl)
			if( pwwwclient->connect(cosm_server_ip, cosm_server_port) ) {
				// send the HTTP PUT request:
				WCPrintP(pwwwclient,"PUT "); pwwwclient->print(feedUrl); WCPrintP(pwwwclient," HTTP/1.1\n");
				WCPrintP(pwwwclient,"Host: ");    pwwwclient->println(cosm_server_name);
				WCPrintP(pwwwclient,"X-ApiKey: ");   pwwwclient->println(szCOSM_API_KEY);
				WCPrintP(pwwwclient,"User-Agent: "); pwwwclient->println(USERAGENT);
				WCPrintP(pwwwclient,"Content-Type: text/csv\n");
				WCPrintP(pwwwclient,"Content-Length: ");

				// calculate the length of the sensor reading in bytes:
				// 8 bytes for "sensor1," + number of digits of the data:
				unsigned long thisLength = strlen(www_logdata);
				pwwwclient->println(thisLength);

				// last pieces of the HTTP PUT request:
				WCPrintP(pwwwclient,"Connection: close\n\n");

				// here's the actual content of the PUT request:
				pwwwclient->println(www_logdata);

				APDDebugLog::log(APDUINO_MSG_COSMLOGDONE,NULL);		// debug
			} else {
				APDDebugLog::log(APDUINO_ERROR_CLOGCONNFAIL,NULL);
				pwwwclient->stop();          // stop client now
				this->failure();
			}
		}
	}	else {
		APDDebugLog::log(APDUINO_ERROR_CLOGNOWEBCLIENT,NULL);
		this->failure();
	}
	//APDDebugLog::enable_sync_writes();
	bWebClient = (pwwwclient!=0) && pwwwclient->connected();
}


void APDWeb::log_to_ThingSpeak() {
	APDDebugLog::log(APDUINO_DEBUG_TSLOGCALLED,NULL);
	if ( pwwwclient ) {           // TODO check if we're registered
		if ( !pwwwclient->connected() ) {
			char feedUrl[64] = "";
			char www_logdata[256];
			get_thingspeaklog_string(www_logdata);

			char sztmp[11] = "";
			APDDebugLog::log(APDUINO_DEBUG_TSLOGGING,ultoa(strlen(www_logdata),sztmp,10));
			// todo log this when enabled log levels (www_logdata) (thingspeak_server_name) (feedUrl)
			if( pwwwclient->connect(thingspeak_server_ip, thingspeak_server_port) ) {
				// send the HTTP PUT request:
				WCPrintP(pwwwclient,"POST /update HTTP/1.1\n");
				WCPrintP(pwwwclient,"Host: ");    pwwwclient->println(thingspeak_server_name);
				WCPrintP(pwwwclient,"X-THINGSPEAKAPIKEY: ");   pwwwclient->println(szTHINGSPEAK_API_KEY);
				WCPrintP(pwwwclient,"User-Agent: "); pwwwclient->println(USERAGENT);
				WCPrintP(pwwwclient,"Content-Type: application/x-www-form-urlencoded\n");
				WCPrintP(pwwwclient,"Content-Length: ");

				// calculate the length of the sensor reading in bytes:
				// 8 bytes for "sensor1," + number of digits of the data:
				int thisLength = strlen(www_logdata);
				pwwwclient->println(thisLength);

				// last pieces of the HTTP PUT request:
				WCPrintP(pwwwclient,"Connection: close\n");
				pwwwclient->println();

				// here's the actual content of the PUT request:
				pwwwclient->println(www_logdata);
				//APDDebugLog::log(APDUINO_MSG_TSLOGDONE,NULL);		// debug
			} else {
				//APDDebugLog::log(APDUINO_ERROR_TSLOGCONNFAIL,NULL);
				pwwwclient->stop();          // stop client now
				this->failure();
			}
		}
	}	else {
		// todo log this when enabled log levels
	}
	bWebClient = (pwwwclient!=0) && pwwwclient->connected();
}

// could read anything (any request)
// now used for the web logging
// operations will be sequential anyway, so should keep track of the operation going on to match with the status
// once we have more operations
void APDWeb::loop_webclient()
{
	if (pwwwclient) {                // if we have an EthernetClient instantiated
		if(pwwwclient->available()) {    // with bytes to read
			// todo log this when enabled log levels
			if (pwwwcp != NULL) {
				// todo log this when enabled log levels
				(*pwwwcp)(this);    // call processor
				// TODO: commented out the line below that enforced that the wwwprocessor function is reset. make sure the processors reset themselves!
				//pwwwcp = NULL;  // no more processor ;-)
			}

			// read anything remaining
			// todo log this when enabled log levels "Processing leftovers...\n"

			while (pwwwclient->available()) {    // with bytes to read
				char c = pwwwclient->read();        // then read a byte
#ifdef DEBUG
				Serial.print(c);
#endif
			}}

		// if the server's disconnected, stop the client:
		if (!pwwwclient->connected()) {
			// todo log this when enabled log levels
			pwwwclient->stop();
			bWebClient = false;
		}
	}	else {
		// todo log this when enabled log levels ("WL: error, no web client.\n");
	}
}

void APDWeb::myCPrintP(EthernetClient *pClient, void *Pstring) {
	if (Pstring!=NULL && pClient != NULL) {
		int ilen = strlen_P((char*)Pstring);
		char *psob = (char*)malloc(sizeof(char)*ilen+1);
		if (psob != 0) {
			memset(psob,0,sizeof(char)*ilen+1);
			strcpy_P(psob, (char*)Pstring);
			pClient->print(psob);
			free(psob);
		} else {
			APDDebugLog::log(APDUINO_ERROR_PRINTCLIENTOUTOFMEM,NULL);
		}
	}
}


void APDWeb::new_apduinoconf_parser(void *pAPDWeb, int iline, char *psz) {
	APDWeb *pw = (APDWeb*)pAPDWeb;
	char szhost[32];
	unsigned long uLogFreq = DEFAULT_ONLINE_LOG_FREQ;
	// todo log this when enabled log levels
	//        hostname  |IP4     |Port|feedid
	sscanf_P( psz, PSTR("%s %2x%2x%2x%2x,%d,%lu"),
			szhost,
			&(pw->apduino_server_ip[0]),&(pw->apduino_server_ip[1]),&(pw->apduino_server_ip[2]),&(pw->apduino_server_ip[3]),
			&(pw->apduino_server_port),
			&(uLogFreq));

	//TODO check for errors and use an internal (class) index to keep track of the next rule to be populated
	strncpy(pw->apduino_server_name,szhost,31);
	//Serial.println(szhost); Serial.println(pw->apduino_server_name);
	//Serial.println(pw->apduino_logging_freq);
	// now do something with the values parsed...
}


void APDWeb::new_cosmconf_parser(void *pAPDWeb, int iline, char *psz) {
	APDWeb *pw = (APDWeb*)pAPDWeb;
	char szhost[32];
	//Serial.print("COSM READ: "); Serial.print(psz);
	//        hostname  |IP4     |Port|feedid
	sscanf_P( psz, PSTR("%s %2x%2x%2x%2x,%d,%lu,%lu"),
			szhost,
			&(pw->cosm_server_ip[0]),&(pw->cosm_server_ip[1]),&(pw->cosm_server_ip[2]),&(pw->cosm_server_ip[3]),
			&(pw->cosm_server_port),
			&(pw->cosm_feed_id),
			&(pw->cosm_logging_freq));

	strncpy(pw->cosm_server_name,szhost,31);
	// todo log this when enabled log levels
	// Cosm config should be in APDWeb now
}

void APDWeb::new_thingspeakconf_parser(void *pAPDWeb, int iline, char *psz) {
	APDWeb *pw = (APDWeb*)pAPDWeb;
	char szhost[32];
	// todo log this when enabled log levels
	//        hostname  |IP4     |Port|feedid
	sscanf_P( psz, PSTR("%s %2x%2x%2x%2x,%d,%lu,%lu"),
			szhost,
			&(pw->thingspeak_server_ip[0]),&(pw->thingspeak_server_ip[1]),&(pw->thingspeak_server_ip[2]),&(pw->thingspeak_server_ip[3]),
			&(pw->thingspeak_server_port),
			&(pw->thingspeak_logging_freq));

	strncpy(pw->thingspeak_server_name,szhost,31);
	// todo log this when enabled log levels
	// TS config now should be in APDWeb...
}


void APDWeb::dumpPachube() {
	char conffile[14]="";
	strcpy_P(conffile,PSTR("PACHUBE.CFG"));
	if (APDStorage::p_sd->exists(conffile)) {
		APDStorage::p_sd->remove(conffile);
	}
	SdFile dataFile(conffile, O_WRITE | O_CREAT );
	if (dataFile.isOpen()) {
		char line[RCV_BUFSIZ]="";
		// TODO update with recent fields
		sprintf_P(line,PSTR("%s %2x%2x%2x%2x,%d,%lu,%lu"),
				this->cosm_server_name,
				(this->cosm_server_ip[0]),(this->cosm_server_ip[1]),(this->cosm_server_ip[2]),(this->cosm_server_ip[3]),
				(this->cosm_server_port),
				(this->cosm_feed_id),
				(this->cosm_logging_freq));
		dataFile.println(line);

		dataFile.close();
		// todo log this when enabled log levels ("Pachube Config dumped.");
	} else {
		APDDebugLog::log(APDUINO_ERROR_COSMDUMPOPEN,conffile);
	}
	saveAPIkey(szCOSM_API_KEY,"PACHUBE.KEY");
}

// header - returns HTTP OK status code and sets response content type
void APDWeb::header(EthernetClient *pClient, int content_type) {
	WCPrintP(pClient,"HTTP/1.1 200 OK\n"
			"Content-Type: ");
	switch (content_type) {
	case CONTENT_TYPE_HTML:
		WCPrintP(pClient,"text/html");
		break;
	case CONTENT_TYPE_JSON:
		WCPrintP(pClient,"application/json");
		break;
	case CONTENT_TYPE_JAVASCRIPT:
		WCPrintP(pClient,"application/javascript");
		break;
		// todo implement these later, preferably having an extension-content type mapping file provisioned to SD
	case CONTENT_TYPE_CSS:
	case CONTENT_TYPE_PNG:
	case CONTENT_TYPE_JPG:
	case CONTENT_TYPE_GIF:
	case CONTENT_TYPE_TEXT:
	default:
		WCPrintP(pClient,"text/plain");
	}
	WCPrintP(pClient,"\n\n");
	delay(1);
}

void APDWeb::json_array_item(EthernetClient *pClient, const int index, const char *name, const char *value, const char *logged ) {
	if (index>0) {
		WCPrintP(pClient,",\n");
	}
	WCPrintP(pClient,"{ \"Index\":\"");
	pClient->print(index); WCPrintP(pClient,"\",\n");
  WCPrintP(pClient,"\"Name\":\"");
	pClient->print(name);
	WCPrintP(pClient,"\",\n\"Value\":\"");
	pClient->print(value);
	WCPrintP(pClient,"\",\n\"Logged\":\"");
	pClient->print(logged);
	WCPrintP(pClient,"\"}");			// End Sensor
	//delay(1);
}

void APDWeb::json_status(EthernetClient *pClient) {
	if (*pClient) {
		char tbuf[20] ="";					// temp buf for string fragments

		WCPrintP(pClient,"[");			// main array

		WCPrintP(pClient,"{ \"name\": \"systems\", \"data\": [");
		//sprintf_P(tbuf,PSTR("%s.%s"),APDUINO_VERSION,APDUINO_BUILD);
		//json_array_item(pClient,0,"version",tbuf,"0");
		json_array_item(pClient,0,"version",apduino_fullversion(tbuf),"0");
		strcpy_P(tbuf,PSTR("1970/01/01 00:00:00")); 			       // default string used for timestamp
		json_array_item(pClient,1,"timestamp",APDTime::nowS(tbuf),"0");
    json_array_item(pClient,2,"uptime",APDTime::get_uptime_str(tbuf),"0");
		json_array_item(pClient,3,"wwwclient",dtostrf(uCCount,5,0,tbuf),"0");
		json_array_item(pClient,4,"netfail",dtostrf(iFailureCount,5,0,tbuf),"0");
		json_array_item(pClient,5,"netrestarts",dtostrf(iRestartCount,5,0,tbuf),"0");
		json_array_item(pClient,6,"ramfree",dtostrf(freeMemory(),5,0,tbuf),"0");
		json_array_item(pClient,7,"debuglevel",dtostrf(APDDebugLog::loglevel,16,0,tbuf),"0");
		json_array_item(pClient,8,"auto",dtostrf(this->pra->bProcRules,16,0,tbuf),"0");
		json_array_item(pClient,9,"sdfree","NAN","0"); //dtostrf(APDStorage::get_sd_free_cluster_bytes(),16,0,tbuf)
		WCPrintP(pClient,"] },\n");  // End System data Array, System

		WCPrintP(pClient,"{ \"name\": \"sensors\", \"data\": [");
		for (int i = 0; i < this->psa->iSensorCount; i++) {
			json_array_item(pClient,i,this->psa->pAPDSensors[i]->config.label,this->psa->pAPDSensors[i]->get_value_str(tbuf),((this->psa->pAPDSensors[i]->config.sensor_log) ? "1" : "0"));
		}
		WCPrintP(pClient,"] },\n");  // End Sensors data Array, Sensors

		WCPrintP(pClient,"{ \"name\": \"controls\", \"data\": [");
		for (int i = 0; i < this->pca->iControlCount; i++) {
			json_array_item(pClient,i,this->pca->pAPDControls[i]->config.label,this->pca->pAPDControls[i]->get_value_str(tbuf),((this->pca->pAPDControls[i]->config.control_log) ? "1" : "0"));
		}
		WCPrintP(pClient,"] },\n");		// End Controls data Array, Controls

		WCPrintP(pClient,"{ \"name\": \"rules\", \"data\": [");
		for (int i = 0; i < this->pra->iRuleCount; i++) {
			json_array_item(pClient,i,this->pra->pAPDRules[i]->config.label,this->pra->pAPDRules[i]->get_value_str(tbuf),"0");
		}
		WCPrintP(pClient,"] }\n");	// End Rules data Array, Rules

		WCPrintP(pClient,"]");		// End Main Array
		// give the web browser time to receive the data
		delay(1);
		// todo log this when enabled log levels
	} else {
		APDDebugLog::log(APDUINO_ERROR_JSNOCLIENT,NULL);
	}
}
