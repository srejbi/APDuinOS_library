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
 * apd_msg_codes.h
 *
 *  Created on: Oct 2, 2012
 *      Author: George Schreiber
 */

#ifndef APD_MSG_CODES_H_
#define APD_MSG_CODES_H_

// MESSAGES
#define APDUINO_MSG_SERIAL_INIT		        0x0101	// Serial port initialized
#define APDUINO_MSG_ENABLERULEPROC        0x0102	// Enabled rule processing
#define APDUINO_MSG_SDLOGOK					      0x0103	// Logging to SD prepared
#define APDUINO_MSG_TIMESTAMP		        	0x0104	// Current system timestamp: %s
#define APDUINO_MSG_SDLOGGING             0x0105	// Logging to SD card %d chars
#define APDUINO_MSG_SDLOGRESCHED			    0x0106	// SD logging interval reconfigured

#define APDUINO_MSG_ETHERNETFROMCONF      0x0105	// Initializing ethernet with configuration from file
#define APDUINO_MSG_NETINIT               0x0106	// Initializing ethernet
#define APDUINO_MSG_DHCPFALLBACK          0x0107 	// Fallback to DHCP

#define APDUINO_MSG_STORAGEINIT           0x0110	// Initializing Storage
//#define APDUINO_MSG_STORAGESTART          0x0111	// Starting Storage
#define APDUINO_MSG_SDFATINIT             0x0112	// Initializing SD FAT
#define APDUINO_MSG_SDFATSTARTED          0x0113	// SD FAT Started
#define APDUINO_MSG_SSPINPREPARED         0x0114	// Storage prepared SS pin for output
#define APDUINO_MSG_LOGCHECK	            0x0115	// Checking existing log file
#define APDUINO_MSG_LOGROTATENEEDED       0x0116	// Logrotate needed
#define APDUINO_MSG_LOGROTATE             0x0117	// Logrotate needed
#define APDUINO_MSG_LOGROTATED            0x0118	// Logrotate performed on %d files
#define APDUINO_MSG_SDLOGGINGOK           0x011A	// Logging to SD card ok

#define APDUINO_MSG_RTCNOTRUNNING         0x0120  // RTC is not running (maybe no hardware?)
#define APDUINO_MSG_HWRTCOK               0x0121	// HW RTC running OK
#define APDUINO_MSG_SWRTCSTART            0x0122	// Starting SW clock
#define APDUINO_MSG_SETUPUDPFORNTP        0x0123	// Setting up UDP for NTP
#define APDUINO_MSG_UDPFORNTPOK           0x0124 	// UDP networking prepared for NTP
#define APDUINO_MSG_NTPUDPPACKPREP        0x0125	// Preparing NTP packet for sync
#define APDUINO_MSG_NTPUDPPACKSEND        0x0126	// Sending NTP packet for sync
#define APDUINO_MSG_TIMEADJUST		        0x0127	// Adjusting clock...
#define APDUINO_MSG_TIMEADJUSTED		      0x0128	// Adjusted clock

#define APDUINO_MSG_ETHSTARTED            0x0130	// Ethernet started
#define APDUINO_MSG_CONFETHSTARTED	      0x0131  // Ethernet started from config
#define APDUINO_MSG_NETOK                 0x0132	// Ethernet looks ok
#define APDUINO_MSG_TRYINGSTATICIP        0x0132	// Trying to start ethernet with static config
#define APDUINO_MSG_TRYINGDHCPIP          0x0133	// Trying to start ethernet with DHCP
#define APDUINO_MSG_DHCPLEASED            0x0134	// DHCP Lease obtained
#define APDUINO_MSG_NETFAILSRESTART       0x0135	// Network failure count triggers attempt to restart net
#define APDUINO_MSG_AOLOGCALLED           0x0136	// APDuino Online logging...
#define APDUINO_MSG_AOLOGGING             0x0137	// APDuino Online logging %d chars.
#define APDUINO_MSG_AOLOGDONE             0x0138	// APDuino Online logging done
#define APDUINO_MSG_COSMLOGCALLED         0x0139	// Cosm logging...
#define APDUINO_MSG_COSMLOGGING           0x0140	// Cosm logging %d chars.
#define APDUINO_MSG_COSMLOGDONE           0x0141	// Cosm logging done
#define APDUINO_MSG_TSLOGCALLED           0x0142	// ThingSpeak logging started
#define APDUINO_MSG_TSLOGGING             0x0143	// ThingSpeak logging %d chars.
#define APDUINO_MSG_TSLOGDONE             0x0144	// ThingSpeak logging done
#define APDUINO_MSG_AOLOGSTARTED          0x0145	// APDuino Online logging started
#define APDUINO_MSG_COSMLOGSTARTED        0x0146	// Cosm logging started
#define APDUINO_MSG_PROCPROVREQ           0x0147	// Processing provisioning request
#define APDUINO_MSG_PROVFILE           		0x0148	// Provisioning file %s
#define APDUINO_MSG_WWWFILEACCESS					0x0149  // WWW File Access %s

