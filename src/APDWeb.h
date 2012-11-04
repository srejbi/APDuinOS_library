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
 * APDWeb.h
 *
 *  Created on: Mar 28, 2012
 *      Author: George Schreiber
 *
 * Based on http://code.google.com/p/arduino/issues/detail?id=742
 * this file requires Ethernet/DHCP patched with https://github.com/arduino/Arduino/commit/b8fbffeac47e285bd68db80a0d5e716d8997df42
 * in order that a proper hostname is set in the DHCP request.
 */

#ifndef APDWEB_H_
#define APDWEB_H_

#include <Arduino.h>
#include <SPI.h>                 // SPI comms
//#include <Base64.h>
#include <Ethernet.h>            // ethernet shield
#include <MemoryFree.h>
#include <Metro.h>
#include "APDSensor.h"
#include "APDControl.h"
#include "APDRule.h"
#include "APDStorage.h"
#include "APDSerial.h"
#include "APDTime.h"
#include "APDLogWriter.h"

const char USERAGENT[] = "APDuinOS";
const char WEBLOG_URI[]="/devices/lastlog";      // see apduino online specs.
//const char COSMLOG_URI[]="";                 // implement Patchube logging later as an option

#define APDUINO_SERVER PSTR("apduino.com")
#define APDUINO_SERVER_IP (byte []){204,12,228,115}
#define DEFAULT_ONLINE_LOG_FREQ		60000

#define COSM_SERVER		PSTR("api.cosm.com")
#define COSM_SERVER_IP (byte []){216,52,233,122}

#define DEFAULT_TIMESERVER_IP (byte []){193,225,14,181}

#define WEBCLIENT_BUSY_TIMEOUT_MS				30000
#define MAX_WEBCLIENT_BUSY_LOOPS			  10000
#define MAX_NET_FAILURES                3

// operation states
#define OPSTATE_BLANK			0
#define OPSTATE_CONFIGURED		1
#define OPSTATE_STARTED			2
#define OPSTATE_PAUSED			4
#define OPSTATE_ERROR       128

// dispatched requests
#define DREQ_NOOP					0
#define DREQ_RECONF				1
#define DREQ_RESET				2
#define DREQ_RELOADRULES  3

// content types (used in header function)
#define CONTENT_TYPE_TEXT				0
#define CONTENT_TYPE_HTML   		1
#define CONTENT_TYPE_JSON   		2
#define CONTENT_TYPE_JAVASCRIPT 3
#define CONTENT_TYPE_CSS				4
#define CONTENT_TYPE_PNG				10
#define CONTENT_TYPE_JPG				11
#define CONTENT_TYPE_GIF				12

struct NETCONF {
  byte mac[6];            //TODO generate random mac, more than 1 APDuino can have collisions
  byte ip[4];                          // ip address
  byte gateway[4];                     // gateway
  byte subnet[4];                      // subnet mask
  byte pridns[4];                      // primary dns
  unsigned int localPort;                       // local port to listen for UDP packets (NTP communications)
  unsigned int wwwPort;                           // local port to listen on with WWW server
};

class APDWeb
{
public:
  APDWeb();
  APDWeb(NETCONF *pnc);
  virtual
  ~APDWeb();

  //void startWebServer(APDSensor **pSensors, int iSensorCount, APDControl **pControls, int iControlCount, APDRule **pRules, int iRuleCount, APDStorage *pAPDStorage);
  void startWebServer(APDSensor **pSensors, int iSensorCount, APDControl **pControls, int iControlCount, APDRule **pRules, int iRuleCount);
  void loop();

  bool pause_service();
  bool continue_service();

  // static configuration parsers (callbacks, sort of)
  static void new_apduinoconf_parser(void *pAPDWeb, int iline, char *psz);
  static void new_cosmconf_parser(void *pAPDWeb, int iline, char *psz);
  static void new_thingspeakconf_parser(void *pAPDWeb, int iline, char *psz);

  // static www-client response processor
  static void registration_response(APDWeb *pAPDWeb);

private:
  // private data members
  NETCONF net;
  int operational_state;					// mask with OPSTATES
  int dispatched_requests;				// pass requests to APDuino (APDuino should read and process this regularly)

  EthernetServer *pwwwserver;
  EthernetClient *pwwwclient;
  unsigned int uCCount;                     // count WWW clients
  void (*pwwwcp)(APDWeb *);                 // pointer to the actual web client processor (depending on what request was made, a reader can be assigned to process server resp. if we care)
  boolean bWebClient;												// if a client is known to be connected

