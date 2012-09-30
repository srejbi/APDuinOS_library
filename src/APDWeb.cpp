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
 * APDWeb.cpp
 *
 *  Created on: Mar 28, 2012
 *      Author: George Schreiber
 */

#include "APDWeb.h"

#define WCPrintP(pc,s) this->myCPrintP(pc,(PSTR(s)));

APDWeb::APDWeb(APDTime *pTime)
{
	// TODO Auto-generated constructor stub
	initBlank(pTime);
	if (start()) {
		SerPrintP("Eth started.\n");
	} else {
		SerPrintP("Failed to configure Ethernet. Fix DHCP on your LAN or provide a valid static config on SD and reset.\n");
	}
}


APDWeb::~APDWeb()
{
	if (pwwwclient != NULL) free(pwwwclient);
	if (pwwwserver != NULL) free(pwwwclient);

	// TODO Auto-generated destructor stub
}

APDWeb::APDWeb(NETCONF *pnc, APDTime *pTime)
{
	// TODO Auto-generated constructor stub
	initBlank(pTime);
	memcpy(&net,pnc,sizeof(NETCONF));             // copy the provided config
	if (start()) {
		SerPrintP("Eth started with conf provided.\n");
	} else {
		SerPrintP("Failed to start Ethernet with static configuration. Fallback to DHCP.\n");
		initBlank(pTime);
		if (start()) {
			SerPrintP("Ethernet should be ok.\n");
		} else {
			SerPrintP("Fix DHCP on your LAN or provide a valid static config on SD and reset.\n");
		}
	}
}