#define APDUINO_MSG_LOADINGSENSORS        0x0150	// Sensor Array loading Sensors from config
#define APDUINO_MSG_SENSORCOUNT 	        0x0151	// Sensor Array counted %d lines in SENSORS.CFG
#define APDUINO_MSG_SENSORSLOADING        0x0152	// Loading Sensor Array from config %s
#define APDUINO_MSG_SENSORSLOADED         0x0153	// Sensor Array loaded

#define APDUINO_MSG_LOADINGCONTROLS       0x0160	// Control Array loading Controls from config %s
#define APDUINO_MSG_CONTROLCOUNT 	        0x0161	// Control Array counted %d lines in CONTROLS.CFG
#define APDUINO_MSG_CONTROLSLOADED        0x0162	// Control Array loaded
#define APDUINO_MSG_CONTROLSPOSTPROCESSED 0x0163	// Control Array postprocessed

#define APDUINO_MSG_LOADINGRULES          0x0170	// Rule Array loading Rules from config %s
#define APDUINO_MSG_RULECOUNT             0x0171	// Rule Array counted %d lines in RULES.CFG
#define APDUINO_MSG_RULESLOADED           0x0172	// Rule Array loaded
#define APDUINO_MSG_RULESPOSTPROCESSED    0x0173	// Rule Array postprocessed

#define APDUINO_MSG_CRONLAUNCHED          0x01A0	// Cron scheduler started, first run at %ld mills
#define APDUINO_MSG_CRONCHECK     	      0x01A1	// Cron scheduler checking jobs
#define APDUINO_MSG_CRONRUN               0x01A2	// Cron schedule met, running job

#define APDUINO_MSG_OWADDRVERIFY  				0x01C0	// 1-Wire sensor: address verification for %s
#define APDUINO_MSG_OWSEARCHING						0x01C1	// 1-Wire sensor: continue search on wire...
#define APDUINO_MSG_OWADDRFOUND						0x01C2	// 1-Wire sensor: address found
#define APDUINO_MSG_OWFIRSTADDRSAVED			0x01C3	// 1-Wire sensor: first address found was saved
#define APDUINO_MSG_OWDS18S20							0x01C4	// 1-Wire sensor: DS18S20 chip
#define APDUINO_MSG_OWDS18B20							0x01C5	// 1-Wire sensor: DS18B20 chip
#define APDUINO_MSG_OWDS1822							0x01C6	// 1-Wire sensor: DS1822 chip
#define APDUINO_MSG_OWNOMOREADDR  				0x01C7	// 1-Wire sensor: no more addresses.
#define APDUINO_MSG_OWADDRVEROK  					0x01C8	// 1-Wire sensor: specified address found or first taken.
#define APDUINO_MSG_OWADDRVERDONE  			  0x01C9	// 1-Wire sensor: address enumeration complete.
#define APDUINO_MSG_OWDIAGNOSTICS			    0x01CA	// 1-Wire sensor: diagnostics running.
#define APDUINO_MSG_OWDIAGNOSTICSDONE	    0x01CB	// 1-Wire sensor: diagnostics done.

#define APDUINO_MSG_VIBSENDEFAULTRC 	    0x01E0	// Vibration Sensor: using default read count (10)

// ERRORS
#define APDUINO_ERROR_UNKNOWN		          0x0200	// Unknown/undefined error.
#define APDUINO_ERROR_SSCANF		          0x0201	// Bad version of sscanf/sprintf linked.
#define APDUINO_ERROR_OUTOFRAM	          0x0202	// Not enough free ram.
#define APDUINO_ERROR_STORAGENOTSETUP     0x0203  // Storage not setup
#define APDUINO_ERROR_LOGUNKNOWN		      0x0204	// Unknown error in logging to SD prepararion
#define APDUINO_ERROR_STORAGENOTREADY	    0x0205	// Storage not ready
#define APDUINO_ERROR_NOTIMEOBJECT        0x0206	// No time object
#define APDUINO_ERROR_STORAGEALLOCALREADY 0x0207  // Storage already allocated error
#define APDUINO_ERROR_STORAGEALLOC        0x0208  // Storage allocation error
#define APDUINO_ERROR_COULDNOTPAUSEWWW    0x0209	// Could not pause Web service
#define APDUINO_ERROR_BADNETCONFIG        0x020A	// Bad network config. Please reprovision from APDuino Online.
#define APDUINO_ERROR_BADNETCONFIG2       0x020B	// Bad ethernet config detected in net setup
#define APDUINO_ERROR_SUSPECTSTORAGEERR   0x020C	// Suspected storage error in net setup
#define APDUINO_ERROR_NETALREADYSTARTED   0x020D	// Net already started?
#define APDUINO_ERROR_NONETFORWWW         0x020E	// No network interface set up for WWW server