  boolean bEthConfigured;                   // will be true if we have an eth connection (DHCP or static)

  //APDTime *pAPDTime;                        // timekeeping; will receive ptr. no need to free
  //APDStorage *pAPDStorage;									// pointer to storage

  APDSensor **pAPDSensors;
  int iSensorCount;
  APDControl **pAPDControls;
  int iControlCount;
  APDRule **pAPDRules;
  int iRuleCount;

  boolean bDHCP;                          // if DHCP was used

  // failure detection & repair
  int iFailureCount;										// count detected failures
  int iRestartCount;										// count restarts
  bool bRestart;												// set to true to restart
  int iBusyCounter;										  // count busy loops
  unsigned long wcb_millis;						  // measure the time from the 1st busy loop

  // online loggers
  byte apduino_server_ip[4];           // apduino.localhost -- test server on LAN
  char apduino_server_name[32];        // test APDuino server on LAN
  unsigned int apduino_server_port;    // standard HTTP port
  unsigned long apduino_logging_freq;		// how often to log (ms)

  byte cosm_server_ip[4];            // api.cosm.com 64.94.18.121
  char cosm_server_name[32];         // api.cosm.com
  unsigned int cosm_server_port;     // standard HTTP port
  unsigned long cosm_feed_id;         // feed id
  unsigned long cosm_logging_freq;    // logging freq

  byte thingspeak_server_ip[4];            // api.thingspeak.com  184.106.153.149
  char thingspeak_server_name[32];         // api.thingspeak.com
  unsigned int thingspeak_server_port;     // standard HTTP port
  unsigned long thingspeak_logging_freq;    // logging freq

  // API keys
  char szAPDUINO_API_KEY[65];               // the api key for apduino online
  char szCOSM_API_KEY[65];               // the api key for Pachube
  char szTHINGSPEAK_API_KEY[65];               // the api key for ThingSpeak

  // Metros for loggers
  Metro *pmetro;                         // weblog metro
  Metro *phmetro;                        // pachube logging metro
  Metro *tsmetro;                        // thingspeak logging metro


  // private methods

  // error-detection and recovery attempt
  void failure();
  void wc_busy();

  // www-processing helpers
  boolean basicAuthorize(EthernetClient *pclient);
  void sendAuthRequest(EthernetClient *pclient, const char *szrealm);
  void forwardToMarker(EthernetClient *pclient, char *szBuf, char *szMarker);

  // log string generators
  void get_lastlog_string(char *szLogBuf);
  void get_cosmlog_string(char *szLogBuf);
  void get_thingspeaklog_string(char *szLogBuf);

  // initialization
  void initBlank();
  boolean start();
  boolean restart();

  // configuration
  void log_to_ApduinoOnline();
  void log_to_Cosm();
  void log_to_ThingSpeak();

  boolean self_register();
  boolean setup_webclient();

  boolean setupAPDuinoOnline();
  boolean startWebLogging(unsigned long uWebLoggingFreq);
  boolean setupCosmLogging();
  boolean setupThingSpeakLogging();

  void dumpPachube();

  boolean loadAPIkey(char *szAPIKey, const char *szAPIFile);
  void saveAPIkey(const char *szAPIKey, const char *szAPIFile);

  // helpers
  static void myCPrintP(EthernetClient *pClient, void *Pstring);

  // control & processing
  void loop_webclient();
  void loop_server();

  // www helpers
  //static void web_header(EthernetClient *pClient);
  void web_startpage(EthernetClient *pClient, char *title,int refresh);
  void web_endpage(EthernetClient *pClient);
  static void webstatus_table_item(EthernetClient *pClient, const char *group, const int index, const char *name, const char *value, const char *logged );
  void web_status(EthernetClient *pClient);
  void web_maintenance(EthernetClient *pClient);
  static void web_notfound(EthernetClient *pClient);
  void ListFiles(EthernetClient client, const char *szPath, uint8_t flags);
  bool ServeFile(EthernetClient client, const char *szPath);
  void processProvisioningRequest(EthernetClient *pclient, boolean brespond);
  void claim_device_link(EthernetClient *pClient);

  static void header(EthernetClient *pClient, int content_type);

  static void json_array_item(EthernetClient *pClient, const int index, const char *name, const char *value, const char *logged );
  //static void json_header(EthernetClient *pClient);
  void json_status(EthernetClient *pClient);

  friend class APDuino;
};

#endif /* APDWEB_H_ */