void APDWeb::initBlank(APDTime *pTime)
{
#ifdef DEBUG
	SerPrintP("APDWeb initializing...\n");
#endif
	//pAPDTime = NULL;
	pAPDTime = pTime;
#ifdef DEBUG
	if (pAPDTime == NULL) SerPrintP("NO VALID TIME SOURCE!\n");
#endif
	// TODO generate mac randomness
	memcpy(&net.mac,(byte []){ 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED },6);            //TODO generate random mac, more than 1 APDuino can have collisions
	memset(&net.ip,0,4*sizeof(byte));                          // uninitialized address
	memset(&net.gateway,0,4*sizeof(byte));                     // uninitialized address
	memset(&net.subnet,0,4*sizeof(byte));                      // uninitialized address
	memset(&net.pridns,0,4*sizeof(byte));                      // uninitialized address

	//memcpy_P(&apduino_server_ip,(byte[])APDUINO_SERVER_IP,4*sizeof(byte));           // apduino.localhost -- test server on LAN
	memcpy(&apduino_server_ip,APDUINO_SERVER_IP,4*sizeof(byte));           // apduino.localhost -- test server on LAN
	strcpy_P(apduino_server_name,APDUINO_SERVER);                   // test APDuino server on LAN
	apduino_server_port = 80;                 // standard HTTP port
	apduino_logging_freq = DEFAULT_ONLINE_LOG_FREQ;

	//memcpy_P(&cosm_server_ip,(byte[])COSM_SERVER_IP,4*sizeof(byte));           // api.pachube.com 216.52.233.122
	memcpy(&cosm_server_ip,COSM_SERVER_IP,4*sizeof(byte));           // api.pachube.com 216.52.233.122
	strcpy_P(cosm_server_name, COSM_SERVER );
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
	pAPDSensors = NULL;
	pAPDControls = NULL;
	pAPDRules = NULL;
	iSensorCount = -1;
	iControlCount = -1;
	iRuleCount = -1;
	pAPDStorage = NULL;
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

boolean APDWeb::start() {

	SerPrintP("\nMAC:");
	for (byte thisByte = 0; thisByte < 6; thisByte++) {
		Serial.print(net.mac[thisByte],HEX);
		if (thisByte < 5) SerPrintP(":");
	}

	//net.ip[0] = 0;
	if ((bRestart && !this->bDHCP) || (!bRestart && net.ip[0] != 0)) {                 // if an IP seems to be provided

		SerPrintP("Trying static IP...\n");
		SerPrintP("IP: ");
		// display and save in RAM config to netconfig
		SerDumpIP(net.ip);
		SerPrintP("MASK: ");
		// display and save in RAM config to netconfig
		SerDumpIP(net.subnet);
		SerPrintP("GW: ");
		// display and save in RAM config to netconfig
		SerDumpIP(net.gateway);
		SerPrintP("DNS: ");
		// display and save in RAM config to netconfig
	    SerDumpIP(net.pridns);

		Ethernet.begin(net.mac, net.ip, net.pridns, net.gateway, net.subnet);
		bEthConfigured = true;
		operational_state = OPSTATE_CONFIGURED | OPSTATE_STARTED;
		this->bDHCP = false;
		// todo read on howto check state
	} else {                              // go for DHCP
		SerPrintP("Trying DHCP...");
		if (Ethernet.begin(net.mac) == 0) {
			SerPrintP("Error.");
			operational_state = OPSTATE_BLANK | OPSTATE_ERROR;
			return false;
		}
		// we should have a lease now
		SerPrintP("DHCP DONE.\nIP: ");

		// we should have a lease now, save data
		uint32_t ipaddr = (uint32_t)Ethernet.localIP();
		memcpy((void *)(byte *)net.ip, (void *)&ipaddr, sizeof(byte)*4);
		ipaddr = (uint32_t)Ethernet.subnetMask();
		memcpy((void *)(byte *)net.subnet, (void *)&ipaddr, sizeof(byte)*4);
		ipaddr = (uint32_t)Ethernet.gatewayIP();
		memcpy((void *)(byte *)net.gateway, (void *)&ipaddr, sizeof(byte)*4);
		// TODO what about DNS?
#if (ARDUINO >= 101)
		ipaddr = Ethernet.dnsServerIP();
		memcpy((void *)(byte *)net.pridns, (void *)&ipaddr, sizeof(byte)*4);
#endif
		// display and save in RAM config to netconfig
		SerDumpIP(net.ip);
		SerPrintP("\nMASK: ");
		SerDumpIP(net.subnet);
		SerPrintP("\nGW: ");
		SerDumpIP(net.gateway);
		SerPrintP("\nDNS: ");
	    SerDumpIP(net.pridns);

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
	// if running in a multithreading environment, we should lock the APDWeb with a semaphore here...
	if (bEthConfigured) {
		if (pwwwclient) {
			SerPrintP("STOPPING CLIENTS\n");
			pwwwclient->stop();
			delay(1000);
		}
		SerPrintP("RESTARTING NET\n");
		bEthConfigured = false;
		operational_state = OPSTATE_BLANK;
		if (this->start()) {
			this->self_register();
		} else {
			operational_state |= OPSTATE_ERROR;
			this->failure();
		}
	} else {
		return false;
	}
	return bEthConfigured;
}

void APDWeb::failure() {
	SerPrintP("FC"); Serial.print(this->iFailureCount); SerPrintP("++");
	if (++this->iFailureCount >= MAX_NET_FAILURES) {	// TODO replace 3 with a variable
		SerPrintP("->FC="); Serial.print(iFailureCount);
		this->bRestart = true;		// request a restart (will be done in loop)
		SerPrintP(" -> restart req.\n");
	}
}

void APDWeb::wc_busy() {
	//SerPrintP("WCB"); Serial.print(this->iBusyCounter); SerPrintP("++");
	unsigned long nowms = millis();
	if (wcb_millis == 0 || iBusyCounter == 0) {
		wcb_millis = nowms;
	}

	if (++this->iBusyCounter >= MAX_WEBCLIENT_BUSY_LOOPS  || (nowms - wcb_millis > WEBCLIENT_BUSY_TIMEOUT_MS)) {	// TODO replace 50 with a variable
		SerPrintP("->WCB="); Serial.print(iBusyCounter);
		SerPrintP(" busy ms:"); Serial.print(nowms - wcb_millis); SerPrintP("\n");
		this->failure();
		this->iBusyCounter = 0;
		this->wcb_millis = 0;
	}
}

boolean APDWeb::setupAPDuinoOnline() {
	boolean retcode = false;
#ifdef DEBUG
	SerPrintP("WILL SETUP APDUINO ONLINE...\n");
#endif
	szAPDUINO_API_KEY[0] = 0;
	if (bEthConfigured && pAPDStorage != NULL && pAPDStorage->ready()) {
#ifdef DEBUG
		SerPrintP("WE HAVE NET\n");
#endif
//		//  TODO save reg. data to temp buffer and save after proper handshake w/ server
//		strcpy(this->apduino_server_name, hostname);
//#ifdef DEBUG
//		Serial.print(apduino_server_name); SerPrintP("@");
//#endif
//		for (int i=0;i<4;i++) {
//			this->apduino_server_ip[i] = (*phostip)[i];
//#ifdef DEBUG
//			Serial.print(apduino_server_ip[i],DEC);SerPrintP(".");
//#endif
//		}
//		this->apduino_server_port = hostport;

		//loadAPIkey(szAPDUINO_API_KEY,"APIKEY.CFG");             // TODO -> load api key for ; allow multiple keys for different services




		if (pAPDStorage->readFileWithParser("ONLINE.CFG",&new_apduinoconf_parser,(void*)this) > 0) {
		} else {
			// use defaults
			memcpy(&apduino_server_ip,APDUINO_SERVER_IP,4*sizeof(byte));           // apduino.localhost -- test server on LAN
			strcpy_P(apduino_server_name,APDUINO_SERVER);                   // test APDuino server on LAN
			apduino_server_port = 80;
			apduino_logging_freq = DEFAULT_ONLINE_LOG_FREQ;
		}
		//

		SerPrintP("APDuino Online server:"); Serial.println(apduino_server_name);
		SerPrintP("@");
		SerDumpIP(apduino_server_ip);

		if (apduino_server_ip[0]>0) {                            // if we have a server name
			loadAPIkey(szAPDUINO_API_KEY,"APIKEY.CFG");             // load api key for apduino.com
			Serial.print(szAPDUINO_API_KEY);
			delay(20);

			#ifdef DEBUG
					SerPrintP("Executing self-registration proc.\n");
			#endif
			self_register();								// this will get a server-generated key

			startWebLogging(apduino_logging_freq);
			retcode = true;
//					if (this->pmetro == NULL) {
//						this->pmetro = new Metro(apduino_logging_freq, true);                     // TODO check this
//						retcode = true;
//					}


		} else {
			SerPrintP("E21.\n");
		}

		retcode = true;
	} else {
		SerPrintP("E20.\n");
	}
	return retcode;
}


boolean APDWeb::loadAPIkey(char *szAPIKey, char *szAPIFile) {
	//SerPrintP("LOADING API KEY...");
	if (pAPDStorage!=NULL && pAPDStorage->ready()) {
		int i=0;
		char line[BUFSIZ]="";
		int bread=0;
		SdFile dataFile(szAPIFile, O_RDONLY );
		if (dataFile.isOpen()) {
			//SerPrintP("Opened file\n");
			while (bread=dataFile.fgets(line, sizeof(line))) {      // get the next line
				if (char *nl=strstr(line,"\n") ) {
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
	//SerPrintP("BAILING OUT\n");
	return false;
}


void APDWeb::saveAPIkey(char *szAPIKey, char *szAPIFile)
{
	if (pAPDStorage!=NULL && pAPDStorage->ready()) {
		SdFile dataFile(szAPIFile, O_WRITE | O_CREAT);
		if (dataFile.isOpen()) {
			dataFile.println(szAPIKey);
			dataFile.close();
		} else {
			SerPrintP("IO ERR.");		// TODO add error codes
		}
	} else {
		SerPrintP("NO STORAGE.");
	}
}

boolean APDWeb::startWebLogging(unsigned long uWWWLoggingFreq) {
	boolean retcode = false;
	if (this->pmetro == NULL) {
		this->pmetro = new Metro(uWWWLoggingFreq, true);
	} else {
		this->pmetro->interval(uWWWLoggingFreq);
	}
	this->pmetro->reset();
	retcode = (this->pmetro != NULL);

	SerPrintP("APDW: ONLINE LOG SINK: ");
	Serial.print(apduino_server_name); SerPrintP("@");
	SerDumpIP(apduino_server_ip);

	SerPrintP("\n\n");
	return retcode;
}


boolean APDWeb::setupCosmLogging() {
	boolean retcode = false;
	SerPrintP("COSM...");

	szCOSM_API_KEY[0] = 0;
	if (bEthConfigured && pAPDStorage != NULL && pAPDStorage->ready() && this->phmetro == NULL) {
		if (pAPDStorage->readFileWithParser("PACHUBE.CFG",&new_cosmconf_parser,(void*)this) > 0) {
			SerPrintP("server:"); Serial.println(cosm_server_name);
			SerPrintP("@");
			SerDumpIP(cosm_server_ip);
			if (cosm_server_ip[0]>0) {                            // if we have a server name
				//SerPrintP("loading API key\n");
				loadAPIkey(szCOSM_API_KEY,"PACHUBE.KEY");             // TODO -> load api key for ; allow multiple keys for different services
				Serial.print(szCOSM_API_KEY);
				delay(20);

			} else {
				SerPrintP("FAIL.\n");
			}
			SerPrintP("SET UP.\n");
			this->phmetro = new Metro(cosm_logging_freq, true);                     // TODO check this
			retcode = true;

		} else {
			SerPrintP("CONF ERR.\n");
		}

	} else {
		SerPrintP("ERR.\n");
	}
	return retcode;
}


boolean APDWeb::setupThingSpeakLogging() {
	boolean retcode = false;
	SerPrintP("THINGSPEAK...");

	szTHINGSPEAK_API_KEY[0] = 0;
	if (bEthConfigured && pAPDStorage != NULL && pAPDStorage->ready() && this->tsmetro == NULL) {
		if (pAPDStorage->readFileWithParser("THINGSPK.CFG",&new_thingspeakconf_parser,(void*)this) > 0) {
			SerPrintP("server:"); Serial.println(thingspeak_server_name);
			SerPrintP("@");
			SerDumpIP(thingspeak_server_ip);

			if (thingspeak_server_ip[0]>0) {                            // if we have a server name
//				SerPrintP("loading API key\n");
				loadAPIkey(szTHINGSPEAK_API_KEY,"THINGSPK.KEY");             // TODO -> load api key for ; allow multiple keys for different services
				Serial.print(szTHINGSPEAK_API_KEY);
				delay(20);
				//              SerPrintP("creating streams...\n");
				//              for (int i=0;i<iSensorCount;i++) {
				//                if (pAPDSensors[i]->config.sensor_log) {          // if sensor to be logged
				//                    SerPrintP("SHOULD SETUP"); Serial.print(pAPDSensors[i]->config.label); SerPrintP(" PH STREAM\n");
				//                  setupPachubeDataStream(pAPDSensors[i]->config.label,pAPDSensors[i]->fvalue);
				//                }
				//              }



			} else {
				SerPrintP("FAIL.\n");
			}
			SerPrintP("SETUP.\n");
			this->tsmetro = new Metro(thingspeak_logging_freq, true);                     // TODO check this
			retcode = true;

		} else {
			SerPrintP("CONF ERR.\n");
		}

	} else {
		SerPrintP("ERR.\n");
	}
	return retcode;
}


void APDWeb::startWebServer(APDSensor **pSensors, int iSensorCount, APDControl **pControls, int iControlCount, APDRule **pRules, int iRuleCount, APDStorage *pAPDStorage)
{
	SerPrintP("WS");
	if (pwwwserver == NULL) {
		SerPrintP("Starting WWW Server on port "); Serial.print(net.wwwPort); SerPrintP("...\n");
		pwwwserver = new EthernetServer(net.wwwPort);
		pwwwserver->begin();
		this->pAPDSensors = pSensors;
		this->pAPDControls = pControls;
		this->pAPDRules = pRules;
		this->iSensorCount = iSensorCount;
		this->iControlCount = iControlCount;
		this->iRuleCount = iRuleCount;
		this->pAPDStorage = pAPDStorage;
		this->setup_webclient();
	} else {
		SerPrintP("WS already running?\n");
	}
}


void APDWeb::web_header(EthernetClient *pClient) {
	WCPrintP(pClient,"HTTP/1.1 200 OK\n");
	WCPrintP(pClient,"Content-Type: text/html\n\n");
	delay(1);
}

void APDWeb::web_startpage(EthernetClient *pClient, char *title,int refresh=0) {
	if (*pClient) {
		WCPrintP(pClient,"<html>\n");
		WCPrintP(pClient,"<head><title>APDUINO - "); pClient->print(title); WCPrintP(pClient,"</title>\n");
		WCPrintP(pClient,"<link rel=\"shortcut icon\" type=\"image/x-icon\" href=\"http://apduino.localhost/images/apduino_icon.png\"/>\n");
		if (refresh) {
			WCPrintP(pClient,"<meta http-equiv=\"refresh\" content=\""); pClient->print(refresh); WCPrintP(pClient,"\"/>\n");
		}
		WCPrintP(pClient,"<link href=\"http://"); pClient->print(apduino_server_name); WCPrintP(pClient,"/devices/"); WCPrintP(pClient,"stylesheet?api_key=");  pClient->print(szAPDUINO_API_KEY); WCPrintP(pClient,"\" media=\"screen\" rel=\"stylesheet\" type=\"text/css\" />");
		WCPrintP(pClient,"</head>\n");
		WCPrintP(pClient,"<body>\n");
		//WCPrintP(pClient,"<img src=\"http://"); pClient->print(apduino_server_name); WCPrintP(pClient,"/images/apduino.png\" />"
		WCPrintP(pClient,"<div class=\"apduino_header\">");
		WCPrintP(pClient,"<h1>APDuino: "); pClient->print(title);	WCPrintP(pClient,"</h1><hr/>\n");

		WCPrintP(pClient,"<div class=\"stats\">");
		WCPrintP(pClient,"<sub>Version: ");    pClient->print(APDUINO_VERSION); WCPrintP(pClient,"."); pClient->print(APDUINO_BUILD); WCPrintP(pClient,"</sub>");
		WCPrintP(pClient,"<table>\n<tr><th>TimeStamp</th><th>Uptime</th><th>WWW Client</th><th>Net Fails</th><th>Net Restarts</th><th>RAM Free</th></tr>\n");

		WCPrintP(pClient,"<tr><td>");
		char ts[] = "1970/01/01 00:00:00";        // string used for timestamp
		if (this->pAPDTime != NULL) pAPDTime->nowS(ts);
		pClient->print(ts);
		WCPrintP(pClient,"</td><td>");
		if (this->pAPDTime != NULL) pAPDTime->getUpTimeS(ts);
		pClient->print(ts);
		WCPrintP(pClient,"</td><td>");
		char tbuf[20] ="";
		dtostrf(uCCount,5,0,tbuf);
		pClient->print(tbuf);
		WCPrintP(pClient,"</td><td>");
		dtostrf(iFailureCount,5,0,tbuf);
		pClient->print(tbuf);
		WCPrintP(pClient,"</td><td>");
		dtostrf(iRestartCount,5,0,tbuf);
		pClient->print(tbuf);
		WCPrintP(pClient,"</td><td>");
		dtostrf(freeMemory(),5,0,tbuf);
		pClient->print(tbuf);
		WCPrintP(pClient,"</td></tr>\n");
		WCPrintP(pClient,"</table></div>\n");
		delay(1);
//		WCPrintP(pClient,"<p>");
//		//TODO fix timestamping - DONE (?)
//
//		WCPrintP(pClient,"</p>");
		WCPrintP(pClient,"<hr/>");
		WCPrintP(pClient,"</div>");		// closing header

		WCPrintP(pClient,"<div id=\"sidebar\"><ul>");							// sidebar
		if (!(operational_state & OPSTATE_PAUSED)) { WCPrintP(pClient,"<li><a href=\"/\">Status</a></li>"); }
		WCPrintP(pClient,"<li><a href=\"/sd/\">Files</a></li>");
		if (!(operational_state & OPSTATE_PAUSED)) { WCPrintP(pClient,"<li><a href=\"/reconfigure\">Reload Config</a></li>");  }
		if (!(operational_state & OPSTATE_PAUSED)) { WCPrintP(pClient,"<li><a href=\"/reset\">Soft Reset</a></li>");  }
		WCPrintP(pClient,"<li><a href=\"/claimdevice\">Claim Device</a></li>");
		WCPrintP(pClient,"</ul></div>");												  // closing sidebar

	} else {
		SerPrintP("Client gone.?!");
	}
}

void APDWeb::web_endpage(EthernetClient *pClient) {
	if (*pClient) {
		pClient->println("</body></html>");
#ifdef DEBUG
		SerPrintP("SEND_STATUS END.\n");
#endif
		// give the web browser time to receive the data
		delay(1);
	} else {
#ifdef DEBUG
		SerPrintP("Client gone.?!\n");
#endif
	}
}

void APDWeb::web_status(EthernetClient *pClient) {
	if (*pClient) {
		char tbuf[20] ="";
		// TODO put header into a separate function
		//WCPrintP(pClient,"<div class=\"apduino_header\>");
		//    SerPrintP("\nSEND_STATUS START.");


#ifdef DEBUG
		Serial.print(iSensorCount); SerPrintP("sensors should be listed...\n");
#endif

		WCPrintP(pClient,"<div class=\"status\">");

		WCPrintP(pClient,"<div class=\"sensors\"><h1>Sensors</h1>");
		//WCPrintP(pClient,"<table><tr><th>Sensor</th><th>Value</th><th>Log?</th><th></th></tr>");
		WCPrintP(pClient,"<table id=\"sensors_table\">");


		//    SerPrintP("Output sensors...");
		for (int i = 0; i < iSensorCount; i++) {
#ifdef DEBUG
			SerPrintP("Sensor "); Serial.print(i);
#endif
			WCPrintP(pClient,"<tr class=\"line\">");
			WCPrintP(pClient,"<td class=\"sensor_name\">");
			//WCPrintP(pClient,"SENSOR_");
			pClient->print(i); WCPrintP(pClient," : ");
			pClient->print((char *)pAPDSensors[i]->config.label);
			WCPrintP(pClient,"</td><td class=\"sensor_value\">");
			//WCPrintP(pClient," = ");
			pAPDSensors[i]->getValueS(tbuf);
			pClient->print(tbuf);
			//WCPrintP(pClient,"</td><td>");
			//if (pAPDSensors[i]->config.sensor_log) WCPrintP(pClient,"Y");
			//pClient->println("<br/>");
			WCPrintP(pClient,"</td><td");
			if (pAPDSensors[i]->config.sensor_log) {
				WCPrintP(pClient," id=\"chart-"); pClient->print((char *)pAPDSensors[i]->config.label); WCPrintP(pClient,"\"");
			}
			WCPrintP(pClient,">");
			WCPrintP(pClient,"</td></tr>");
			delay(1);
		}

		WCPrintP(pClient,"</table>");
		WCPrintP(pClient,"</div>");
		// CONTROLS OUTPUT
		WCPrintP(pClient,"<div class=\"controls\"><h1>Controls</h1>");
		//WCPrintP(pClient,"<table><tr><th>Control</th><th>Value</th><th>Log?</th><th></th></tr>");
		WCPrintP(pClient,"<table id=\"controls_table\">");

		//    SerPrintP("Output controls...");
		for (int i = 0; i < iControlCount; i++) {
#ifdef DEBUG
			SerPrintP("Control "); Serial.print(i);
#endif
			WCPrintP(pClient,"<tr class=\"line\">");
			WCPrintP(pClient,"<td class=\"control_name\">");
			pClient->print(i); WCPrintP(pClient," : ");
			pClient->print((char *)pAPDControls[i]->config.label);
			WCPrintP(pClient,"</td><td class=\"control_value\">");
			//WCPrintP(pClient," = ");
			pAPDControls[i]->getValueS(tbuf);		// get control value
			pClient->print(tbuf);
			WCPrintP(pClient,"</td><td>");
			//if (pAPDSensors[i]->config.sensor_log) WCPrintP(pClient,"Y");
			pClient->println("<br/>");
			WCPrintP(pClient,"</td><td");
			if (pAPDControls[i]->config.control_log) {
				WCPrintP(pClient," id=\"chart-"); pClient->print((char *)pAPDControls[i]->config.label); WCPrintP(pClient,"\"");
			}
			WCPrintP(pClient,">");
			WCPrintP(pClient,"</td></tr>");
			delay(1);
		}

		WCPrintP(pClient,"</table>");		// controls table
		WCPrintP(pClient,"</div>\n");		// controls div

		//    RULES OUTPUT
		WCPrintP(pClient,"<div class=\"rules\"><h1>Rules</h1>");
		//WCPrintP(pClient,"<table><tr><th>Control</th><th>Value</th><th>Log?</th><th></th></tr>");
		WCPrintP(pClient,"<table id=\"rules_table\">");

		for (int i = 0; i < iRuleCount; i++) {
#ifdef DEBUG
			SerPrintP("Rule "); Serial.print(i);
#endif
			WCPrintP(pClient,"<tr class=\"line\">");
			WCPrintP(pClient,"<td class=\"rule_name\">");
			pClient->print(i); WCPrintP(pClient," : ");
			pClient->print((char *)pAPDRules[i]->config.label);
			WCPrintP(pClient,"</td><td class=\"rule_value\">");
			//WCPrintP(pClient," = ");
			pAPDRules[i]->getValueS(tbuf);		// get control value
			pClient->print(tbuf);
			WCPrintP(pClient,"</td><td>");
			//if (pAPDSensors[i]->config.sensor_log) WCPrintP(pClient,"Y");
			pClient->println("<br/>");
			WCPrintP(pClient,"</td><td");
			//if (pAPDRules[i]->config.control_log) {
			//	WCPrintP(pClient," id=\"chart-"); pClient->print((char *)pAPDControls[i]->config.label); WCPrintP(pClient,"\"");
			//}
			WCPrintP(pClient,">");
			WCPrintP(pClient,"</td></tr>");
			delay(1);
		}

		WCPrintP(pClient,"</table>");		// controls table
		WCPrintP(pClient,"</div>\n");		// controls div


		WCPrintP(pClient,"</div>\n");		// status div
		// give the web browser time to receive the data
		delay(1);
	} else {
		SerPrintP("W02");
	}
}

void APDWeb::web_maintenance(EthernetClient *pClient) {
	WCPrintP(pClient, "HTTP/1.1 503 (Service unavailable)\n");
	WCPrintP(pClient, "Content-Type: text/html\n\n");
	WCPrintP(pClient, "<h2>The requested service is temporarily unavailable. Please try again later.</h2>\n");
}

void APDWeb::web_notfound(EthernetClient *pClient) {
	WCPrintP(pClient, "HTTP/1.1 404 Not Found\n");
	WCPrintP(pClient, "Content-Type: text/html\n\n");
	WCPrintP(pClient, "<h2>File Not Found!</h2>\n");
}

void APDWeb::claim_device_link(EthernetClient *pClient) {
	if (*pClient) {
		//    SerPrintP("\nSEND_STATUS START.");
#ifdef DEBUG
		WCPrintP(pClient,"<sub>Version: ");    pClient->print(APDUINO_VERSION); WCPrintP(pClient,"."); pClient->print(APDUINO_BUILD); WCPrintP(pClient,"</sub>");
		delay(1);
#endif
		WCPrintP(pClient,"<a href=\"http://"); pClient->print(apduino_server_name); WCPrintP(pClient,"/devices/claim_device?api_key=");
		pClient->print(szAPDUINO_API_KEY); WCPrintP(pClient,"\">Claim your APDuino!</a>");
		WCPrintP(pClient,"<hr/>");
		// give the web browser time to receive the data
		delay(1);
	} else {
		SerPrintP("W01");
	}
}


void APDWeb::ListFiles(EthernetClient client, const char *szPath, uint8_t flags) {
	// This code is just copied from SdFile.cpp in the SDFat library
	// and tweaked to print to the client output in html!
	SdFile *proot = this->pAPDStorage->p_root;
	dir_t p;
	if (this->pAPDStorage != NULL && this->pAPDStorage->ready() == true && this->pAPDStorage->p_root != NULL ) {
#ifdef DEBUG
		SerPrintP("REWIND ROOT");
#endif
		//this->pAPDStorage->p_root->rewind();
		proot->rewind();
		if (szPath != NULL && szPath[0] != 0) {
			proot = new SdFile(szPath, O_RDONLY);
		}
		WCPrintP(&client,"<div class=\"files\">\n<ul>\n");
#ifdef DEBUG
		SerPrintP("LIST FILES...");
#endif
		// link to upper level if not at root
		if (proot != this->pAPDStorage->p_root) {
			WCPrintP(&client,"<li><a href=\"../\">..</a></li>\n");
		}
		while (proot->readDir(&p) > 0 && p.name[0] != DIR_NAME_FREE) {
#ifdef DEBUG
			SerPrintP("dir read\n");
#endif
			// done if past last used entry
			//if (p.name[0] == DIR_NAME_FREE) break;
			// skip deleted entry and entries for '.'
			if (p.name[0] == DIR_NAME_DELETED || (p.name[0] == '.' && p.name[1] != '.')) continue;
			//
			if (DIR_IS_FILE_OR_SUBDIR(&p)) {
				// print any indent spaces
				WCPrintP(&client,"<li><a href=\"");
//				if (szPath != NULL && szPath[0] != 0) {
//
//					Serial.print(szPath);
//
//					client.print(szPath);
//					WCPrintP(&client,"/");
//				}
				for (uint8_t i = 0; i < 11; i++) {
					if (p.name[i] == ' ') continue;
					if (i == 8) {
						WCPrintP(&client,".");
					}
					client.print((char)p.name[i]);
					Serial.print((char)p.name[i]);
				}
				if (DIR_IS_SUBDIR(&p)) {
					WCPrintP(&client,"/");
				}
				Serial.println("");
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
					proot->printFatDate(p.lastWriteDate);
					client.print(' ');
					proot->printFatTime(p.lastWriteTime);
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
		SerPrintP("W03");
		WCPrintP(&client,"W03 - NO STORAGE!\n");
	}
	if (proot != this->pAPDStorage->p_root) {
		free(proot);
	}
}


void APDWeb::loop_server()
{
	if (pwwwserver != NULL) {      // if server is instantiated
		char clientline[BUFSIZ];
		clientline[BUFSIZ-1]=0;      // a terminating 0 at the last position
		int index = 0;

		EthernetClient client = pwwwserver->available();
		if (client) {
			uCCount++;                              // one more web client
			boolean current_line_is_blank = true;      // an http request ends with a blank line

			index = 0;                              // reset the input buffer

			while (client.connected()) {            // as long as the client is connected
				if (client.available()) {             // if there are bytes to read
					char c = client.read();                // read 1 character

					if (c != '\n' && c != '\r') {         // If it isn't a new line, add the character to the buffer
						clientline[index] = c;
						index++;

						if (index >= BUFSIZ)                // are we too big for the buffer? start tossing out data
							index = BUFSIZ -1;
						continue;            // continue to read more data!
					}
					// got a \n or \r new line, which means the string is done
					clientline[index] = 0;

					// Look for substring such as a request to get the root file
					if (strstr_P(clientline, PSTR("GET /sd/ ")) != 0) {
						// send a standard http response header
						web_header(&client);
						// print all the files, use a helper to keep it clean
						web_startpage(&client,"files");
						//WCPrintP(&client, "<h2>Files:</h2>\n");
						ListFiles(client, NULL, LS_DATE | LS_SIZE);		// list root
						web_endpage(&client);
					} else if (strstr_P(clientline, PSTR("GET /sd/")) != 0) {
						// this time no space after the /, so a sub-file!
						char *filename;

						//filename = clientline + 5; // pointer to after "GET /"
						filename = clientline + 8; // pointer to after "GET /sd/"
						(strstr_P(clientline, PSTR(" HTTP")))[0] = 0;       // terminate string just before proto string
#ifdef DEBUG
						SerPrintP("REQ: "); Serial.println(filename);  // print the file we want
#endif
            // serve file
						SdFile file;
//#ifdef DEBUG
						SerPrintP("file server\n");
//#endif
						if (! file.open(this->pAPDStorage->p_root, filename, O_READ)) {
							web_notfound(&client);
							break;
						}
#ifdef DEBUG
						SerPrintP("Opened!");
#endif
						if ( file.isFile() ) {
							//SerPrintP("FILE");
							WCPrintP(&client, "HTTP/1.1 200 OK\nContent-Type: ");
							if (strstr_P(clientline, PSTR(".htm")) != 0 || strstr_P(clientline, PSTR(".html")) != 0 ) {
								WCPrintP(&client, "text/html");
							} else {
								WCPrintP(&client, "text/plain");
							}
							WCPrintP(&client, "\n\n");

							int16_t c;
							while ((c = file.read()) > -1) {
								// uncomment the serial to debug (slow!)
								//Serial.print((char)c);
								client.print((char)c);
							}
						} else if (file.isDir()) {
							//SerPrintP("DIR");
							// send a standard http response header
							web_header(&client);
							// print all the files, use a helper to keep it clean
							web_startpage(&client,"files");
							//WCPrintP(&client, "<h2>Files:</h2>\n");
							ListFiles(client, filename, LS_DATE | LS_SIZE);		// list root
							web_endpage(&client);
						}
						file.close();
					//} else if ((strstr_P(clientline, PSTR("GET /status")) != 0 && strlen(clientline) == 11) ||		// /status
					} else if (strstr_P(clientline, PSTR("GET /status ")) != 0 ||		// /status
							strstr_P(clientline, PSTR("GET / ")) != 0) {				// also for www root
						if (!(this->operational_state & OPSTATE_PAUSED)) {
	#ifdef DEBUG
							SerPrintP("Sending HTTP Resp...");
	#endif
							// send a standard http response header
							web_header(&client);
							web_startpage(&client,"status",20);
							web_status(&client);
							web_endpage(&client);
	#ifdef DEBUG
							SerPrintP("HTTP Resp Sent.");
	#endif
						} else {
							web_maintenance(&client);
						}
					} else if (strstr_P(clientline,PSTR("GET /reconfigure")) != 0) {
						web_header(&client);
						web_startpage(&client,"reconfigure",0);
						WCPrintP(&client,"Reconfiguration request acknowledged.");
						web_endpage(&client);
						this->dispatched_requests = DREQ_RECONF;		// APDuino should read it
					} else if (strstr_P(clientline,PSTR("GET /reset")) != 0) {
						web_header(&client);
						web_startpage(&client,"reset",0);
						WCPrintP(&client,"Reset request acknowledged.");
						web_endpage(&client);
						this->dispatched_requests = DREQ_RESET;		// APDuino should read it
					}  else if (strstr_P(clientline, PSTR("GET /status.json")) != 0) {
						// send a standard http response header
						json_header(&client);
						json_status(&client);
					} else if (strstr_P(clientline,PSTR("GET /claimdevice")) != 0) {
						web_header(&client);
						web_startpage(&client,"claimdevice",0);
						claim_device_link(&client);
						web_endpage(&client);
						// TODO investigate/research why the Reset_AVR messes up the device
				/* } else if (strstr_P(filename,PSTR("reset")) != 0) {
						SerPrintP("Web initiated reset.\n");
						Reset_AVR();*/
					} else if (strstr_P(clientline, PSTR("POST /provisioning")) != 0) {
						if (!(this->operational_state & OPSTATE_PAUSED)) {
							this->processProvisioningRequest(&client);
						} else {
							web_maintenance(&client);
						}
					} else {
#ifdef DEBUG
						SerPrintP("404\n");
#endif
						// everything else is a 404
						web_notfound(&client);
					}
					break;
				}
			}

			delay(1);					// give the web browser time to receive the data
			client.stop();		// disconnect the client

		}
	}
}


void APDWeb::forwardToMarker(EthernetClient *pclient, char *szBuf, char *szMarker) {
	//SerPrintP("FORWARD TO"); Serial.print(szMarker);
	szBuf[0]=0;
	int index = 0;
	// forward to the data portion
	while (pclient->available() && strstr(szBuf, szMarker)==0) {
		char c = pclient->read();
		szBuf[index] = c;
		index++;
		szBuf[index]=0;
		// are we too big for the buffer? shift left
		if (index >= BUFSIZ-1) {
			for (int i=0; i < BUFSIZ-1; i++) szBuf[i] = szBuf[i+1];
			index = BUFSIZ -2;
		}
#ifdef DEBUG_LOG
		// continue to read more data!
		Serial.print(c);
		pclient->print(c);
#endif
		//continue;
	}
#ifdef DEBUG
	SerPrintP("Bailing out with :"); Serial.println(szBuf);
#endif
}


void APDWeb::processProvisioningRequest(EthernetClient *pclient) {
	uint8_t uProv=0;
	unsigned int bytesProv = 0;
	unsigned int bytesProvSaved = 0;
	if (pclient != NULL) {
#ifdef DEBUG
		SerPrintP("PROVISIONING.");
#endif
		char clientline[BUFSIZ];
		char destfile[14]="";
		int index = 0;
		clientline[index]=0;
		clientline[BUFSIZ-1]=0;

		web_startpage(pclient,"provisioning");
#ifdef DEBUG
		if (pAPDStorage != NULL) {
			SerPrintP("STORAGE AVAILABLE");
		} else {
			SerPrintP("STORAGE NOT AVAILABLE");
		}
#endif
#ifdef DEBUG
		// send back what we received - for debug
		WCPrintP(pclient, "<hr />\n");
		client.println(clientline);
		WCPrintP(pclient, "<hr />\n");
#endif

		while (pclient->available()) {

			forwardToMarker(pclient,clientline,"destination=");		// TODO destination should go to PROGMEM

			if (strstr_P(clientline, PSTR("destination="))) {
				forwardToMarker(pclient,clientline,"&");				// reads to the next param (joined with &)

				if (strstr(clientline, "&")) {
					*strstr(clientline, "&") = 0;
#ifdef DEBUG_LOG
					WCPrintP(pclient, "<hr/><b>DESTINATION</b>=\n");
					pclient->println(clientline);
#endif
					strcpy(destfile,clientline);										// save the dest file name
#ifdef DEBUG_LOG
					WCPrintP(pclient, "<hr/>\n");
#endif
				}

				// forward to the data portion
				forwardToMarker(pclient,clientline,"data=");

				if (strstr(clientline, "data=")) {
#ifdef DEBUG
					SerPrintP("Processing Data");
#endif
					// remove temp file if exists
					if (pAPDStorage->p_sd->exists("PROV.TMP")) pAPDStorage->p_sd->remove("PROV.TMP");
					SdFile tempFile("PROV.TMP", O_WRITE | O_CREAT );

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
#ifdef DEBUG
								SerPrintP("HEX CODE:"); Serial.println(hexcode);
#endif
								int cc = read_hex_byte(hexcode);
#ifdef DEBUG
								Serial.print(cc);
#endif
								char newc = cc;
#ifdef DEBUG
								SerPrintP("Translates to: '"); Serial.print(newc); SerPrintP("'");
#endif
								if (newc == '&') {
									bytesProv++;
									if (tempFile.isOpen()) {
										tempFile.write(newc);      // we print this character early to file
										bytesProvSaved++;
									}
#ifdef DEBUG_INFO
									Serial.print(c);
									pclient->print(c);
#endif

									newc = 1;      // hack / & would stop the checks/
								}
								c = newc;
							} else {
								SerPrintP("E22\n");
							}
						}

						if (c != 1 && c!= '&') {      // if not & /does not belong to data anymore/ or ascii(1) /we made it so, no other nonprintables expected/
							bytesProv++;
							if (tempFile.isOpen()) {
								tempFile.write(c);    // write the char to the tempfile
								bytesProvSaved++;
							}
#ifdef DEBUG_INFO
							Serial.print(c);
							pclient->print(c);
#endif
						}
						clientline[index] = c;
						index++;
						clientline[index]=0;
						// are we too big for the buffer? shift left
						if (index >= BUFSIZ-1) {
							for (int i=0; i < BUFSIZ -1; i++) clientline[i] = clientline[i+1];
							index = BUFSIZ -2;
						}

						// TEMPFILE SHOULD BE VALIDATED and RENAMED to DESTINATION
#ifdef DEBUG
						// continue to read more data!
						Serial.print(c);
						pclient->print(c);
						//continue;
#endif
					}

					if (tempFile.isOpen()) tempFile.close(); // either no more bytes or & reached, anyways, file is to be closed at this point
					if (strlen(destfile) > 0) {
#ifdef DEBUG_LOG
						SerPrintP("renaming file to "); Serial.println(destfile);
#endif
						if (pAPDStorage->p_sd->exists(destfile)) pAPDStorage->p_sd->remove(destfile);      // TODO add backup
						pAPDStorage->p_sd->rename("PROV.TMP",destfile);
#ifdef DEBUG_LOG
						WCPrintP(pclient, "<b>OK</b>");
#endif
						uProv++;
					}

					if (strstr(clientline, "&")) {
						*strstr(clientline, "&") = 0;
#ifdef DEBUG_LOG
						WCPrintP(pclient, "<hr/><b>DATA</b>=");
						pclient->print(clientline);
						WCPrintP(pclient, "<hr/>\n");
#endif
#ifdef DEBUG
						SerPrintP("DATA:"); Serial.println(clientline);
#endif
					}

				}

			} else {	// invalid request or end of provisioning
				if (uProv<1) WCPrintP(pclient, "INVALID REQUEST\n");
			}

		} 	// while client has available bytes

		WCPrintP(pclient, "<hr />\n");
		WCPrintP(pclient, "Received "); pclient->print(bytesProv); WCPrintP(pclient, " bytes, provisioned "); pclient->print(bytesProv); WCPrintP(pclient," bytes into "); pclient->print(uProv); WCPrintP(pclient," files.\n");
		if (bytesProv != bytesProvSaved) {
			WCPrintP(pclient, "<div class=\"error\">Corrupted provisioning!?</div>");
		}
		// TODO a callback to apduino online would be nice
		// TODO or a redirect
		web_endpage(pclient);
	}
}



void APDWeb::registration_response(APDWeb *pAPDWeb){
	bool bReReg = false;
	delay(500);    // debug
	int content_length = -1;
	char new_api_key[65]="";
	if (pAPDWeb->pwwwclient != NULL && pAPDWeb->pwwwclient->available()) {
#ifdef DEBUG
		SerPrintP("SR: Processing server response...");
#endif
		while (pAPDWeb->pwwwclient->available()) {    // with bytes to read
			//      char c = pAPDWeb->pwwwclient->read();        // then read a byte
			//#ifdef DEBUG
			//      Serial.print(c);
			//#endif
			char www_respline[BUFSIZ] ="";
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
						if (index >= BUFSIZ-1) {           // are we too big for the buffer? shift left
							for (int i=0; i < BUFSIZ-1; i++) www_respline[i] = www_respline[i+1];
							index = BUFSIZ -2;
						}
					}
				}

				// a line in www_respline
#ifdef DEBUG
				SerPrintP("LINE2CHK: "); Serial.println(www_respline);
#endif
				if (!bProcessingBody) {      // if fetching lines of the HTTP header
					if (iStatusCode < 0) {          // if no status code yet
						sscanf(www_respline,"Status: %d",&iStatusCode);    // scan for status
					} else {                       // if status already found
						SerPrintP("STATUS : "); Serial.println(iStatusCode);
						if (www_respline[0] == '\n') {    // if blank line (separating header and body)
							bProcessingBody = true;            // now comes the body part
						}
					}
				} else {                  // if fetching lines from the HTTP body
					if (content_length < 0) {  // we are in the body but no length yet (1st row)
						content_length = atoi(www_respline);
#ifdef DEBUG
						SerPrintP("content_length:"); Serial.println(content_length);
#endif
					} else if (new_api_key[0]==0) {
						if (sscanf(www_respline,"REG_API_KEY=%s",new_api_key)) {
							if (pAPDWeb->szAPDUINO_API_KEY[0]==0) {
								strcpy(pAPDWeb->szAPDUINO_API_KEY,new_api_key);

								// TODO save API key for host!
								pAPDWeb->saveAPIkey(pAPDWeb->szAPDUINO_API_KEY, "APIKEY.CFG");
							  bReReg = true;

							  SerPrintP("Registered your device on APDuino Online.\nClaim your device at: http://");
								Serial.print(pAPDWeb->apduino_server_name);
								SerPrintP("/devices/claim_device?api_key=");
								Serial.println(pAPDWeb->szAPDUINO_API_KEY);
							}
#ifdef DEBUG
							else {
								SerPrintP("API key present already.\n");
							}
#endif
						} else {

						}
					} else {
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
		if (bReReg) {
			SerPrintP("Confirming registration.\n");
			pAPDWeb->self_register();          // re-run registration with the new API key so server creates the device
		}
	}
}

// TODO  add 'claim device' functionality:
// -> once registerdevice but no deviceid, offer link with API_KEY to claim device. it should result a deviceid


boolean APDWeb::self_register() {
	char www_postdata[96];
	if ( pwwwclient!=NULL ) {
		if ( !pwwwclient->connected() ) {
			sprintf(www_postdata,"lan_ip=%d.%d.%d.%d&v=%s.%s",net.ip[0],net.ip[1],net.ip[2],net.ip[3],APDUINO_VERSION,APDUINO_BUILD);
#ifdef DEBUG
			SerPrintP("SR: "); Serial.print(www_postdata); SerPrintP(" ...");
#endif
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
				//pwwwclient->println();

				// here's the actual content of the PUT request:
				pwwwclient->println(www_postdata);
				SerPrintP("Request sent.");
				if (pwwwcp ==NULL)
					pwwwcp = (&registration_response);      // set reader 'callback'
//#ifdef DEBUG
				else {
					SerPrintP("E23");
				}
//#endif
			} else {
#ifdef DEBUG
				SerPrintP("E24");
#endif
				pwwwclient->stop();
			}
		} else {
			//#ifdef DEBUG
			SerPrintP("E25");
			this->wc_busy();
			//#endif
		}
		bWebClient = (pwwwclient!=0) && pwwwclient->connected();
#ifdef DEBUG
		SerPrintP("WWWCLI: "); Serial.println(bWebClient,DEC); SerPrintP("WWCLI?"); Serial.println((int)pwwwclient,DEC);
#endif
	} else {
		//#ifdef DEBUG
		SerPrintP("E26");
		this->failure();
		//#endif
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
		//SerPrintP("WCLOOP\n");
		loop_webclient();
		if (!pwwwclient->connected()) {           // if no web client is active
			if ((this->operational_state & OPSTATE_STARTED) && !(this->operational_state & OPSTATE_PAUSED)) {
				iBusyCounter = 0;
				if (this->iSensorCount>0 || this->iControlCount>0) {	// TODO fix this quick hack to see properly if there is anything to log
					if (this->pmetro != NULL && this->pmetro->check()) {
						SerPrintP("\nWEBLOG\n");
						this->web_logging();
						this->pmetro->reset();
					} else if ( this->phmetro != NULL && this->phmetro->check()) {
						SerPrintP("\nCOSMLOG\n");
						this->pachube_logging();
						this->phmetro->reset();
					} else if ( this->tsmetro != NULL && this->tsmetro->check()) {
						SerPrintP("\nTHINGSPEAK LOG\n");
						this->thingspeak_logging();
						this->tsmetro->reset();
					}
				}
			} else {
				// SerPrintP("Paused.");
			}
		} else {
			//SerPrintP("WC busy...\n");
			this->wc_busy();
		}
	} else {
		SerPrintP("No webclient class?!\n");
		this->failure();
	}
	if (pwwwserver != NULL) {
		//SerPrintP("WSLOOP\n");
		loop_server();
	}
	if (bRestart) {
		SerPrintP("restart requested\n");
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
	char ts[] = "1970/01/01 00:00:00";        // string used for timestamp
	if (this->pAPDTime != NULL) this->pAPDTime->nowS(ts);                 // pull correct time
	char *pcLog = szLogBuf;
	char dataString[16]="";                // make a string for assembling the data to log:

	strcpy(pcLog,"datarow=");
	pcLog+=8;
	strcpy(pcLog,ts);
	pcLog+=strlen(ts);
	for (int i=0;i<iSensorCount;i++) {
		if (pAPDSensors[i]->config.sensor_log) {          // if sensor to be logged
			//strcpy(pcLog,pAPDSensors[i]->config.label);
			//pcLog+=strlen(pAPDSensors[i]->config.label);
			*pcLog=','; pcLog++;// *pcLog = '\0';
			pAPDSensors[i]->getValueS(dataString);
			strcpy(pcLog,dataString);
			pcLog+=strlen(dataString);
		}
	}
	*pcLog='\n'; pcLog++; *pcLog='\0'; // \n\0
}


// TODO: add size control, avoid writing to random places
void APDWeb::get_pachubelog_string(char *szLogBuf) {
	char *pcLog = szLogBuf;
	char dataString[16]="";                // make a string for assembling the data to log:

	for (int i=0;i<iSensorCount;i++) {
		if (pAPDSensors[i]->config.sensor_log) {          // if sensor to be logged
			strcpy(pcLog,pAPDSensors[i]->config.label);
			pcLog+=strlen(pAPDSensors[i]->config.label);
			*pcLog=','; pcLog++;// *pcLog = '\0';
			pAPDSensors[i]->getValueS(dataString);
			strcpy(pcLog,dataString);
			pcLog+=strlen(dataString);
			*pcLog='\n'; pcLog++;
		}
	}
	for (int i=0;i<iControlCount;i++) {
		if (pAPDControls[i]->config.control_log) {          // if control to be logged
			strcpy(pcLog,pAPDControls[i]->config.label);
			pcLog+=strlen(pAPDControls[i]->config.label);
			*pcLog=','; pcLog++;// *pcLog = '\0';
			pAPDControls[i]->getValueS(dataString);
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

	for (int i=0;i<iSensorCount;i++) {
		if (pAPDSensors[i]->config.sensor_log) {          // if sensor to be logged
			char szFN[16]="";
			uSens++;
			sprintf_P(szFN,PSTR("field%d"),uSens);
			if (pcLog > szLogBuf) {
				*pcLog='&'; pcLog++;
			}
			//strcpy(pcLog,pAPDSensors[i]->config.label);
			//pcLog+=strlen(pAPDSensors[i]->config.label);
			strcpy(pcLog,szFN);
		  pcLog+=strlen(szFN);
			*pcLog='='; pcLog++;// *pcLog = '\0';
			pAPDSensors[i]->getValueS(dataString);
			strcpy(pcLog,dataString);
			pcLog+=strlen(dataString);
			//*pcLog='\n'; pcLog++;
		}
	}
	*pcLog='\n'; pcLog++; *pcLog='\0'; // \n\0
}


// requires Ethernet connection to be started already
void APDWeb::web_logging() {
	char www_logdata[512];
	if ( pwwwclient ) {           // TODO check if we're registered
		if ( !pwwwclient->connected() ) {
			get_lastlog_string(www_logdata);
#ifdef DEBUG
			SerPrintP("WL: "); Serial.print(this->pstr_APDUINO_API_KEY); SerPrintP(" - "); Serial.print(www_logdata); SerPrintP(" ...");
#endif
			if( pwwwclient->connect(apduino_server_ip, apduino_server_port) ) {

				SerPrintP("connecting...");
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
				int thisLength = strlen(www_logdata);
				pwwwclient->println(thisLength);

				// last pieces of the HTTP PUT request:
				WCPrintP(pwwwclient,"Connection: close\n");
				pwwwclient->println();

				// here's the actual content of the PUT request:
				pwwwclient->println(www_logdata);

			} else {
#ifdef DEBUG
				SerPrintP("SR: failed to connect.\n");
#endif
				pwwwclient->stop();          // stop client now
				this->failure();
			}
		}
	}	else {
		SerPrintP("E27");
		this->failure();
	}

	bWebClient = (pwwwclient!=0) && pwwwclient->connected();
}

/*
void APDWeb::setupPachubeDataStream(char *szDataStream,float value) {
  return ;
  if ( pwwwclient ) {           // TODO check if we're registered
      SerPrintP("PHS SETUP----");
    if ( !pwwwclient->connected() ) {
        SerPrintP("progressing----");
      char feedUrl[64] = "";
      char www_logdata[64];
      SerPrintP("ASSEMBLING PH LOG\n");
      sprintf(www_logdata,"%s,%f",szDataStream,value);

      sprintf(feedUrl,"/v2/feeds/%lu/datastreams/%s",cosm_feed_id,szDataStream);

      if( pwwwclient->connect(cosm_server_ip, cosm_server_port) ) {
//#ifdef DEBUG
        SerPrintP("Pachube connect: http://"); Serial.print(cosm_server_name); Serial.print(feedUrl); SerPrintP("\n");
//#endif
        Serial.println(www_logdata);
        SerPrintP("----------------");
        // send the HTTP PUT request:
        WCPrintP(pwwwclient,"POST "); pwwwclient->print(feedUrl); WCPrintP(pwwwclient," HTTP/1.1\n");
        WCPrintP(pwwwclient,"Host: ");    pwwwclient->println(cosm_server_name);
        // TODO fix api key
        WCPrintP(pwwwclient,"X-PachubeApiKey: ");   pwwwclient->println(szCOSM_API_KEY);
        WCPrintP(pwwwclient,"User-Agent: "); pwwwclient->println(USERAGENT);
        //pwwwclient->println("Content-Type: application/x-www-form-urlencoded");
        WCPrintP(pwwwclient,"Content-Type: text/csv;\n");
        WCPrintP(pwwwclient,"Accept: text/csv;\n");
        WCPrintP(pwwwclient,"Content-Length: ");

        // calculate the length of the sensor reading in bytes:
        // 8 bytes for "sensor1," + number of digits of the data:
        int thisLength = strlen(www_logdata);
        pwwwclient->println(thisLength);

        // last pieces of the HTTP PUT request:
        //pwwwclient->println("Connection: close");
        pwwwclient->println();

        // here's the actual content of the PUT request:
        pwwwclient->println(www_logdata);
        SerPrintP("Fetching response...\n");
        while(pwwwclient->connected()) {
            while (pwwwclient->available()) {    // with bytes to read
              char c = pwwwclient->read();        // then read a byte
        //#ifdef DEBUG
              Serial.print(c);
        //#endif
            }
        }

          // if the server's disconnected, stop the client:
          if (!pwwwclient->connected()) {
            pwwwclient->stop();
            bWebClient = false;
          }
      }

    } else {
//#ifdef DEBUG
      SerPrintP("Pachube: failed to connect.\n");
//#endif
      pwwwclient->stop();          // stop client now
    }
  } else {
      SerPrintP("NO NETWORK??");
  }
  bWebClient = (pwwwclient!=0) && pwwwclient->connected();
}
*/


void APDWeb::pachube_logging() {
	if ( pwwwclient ) {           // TODO check if we're registered
		if ( !pwwwclient->connected() ) {
			char feedUrl[64] = "";
			char www_logdata[512];
			SerPrintP("ASSEMBLING PH LOG\n");
			get_pachubelog_string(www_logdata);

			sprintf(feedUrl,"/v2/feeds/%lu.csv",cosm_feed_id);

			Serial.println(www_logdata);
			SerPrintP("\n\nconn:"); Serial.print(cosm_server_name); Serial.println(feedUrl);

			if( pwwwclient->connect(cosm_server_ip, cosm_server_port) ) {
				SerPrintP("connected. sending...");

				// send the HTTP PUT request:
				WCPrintP(pwwwclient,"PUT "); pwwwclient->print(feedUrl); WCPrintP(pwwwclient," HTTP/1.1\n");
				WCPrintP(pwwwclient,"Host: ");    pwwwclient->println(cosm_server_name);
				// TODO fix api key
				WCPrintP(pwwwclient,"X-PachubeApiKey: ");   pwwwclient->println(szCOSM_API_KEY);
				WCPrintP(pwwwclient,"User-Agent: "); pwwwclient->println(USERAGENT);
				//pwwwclient->println("Content-Type: application/x-www-form-urlencoded");
				WCPrintP(pwwwclient,"Content-Type: text/csv\n");
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
				SerPrintP("OK.\n");
			} else {
				//#ifdef DEBUG
				SerPrintP("E29");
				//#endif

				pwwwclient->stop();          // stop client now
				this->failure();
			}
		}
	}	else {
		SerPrintP("E28");
		this->failure();
	}

	bWebClient = (pwwwclient!=0) && pwwwclient->connected();
}


void APDWeb::thingspeak_logging() {
	if ( pwwwclient ) {           // TODO check if we're registered
		if ( !pwwwclient->connected() ) {
			char feedUrl[64] = "";
			char www_logdata[512];
#ifdef DEBUG
			SerPrintP("ASSEMBLING TS LOG\n");
#endif
			get_thingspeaklog_string(www_logdata);

			Serial.println(www_logdata);
#ifdef DEBUG
			SerPrintP("\n\nconn:"); Serial.print(thingspeak_server_name); //Serial.println(feedUrl);
#endif
			if( pwwwclient->connect(thingspeak_server_ip, thingspeak_server_port) ) {
#ifdef DEBUG
				SerPrintP("connected. sending...");
#endif
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
				SerPrintP("done.\n");
			} else {
				//#ifdef DEBUG
				SerPrintP("E291.\n");
				//#endif
				pwwwclient->stop();          // stop client now
				this->failure();
			}
		}
	}
#ifdef DEBUG
	else {
		SerPrintP("E292");
	}
#endif
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
#ifdef DEBUG
			SerPrintP("WEBCLIENT GOT SOMETHING!");
#endif
			if (pwwwcp != NULL) {
#ifdef DEBUG
				SerPrintP("CALL WEBCLIENT PROCESSOR!\n");
#endif
				(*pwwwcp)(this);    // call processor
				// TODO: commented out the line below that enforced that the wwwprocessor function is reset. make sure the processors reset themselves!
				//pwwwcp = NULL;  // no more processor ;-)
			}

			// read anything remaining
#ifdef DEBUG
			SerPrintP("Processing leftovers...\n");
#endif

			while (pwwwclient->available()) {    // with bytes to read
				char c = pwwwclient->read();        // then read a byte
#ifdef DEBUG
				Serial.print(c);
#endif
			}}

		// if the server's disconnected, stop the client:
		if (!pwwwclient->connected()) {
#ifdef DEBUG
			SerPrintP("LOOP WEBCLIENT disconnecting.\n");
#endif
			pwwwclient->stop();
			bWebClient = false;
		}
	}
#ifdef DEBUG
	else {
		SerPrintP("WL: error, no web client.\n");
	}
#endif
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
			SerPrintP("E201("); Serial.print(ilen,DEC); SerPrintP(")");
		}
	}
}


void APDWeb::new_apduinoconf_parser(void *pAPDWeb, int iline, char *psz) {
	APDWeb *pw = (APDWeb*)pAPDWeb;
	char szhost[32];
	unsigned long uLogFreq = DEFAULT_ONLINE_LOG_FREQ;
#ifdef DEBUG
	Serial.print("ONLINE READ: "); Serial.print(psz);
#endif
	//        hostname  |IP4     |Port|feedid
	sscanf( psz, "%s %2x%2x%2x%2x,%d,%lu",
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
	sscanf( psz, "%s %2x%2x%2x%2x,%d,%lu,%lu",
			szhost,
			&(pw->cosm_server_ip[0]),&(pw->cosm_server_ip[1]),&(pw->cosm_server_ip[2]),&(pw->cosm_server_ip[3]),
			&(pw->cosm_server_port),
			&(pw->cosm_feed_id),
			&(pw->cosm_logging_freq));

	strncpy(pw->cosm_server_name,szhost,31);
#ifdef DEBUG
	Serial.println(szhost); Serial.println(pw->cosm_server_name);
	Serial.println(pw->cosm_feed_id);
	Serial.println(pw->cosm_logging_freq);
#endif
	// Cosm config should be in APDWeb now
}

void APDWeb::new_thingspeakconf_parser(void *pAPDWeb, int iline, char *psz) {
	APDWeb *pw = (APDWeb*)pAPDWeb;
	char szhost[32];
#ifdef DEBUG
	Serial.print("THINGSPEAK READ: "); Serial.print(psz);
#endif
	//        hostname  |IP4     |Port|feedid
	sscanf( psz, "%s %2x%2x%2x%2x,%d,%lu,%lu",
			szhost,
			&(pw->thingspeak_server_ip[0]),&(pw->thingspeak_server_ip[1]),&(pw->thingspeak_server_ip[2]),&(pw->thingspeak_server_ip[3]),
			&(pw->thingspeak_server_port),
			&(pw->thingspeak_logging_freq));

	strncpy(pw->thingspeak_server_name,szhost,31);
#ifdef DEBUG
	Serial.println(szhost); Serial.println(pw->thingspeak_server_name);
	Serial.println(pw->thingspeak_logging_freq);
#endif
	// TS config now should be in APDWeb...
}


void APDWeb::dumpPachube() {
	if (pAPDStorage->p_sd->exists("PACHUBE.CFG")) {
		pAPDStorage->p_sd->remove("PACHUBE.CFG");
	}
	SdFile dataFile("PACHUBE.CFG", O_WRITE | O_CREAT );
	if (dataFile.isOpen()) {
		char line[BUFSIZ]="";
		// TODO update with recent fields
		sprintf(line,"%s %2x%2x%2x%2x,%d,%lu,%lu",
				this->cosm_server_name,
				(this->cosm_server_ip[0]),(this->cosm_server_ip[1]),(this->cosm_server_ip[2]),(this->cosm_server_ip[3]),
				(this->cosm_server_port),
				(this->cosm_feed_id),
				(this->cosm_logging_freq));
		dataFile.println(line);

		dataFile.close();
#ifdef DEBUG
		SerPrintP("Pachube Config dumped.");
#endif
	} else {
		// TODO add an error macro in storage, replace all error opening stuff with reference to that
		SerPrintP("W280");
	}
	saveAPIkey(szCOSM_API_KEY,"PACHUBE.KEY");
}


void APDWeb::json_header(EthernetClient *pClient) {
	WCPrintP(pClient,"HTTP/1.1 200 OK\n");
	WCPrintP(pClient,"Content-Type: application/json\n\n");
	delay(1);
}

void APDWeb::json_status(EthernetClient *pClient) {
	if (*pClient) {
		char tbuf[20] ="";

		WCPrintP(pClient,"[");

		WCPrintP(pClient,"{ \"name\": \"sensors\", \"data\": [");

		//    SerPrintP("Output sensors...");
		for (int i = 0; i < iSensorCount; i++) {
			if (i>0) {
				WCPrintP(pClient,",\n");
			}
			WCPrintP(pClient,"{ \"Index\":\"");
			pClient->print(i); WCPrintP(pClient,"\",\n");
		    WCPrintP(pClient,"\"Name\":\"");
			pClient->print((char *)pAPDSensors[i]->config.label);
			WCPrintP(pClient,"\",\n\"Value\":\"");
			pAPDSensors[i]->getValueS(tbuf);
			pClient->print(tbuf);
			WCPrintP(pClient,"\",\n\"Logged\":\"");
			//WCPrintP(pClient, (pAPDSensors[i]->config.sensor_log ? "true" : "false"));
			if (pAPDSensors[i]->config.sensor_log) {
				//WCPrintP(pClient," id=\"chart-"); pClient->print((char *)pAPDSensors[i]->config.label); WCPrintP(pClient,"\"");
				WCPrintP(pClient, "true");
			} else {
				WCPrintP(pClient, "false");
			}
			WCPrintP(pClient,"\"}");			// End Sensor
			delay(1);
		}

		WCPrintP(pClient,"] },\n");  // End Sensors data Array, Sensors

		WCPrintP(pClient,"{ \"name\": \"controls\", \"data\": [");

		for (int i = 0; i < iControlCount; i++) {
			if (i>0) {
				WCPrintP(pClient,",\n");
			}
			WCPrintP(pClient,"{ \"Index\":\"");
			pClient->print(i); WCPrintP(pClient,"\",\n");
		    WCPrintP(pClient,"\"Name\":\"");
			pClient->print((char *)pAPDControls[i]->config.label);
			WCPrintP(pClient,"\",\n\"Value\":\"");
			pAPDControls[i]->getValueS(tbuf);		// get control value
			pClient->print(tbuf);
			WCPrintP(pClient,"\",\n\"Logged\":\"");

			if (pAPDControls[i]->config.control_log) {
//				WCPrintP(pClient," id=\"chart-"); pClient->print((char *)pAPDControls[i]->config.label); WCPrintP(pClient,"\"");
				WCPrintP(pClient, "true");
			} else {
				WCPrintP(pClient, "false");
			}
			WCPrintP(pClient,"\"}");			// End Control
			delay(1);
		}

		WCPrintP(pClient,"] },\n");		// End Controls data Array, Controls

		WCPrintP(pClient,"{ \"name\": \"rules\", \"data\": [");

		for (int i = 0; i < iRuleCount; i++) {
			if (i>0) {
				WCPrintP(pClient,",\n");
			}
			WCPrintP(pClient,"{ \"Index\":\"");
			pClient->print(i); WCPrintP(pClient,"\",\n");
		    WCPrintP(pClient,"\"Name\":\"");
			pClient->print((char *)pAPDRules[i]->config.label);
			WCPrintP(pClient,"\",\n\"Value\":\"");
			pAPDRules[i]->getValueS(tbuf);		// get control value
			pClient->print(tbuf);
			WCPrintP(pClient,"\",\n\"Logged\":\"");

//			if (pAPDRules[i]->config.control_log) {
////				WCPrintP(pClient," id=\"chart-"); pClient->print((char *)pAPDRules[i]->config.label); WCPrintP(pClient,"\"");
//				WCPrintP(pClient, "true");
//			} else {
				WCPrintP(pClient, "false");
//			}
			WCPrintP(pClient,"\"}");	// End Rule
			delay(1);
		}

		WCPrintP(pClient,"] }\n");	// End Rules data Array, Rules


		WCPrintP(pClient,"]");		// End Main Array
		// give the web browser time to receive the data
		delay(1);
		SerPrintP("JSONDONE.");
	} else {
		SerPrintP("W02");
	}
}