#define APDUINO_ERROR_SDFATSTARTERR       0x0210	// Error starting SD FAT
#define APDUINO_ERROR_FILEOPEN            0x0211	// Error opening file '%s'
#define APDUINO_ERROR_LOGOPENERR          0x0212	// Error opening log file
#define APDUINO_ERROR_LOGSDERR            0x0213	// SD Error when opening log file
#define APDUINO_ERROR_LOGROTATEOUTOFMEM   0x0214	// Logrotate failed to allocate RAM
#define APDUINO_ERROR_LOGMSGOUTOFMEM      0x0215	// Debug logger failed to allocate RAM for message string
#define APDUINO_ERROR_LOGITEMOUTOFMEM     0x0216	// Logrotate failed to allocate RAM for log item

#define APDUINO_ERROR_RTCALLOCFAIL        0x0220	// Failed to allocate RTC
#define APDUINO_ERROR_RTCALREADYINIT      0x0221	// RTC already initialized
#define APDUINO_ERROR_SWRTCFAIL           0x0222	// SW RTC error
#define APDUINO_ERROR_SWRTCSETUPFAIL      0x0223	// SW RTC setup failure
#define APDUINO_ERROR_UDPNETINITFAIL      0x0224	// UDP Networking initialization failed
#define APDUINO_ERROR_NTPUDPSTARTFAIL     0x0225	// UDP Networking start failed
#define APDUINO_ERROR_NTPNOUDP            0x0226	// No UDP for NTP
#define APDUINO_ERROR_NTPNORTC            0x0227	// No RTC object for NTP sync

#define APDUINO_ERROR_ETHCONF		          0x0230	// Failed to configure Ethernet. Fix DHCP on your LAN or provide a valid static config on SD and reset.
#define APDUINO_ERROR_DHCPFAILED		      0x0231	// DHCP Failure. Fix DHCP on your LAN or provide a valid static config on SD and reset.
#define APDUINO_ERROR_DHCPSTARTFAIL		    0x0232	// DHCP Failure. Fix DHCP on your LAN or provide a valid static config on SD and reset.
#define APDUINO_ERROR_APDUINOONLINEIP     0x0233	// APDuino Online config error (IP)
#define APDUINO_ERROR_STORAGEERRORAO      0x0234	// STorage error when setting up APduino Online
#define APDUINO_ERROR_AKSAVEIOERR         0x0235 	//  API Key save IO Error
#define APDUINO_ERROR_AKSAVESTORAGE       0x0236 	// API key save error - No storage
#define APDUINO_ERROR_WWWFSNOSTORAGE      0x0237	// Error listing SD card, no storage
#define APDUINO_ERROR_WWWCLIENTOCCUPIED   0x0238	// APDuino Online: web client taken by another process
#define APDUINO_ERROR_WWWCANTCONNECTAO    0x0239	// APDuino Online: connection error
#define APDUINO_ERROR_WWWCLIENTBUSY       0x023A	// APDuino Online: client busy
#define APDUINO_ERROR_AONOWEBCLIENT       0x023B	// APDuino Online: no web client
#define APDUINO_ERROR_AOLOGNOWEBCLIENT    0x023C	// APDuino Online: no web client for logger
#define APDUINO_ERROR_CLOGNOWEBCLIENT     0x023D	// Cosm Logging: no web client for logger
#define APDUINO_ERROR_CLOGCONNFAIL        0x023E	// Cosm Logging: connection failure
#define APDUINO_ERROR_TSLOGCONNFAIL       0x023F	// ThingSpeak Logging: connection failure
#define APDUINO_ERROR_PRINTCLIENTOUTOFMEM 0x0240	// APDWeb: out of RAM when printing progmem string
#define APDUINO_ERROR_JSNOCLIENT          0x0241	// JSON Status: no web client pointer
#define APDUINO_ERROR_COSMDUMPOPEN        0x0242	// Cosm dump file open failed: %s
#define APDUINO_ERROR_BROKENHEXCODE       0x0243	// Broken hex code in http request
#define APDUINO_MSG_AOSELFREG				      0x0244	// APDuino Online: self registering: %s
#define APDUINO_ERROR_WWWAUTHFAIL         0x0245	// WWW Authentication Failed %s
#define APDUINO_ERROR_WWWUNAUTH           0x0246	// WWW Unauthorized, authentication requested
#define APDUINO_ERROR_WWWNOCLIENT         0x0247	// WWW : no client connection in %s

#define APDUINO_ERROR_UNKNOWNSENSORTYPE   0x0250	// Unknown sensor type
#define APDUINO_ERROR_SAALLOCFAIL         0x0251	// Failed to allocate Sensor Array
#define APDUINO_ERROR_SAEMPTY             0x0252	// No sensors defined
#define APDUINO_ERROR_SAALREADYALLOC      0x0253	// Sensor Array already allocated

#define APDUINO_ERROR_CAALREADYALLOC      0x0260	// Control Array already allocated
#define APDUINO_ERROR_CANOCONTROLS		    0x0261	// Control Array: no controls defined
#define APDUINO_ERROR_CAALLOCFAIL			    0x0262	// Control Array allocation failure
#define APDUINO_ERROR_CAINVALIDCUSTFUNC		0x0263	// Control Array: invalid custom function
#define APDUINO_ERROR_CAMISSINGCUSTFUNC		0x0264	// Control Array: missing custom function
#define APDUINO_ERROR_CADUMPOPENFAIL		  0x0265	// Control Array: error opening dump file

#define APDUINO_ERROR_RAALREADYALLOC      0x0270	// Rule Array already allocated
#define APDUINO_ERROR_RANORULES           0x0271	// Rule Array : no rules defined
#define APDUINO_ERROR_RAALLOCFAIL         0x0272	// Rule Array allocation failure
#define APDUINO_ERROR_RADUMPOPENFAIL		  0x0273	// Rule Array: error opening dump file

#define APDUINO_ERROR_RDEFINVALID         0x02A0	// Invalid Rule definition
#define APDUINO_ERROR_RMETROALLOCFAIL     0x02A1	// Failed to allocate Rule Metro
#define APDUINO_ERROR_RACTIONINVALID      0x02A2	// Invalid action definition for Rule
#define APDUINO_ERROR_RAOUTOFMEM          0x02A3	// Rule out of memory
#define APDUINO_ERROR_NOCRONSPEC          0x02A4	// Missing cron specification
#define APDUINO_ERROR_CRONOUTOFRAM        0x02A5	// Cron evaluator out of memory

#define APDUINO_ERROR_OWNOADDR						0x02C0	// 1-Wire sensor: no address found
#define APDUINO_ERROR_OWNOOBJ						  0x02C1	// 1-Wire sensor: missing OneWire object
#define APDUINO_ERROR_OWNOTREADY   			  0x02C2	// 1-Wire sensor: not ready
#define APDUINO_ERROR_OWBADCRC    			  0x02C3	// 1-Wire sensor: invalid CRC
#define APDUINO_ERROR_OWNOTDS18X20				0x02C4	// 1-Wire sensor: not a DS18x20 family chipset
#define APDUINO_ERROR_OWNOADDRESSES				0x02C5	// 1-Wire sensor: no addresses found on wire

#define APDUINO_ERROR_DHTINVALIDCLASS     0x02D0  // DHT sensor: invalid sensor class %d

#define APDUINO_ERROR_SONSENCALIBRATION   0x02E0	// Sonar Sensor: calibration error

// WARNINGS
#define APDUINO_WARNING_TIMEALREADYSETUP  0x0301	// Time already initialized
#define APDUINO_WARN_UNKNOWNREQUEST	      0x0301	// Unknown distributed request
#define APDUINO_WARN_CUSTFUNCMISMATCH     0x0302	// Control is not referencing this custom function
#define APDUINO_WARN_NOCUSTFUNCATIDX      0x0303	// No custom function pointer at specified index
#define APDUINO_WARN_MESSAGESFLUSHED      0x0304	// %d debug messages has been suppressed to save RAM

#define APDUINO_WARN_NOUDPFORNTP          0x0320	// No UDP for NTP
#define APDUINO_WARN_NETCONFDHCPFALLBACK  0x0321	// Failed to start Ethernet with static configuration. Fallback to DHCP.

#define APDUINO_WARN_CTYPEINVALID	        0x0390	// Invalid control type
#define APDUINO_WARN_RULEINVALIDCONTROL   0x03A0	// Invalid or NULL Control referenced in Rule

#define APDUINO_WARN_OWBADADDRFOUND		    0x03A0	// 1-Wire sensor: unexpected address found
#endif /* APD_MSG_CODES_H_ */
