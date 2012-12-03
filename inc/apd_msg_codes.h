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
 * todo ongoing effort to organize log message codes in the proper groups
 *
 *
 *  Created on: Oct 2, 2012
 *      Author: George Schreiber
 */

#ifndef APD_MSG_CODES_H_
#define APD_MSG_CODES_H_

#define LOG_LEVEL_VERBOSE    0			// really detailed (development) logs
#define LOG_LEVEL_DEBUG      1			// messages that can help users debugging a setup
#define LOG_LEVEL_MESSAGES	 2			// messages that give a relatively detailed idea of the program flow
#define LOG_LEVEL_WARNING		 3			// warnings, non blocking errors but unexpected stuff
#define LOG_LEVEL_LOG				 4			// log level: prints access messages
#define LOG_LEVEL_ERROR			 5			// error: only errors
#define LOG_LEVEL_QUIET			 6			// shut up /for well configured systems/

// VERBOSE      0

// DEBUG				1
#define APDUINO_DEBUG_STORAGESTART        0x1011	// Starting Storage
#define APDUINO_DEBUG_LOGROTATE_DEL       0x1018	// Logrotate performed simple delete
#define APDUINO_DEBUG_AOLOGCALLED         0x1036	// APDuino Online logging...
#define APDUINO_DEBUG_AOLOGGING           0x1037	// APDuino Online logging %d chars.
#define APDUINO_DEBUG_COSMLOGCALLED       0x1039	// Cosm logging...
#define APDUINO_DEBUG_COSMLOGGING         0x1040	// Cosm logging %d chars.
#define APDUINO_DEBUG_TSLOGCALLED         0x1042	// ThingSpeak logging started
#define APDUINO_DEBUG_TSLOGGING           0x1043	// ThingSpeak logging %d chars.

#define APDUINO_DEBUG_SETUPUDPFORNTP      0x1020	// Setting up UDP for NTP
#define APDUINO_DEBUG_UDPFORNTPOK         0x1021 	// UDP networking prepared for NTP
#define APDUINO_DEBUG_USING_DEFAULTNTP    0x1022  // Using default NTP server
#define APDUINO_DEBUG_RTCADJUST						0x1025  // Adjusting Hardware RTC
#define APDUINO_DEBUG_NTPSYNC							0x1026  // Starting NTP Sync
#define APDUINO_DEBUG_NTPUDPPACKPREP        0x2027	// Preparing NTP packet for sync

// MESSAGES			2
#define APDUINO_MSG_SERIAL_INIT		        0x2001	// Serial port initialized

#define APDUINO_MSG_SDLOGGING             0x2005	// Logging to SD card %d chars
#define APDUINO_MSG_SDLOGRESCHED			    0x2006	// SD logging interval reconfigured

#define APDUINO_MSG_ETHERNETFROMCONF      0x20E5	// Initializing ethernet with configuration from file
#define APDUINO_MSG_NETINIT               0x20E6	// Initializing ethernet
#define APDUINO_MSG_DHCPFALLBACK          0x20E7 	// Fallback to DHCP

#define APDUINO_MSG_STORAGEINIT           0x2010	// Initializing Storage
#define APDUINO_MSG_SDFATINIT             0x2012	// Initializing SD FAT
#define APDUINO_MSG_SDFATSTARTED          0x2013	// SD FAT Started
#define APDUINO_MSG_SSPINPREPARED         0x2014	// Storage prepared SS pin for output
#define APDUINO_MSG_LOGCHECK	            0x2015	// Checking existing log file
#define APDUINO_MSG_LOGROTATENEEDED       0x2016	// Logrotate needed
#define APDUINO_MSG_LOGROTATE             0x2017	// Logrotate needed
#define APDUINO_MSG_LOGROTATED            0x2018	// Logrotate performed on %d files
#define APDUINO_MSG_SDLOGGINGOK           0x201A	// Logging to SD card ok


#define APDUINO_MSG_SWRTCSTART            0x2022	// Starting SW clock
#define APDUINO_MSG_NTPUDPPACKSEND        0x2026	// Sending NTP packet for sync
#define APDUINO_MSG_TIMEADJUST		        0x2027	// Adjusting clock...
#define APDUINO_MSG_TIMEADJUSTED		      0x2028	// Adjusted clock


#define APDUINO_MSG_ETHSTARTED            0x2030	// Ethernet started
#define APDUINO_MSG_CONFETHSTARTED	      0x2031  // Ethernet started from config
#define APDUINO_MSG_NETOK                 0x2032	// Ethernet looks ok
#define APDUINO_MSG_TRYINGSTATICIP        0x2032	// Trying to start ethernet with static config
#define APDUINO_MSG_TRYINGDHCPIP          0x2033	// Trying to start ethernet with DHCP
#define APDUINO_MSG_DHCPLEASED            0x2034	// DHCP Lease obtained
#define APDUINO_MSG_NETFAILSRESTART       0x2035	// Network failure count triggers attempt to restart net
#define APDUINO_MSG_AOLOGDONE             0x2038	// APDuino Online logging done
#define APDUINO_MSG_COSMLOGDONE           0x2041	// Cosm logging done
#define APDUINO_MSG_TSLOGDONE             0x2044	// ThingSpeak logging done
#define APDUINO_MSG_AOLOGSTARTED          0x2045	// APDuino Online logging started
#define APDUINO_MSG_COSMLOGSTARTED        0x2046	// Cosm logging started
#define APDUINO_MSG_PROCPROVREQ           0x2047	// Processing provisioning request
#define APDUINO_MSG_PROVFILE           	  0x2048	// Provisioning file %s
#define APDUINO_MSG_WWWFILEACCESS		      0x2049  // WWW File Access %s

#define APDUINO_MSG_AOSRRESPPROC		      0x204B  // APDuino Online self-registration response processing
#define APDUINO_MSG_AOSRRESPJOINTPROV     0x204C  // APDuino Online self-registration %d bytes left to scan for joint configuration provisioning

#define APDUINO_MSG_LOADINGSENSORS        0x2350	// Sensor Array loading Sensors from config
#define APDUINO_MSG_SENSORCOUNT 	        0x2351	// Sensor Array counted %d lines in SENSORS.CFG
#define APDUINO_MSG_SENSORSLOADING        0x2352	// Loading Sensor Array from config %s
#define APDUINO_MSG_SENSORSLOADED         0x2353	// Sensor Array loaded
#define APDUINO_MSG_SENSORDIAG         		0x235A	// Sensor Diagnostics Requested


#define APDUINO_MSG_LOADINGCONTROLS       0x2460	// Control Array loading Controls from config %s
#define APDUINO_MSG_CONTROLCOUNT 	        0x2461	// Control Array counted %d lines in CONTROLS.CFG
#define APDUINO_MSG_CONTROLSLOADED        0x2462	// Control Array loaded
#define APDUINO_MSG_CONTROLSPOSTPROCESSED 0x2463	// Control Array postprocessed

#define APDUINO_MSG_LOADINGRULES          0x2570	// Rule Array loading Rules from config %s
#define APDUINO_MSG_RULECOUNT             0x2571	// Rule Array counted %d lines in RULES.CFG
#define APDUINO_MSG_RULESLOADED           0x2572	// Rule Array loaded
#define APDUINO_MSG_RULESPOSTPROCESSED    0x2573	// Rule Array postprocessed

#define APDUINO_MSG_CRONLAUNCHED          0x25A0	// Cron scheduler started, first run at %ld mills
#define APDUINO_MSG_CRONCHECK     	      0x25A1	// Cron scheduler checking jobs
#define APDUINO_MSG_CRONRUN               0x25A2	// Cron schedule met, running job

#define APDUINO_MSG_OWADDRVERIFY  				0x26C0	// 1-Wire sensor: address verification for %s
#define APDUINO_MSG_OWSEARCHING						0x26C1	// 1-Wire sensor: continue search on wire
#define APDUINO_MSG_OWADDRFOUND						0x26C2	// 1-Wire sensor: address found
#define APDUINO_MSG_OWFIRSTADDRSAVED			0x26C3	// 1-Wire sensor: first address found was saved
#define APDUINO_MSG_OWDS18S20							0x26C4	// 1-Wire sensor: DS18S20 chip
#define APDUINO_MSG_OWDS18B20							0x26C5	// 1-Wire sensor: DS18B20 chip
#define APDUINO_MSG_OWDS1822							0x26C6	// 1-Wire sensor: DS1822 chip
#define APDUINO_MSG_OWNOMOREADDR  				0x26C7	// 1-Wire sensor: no more addresses.
#define APDUINO_MSG_OWADDRVEROK  					0x26C8	// 1-Wire sensor: specified address found or first taken.
#define APDUINO_MSG_OWADDRVERDONE  			  0x26C9	// 1-Wire sensor: address enumeration complete.
#define APDUINO_MSG_OWDIAGNOSTICS			    0x26CA	// 1-Wire sensor: diagnostics running.
#define APDUINO_MSG_OWDIAGNOSTICSDONE	    0x26CB	// 1-Wire sensor: diagnostics done.
#define APDUINO_MSG_OWSTOREVALUE    	    0x26CC	// 1-Wire sensor: storing value %f

#define APDUINO_MSG_ATLAS							    0x26D0	// AtlasSensor: diagnostics done.
#define APDUINO_MSG_ATLAS_CO							0x26D1	// AtlasSensor: DEMUXER SW serial channel %d
#define APDUINO_MSG_ATLAS_RX  				    0x26D2	// AtlasSensor: data received "%s"
#define APDUINO_MSG_ATLAS_STO  				    0x26D3	// AtlasSensor: storing "%s"
#define APDUINO_MSG_ATLAS_TX					    0x26D4	// AtlasSensor: data sent at %ul
#define APDUINO_MSG_ATLAS_HW  				    0x26D5	// AtlasSensor: hardware serial prints "%s"
#define APDUINO_MSG_ATLAS_SW					    0x26D6	// AtlasSensor: software serial prints "%s"
#define APDUINO_MSG_ATLAS_HWP  				    0x26D7	// AtlasSensor: hardware serial prints "%s"
#define APDUINO_MSG_ATLAS_SWP					    0x26D8	// AtlasSensor: software serial prints "%s"
#define APDUINO_MSG_ATLAS_HWPC 				    0x26D9	// AtlasSensor: hardware serial prints '%c'
#define APDUINO_MSG_ATLAS_SWPC				    0x26DA	// AtlasSensor: software serial prints '%c'

#define APDUINO_MSG_ATLAS_BUSY				    0x26DF	// AtlasSensor: sensor busy


#define APDUINO_MSG_VIBSENDEFAULTRC 	    0x26E0	// Vibration Sensor: using default read count (10)


// WARNINGS				3
#define APDUINO_WARN_UNKNOWNREQUEST	      0x3001	// Unknown distributed request
#define APDUINO_WARN_CUSTFUNCMISMATCH     0x3002	// Control is not referencing this custom function
#define APDUINO_WARN_NOCUSTFUNCATIDX      0x3003	// No custom function pointer at specified index
#define APDUINO_WARN_MESSAGESFLUSHED      0x3004	// %d debug messages has been suppressed to save RAM

#define APDUINO_WARN_RTCNOTRUNNING        0x3020  // RTC is not running (maybe no hardware?)
#define APDUINO_WARN_TIMEALREADYSETUP     0x3021	// Time already initialized

#define APDUINO_WARN_NOUDPFORNTP          0x30E0	// No UDP for NTP
#define APDUINO_WARN_NETCONFDHCPFALLBACK  0x30E1	// Failed to start Ethernet with static configuration. Fallback to DHCP.

#define APDUINO_WARN_SENSORARRAY					0x3300	// SensorArray Warning

#define APDUINO_WARN_SENSOR								0x3380	// SensorWarning

#define APDUINO_WARN_CTYPEINVALID	        0x3400	// Invalid control type

#define APDUINO_WARN_CONTROL				      0x3480	// Control Warning

#define APDUINO_WARN_RULEINVALIDCONTROL   0x3501	// Invalid or NULL Control referenced in Rule

#define APDUINO_WARN_OWBADADDRFOUND		    0x36A1	// 1-Wire sensor: unexpected address found

#define APDUINO_WARN_ATLAS     				    0x36D1	// AtlasSensor: warning "%s"
#endif /* APD_MSG_CODES_H_ */


// LOGS (SYSTEM MESSAGES) 4
#define APDUINO_LOG_START   					    0x4000	// APDuinOS version %s starting,free ram:
#define APDUINO_LOG_LOGLEVEL					    0x4002	// System loglevel is %d
#define APDUINO_LOG_SDLOGOK					      0x4003	// Logging to SD prepared
#define APDUINO_LOG_TIMESTAMP		        	0x4004	// Current system timestamp: %s

#define APDUINO_LOG_HWRTCOK               0x4020	// HW RTC running OK
#define APDUINO_LOG_SWRTCOK               0x4021	// SW RTC running OK
#define APDUINO_LOG_NETRESTART            0x403E	// Attempting to restart network interface
#define APDUINO_LOG_NETRESTARTED          0x403F	// Network interface restarted
#define APDUINO_LOG_NTPSYNCOK							0x4070	// Clock synced to NTP

#define APDUINO_LOG_RECONF								0x4ABC  // Performing reconfiguration

#define APDUINO_LOG_ENABLERULEPROC        0x4AA1	// Enabled rule processing
#define APDUINO_LOG_DISABLERULEPROC       0x4AA0	// Disabled rule processing

#define APDUINO_LOG_AOSELFREG		          0x4E4A	// APDuino Online: self registering: %s
#define APDUINO_LOG_AOSELFREGREQSENT		  0x4E4B	// APDuino Online: self register request sent with key %s
#define APDUINO_LOG_AOSELFREGOK		        0x4E4C	// APDuino Online: self register succeeded.
#define APDUINO_LOG_WWWSYSACCESS          0x4E4D	// WWW: System Access via HTTP: %s
#define APDUINO_LOG_WWWSENSORACCESS	      0x4E4E	// WWW: Sensor Access via HTTP: %s
#define APDUINO_LOG_WWWCONTROLACCESS      0x4E4F	// WWW: Control Access via HTTP: %s
#define APDUINO_LOG_WWWDEBUGLEVEL		      0x4E50	// WWW: Debug Level Change
#define APDUINO_LOG_WWWCONTROLSET         0x4E5F	// WWW: Control was set: %d, value: %d

// ERRORS	 			5
#define APDUINO_ERROR_UNKNOWN		          0x5000	// Unknown/undefined error.
#define APDUINO_ERROR_SSCANF		          0x5001	// Bad version of sscanf/sprintf linked.
#define APDUINO_ERROR_OUTOFRAM	          0x5002	// Not enough free ram.
#define APDUINO_ERROR_STORAGENOTSETUP     0x5003  // Storage not setup
#define APDUINO_ERROR_LOGUNKNOWN		      0x5004	// Unknown error in logging to SD prepararion
#define APDUINO_ERROR_STORAGENOTREADY	    0x5005	// Storage not ready
#define APDUINO_ERROR_NOTIMEOBJECT        0x5006	// No time object
#define APDUINO_ERROR_STORAGEALLOCALREADY 0x5007  // Storage already allocated error
#define APDUINO_ERROR_STORAGEALLOC        0x5008  // Storage allocation error
#define APDUINO_ERROR_COULDNOTPAUSEWWW    0x5009	// Could not pause Web service
#define APDUINO_ERROR_BADNETCONFIG        0x500A	// Bad network config. Please reprovision from APDuino Online.
#define APDUINO_ERROR_BADNETCONFIG2       0x500B	// Bad ethernet config detected in net setup
#define APDUINO_ERROR_SUSPECTSTORAGEERR   0x500C	// Suspected storage error in net setup
#define APDUINO_ERROR_NETALREADYSTARTED   0x500D	// Net already started?
#define APDUINO_ERROR_NONETFORWWW         0x500E	// No network interface set up for WWW server
#define APDUINO_ERROR_NETRESTARTFAIL      0x5030  // Failed to restart network interface

#define APDUINO_ERROR_SDFATSTARTERR       0x5080	// Error starting SD FAT
#define APDUINO_ERROR_FILEOPEN            0x5081	// Error opening file '%s'
#define APDUINO_ERROR_LOGOPENERR          0x5082	// Error opening log file
#define APDUINO_ERROR_LOGSDERR            0x5083	// SD Error when opening log file
#define APDUINO_ERROR_LOGROTATEOUTOFMEM   0x5084	// Logrotate failed to allocate RAM
#define APDUINO_ERROR_LOGMSGOUTOFMEM      0x5085	// Debug logger failed to allocate RAM for message string
#define APDUINO_ERROR_LOGITEMOUTOFMEM     0x5086	// Logrotate failed to allocate RAM for log item
#define APDUINO_ERROR_LOGITEMLOWRAMFAIL   0x5087	// Logrotate detected low RAM conditions, skipping item

#define APDUINO_ERROR_RTCALLOCFAIL        0x5090	// Failed to allocate RTC
#define APDUINO_ERROR_RTCALREADYINIT      0x5091	// RTC already initialized
#define APDUINO_ERROR_SWRTCFAIL           0x5092	// SW RTC error
#define APDUINO_ERROR_SWRTCSETUPFAIL      0x5093	// SW RTC setup failure
#define APDUINO_ERROR_ETHCONF		          0x50E0	// Failed to configure Ethernet. Fix DHCP on your LAN or provide a valid static config on SD and reset.
#define APDUINO_ERROR_DHCPFAILED		      0x50E1	// DHCP Failure. Fix DHCP on your LAN or provide a valid static config on SD and reset.
#define APDUINO_ERROR_DHCPSTARTFAIL		    0x50E2	// DHCP Failure. Fix DHCP on your LAN or provide a valid static config on SD and reset.
#define APDUINO_ERROR_APDUINOONLINEIP     0x50E3	// APDuino Online config error (IP)
#define APDUINO_ERROR_UDPNETINITFAIL      0x50E4	// UDP Networking initialization failed
#define APDUINO_ERROR_NTPUDPSTARTFAIL     0x50E5	// UDP Networking start failed
#define APDUINO_ERROR_NTPNOUDP            0x50E6	// No UDP for NTP
#define APDUINO_ERROR_NTPNORTC            0x50E7	// No RTC object for NTP sync
#define APDUINO_ERROR_STORAGEERRORAO      0x50E8	// STorage error when setting up APduino Online
#define APDUINO_ERROR_AKSAVEIOERR         0x50E9 	//  API Key save IO Error
#define APDUINO_ERROR_AKSAVESTORAGE       0x50EA 	// API key save error - No storage
#define APDUINO_ERROR_WWWFSNOSTORAGE      0x5037	// Error listing SD card, no storage
#define APDUINO_ERROR_JSNOCLIENT          0x5E01	// JSON Status: no web client pointer
#define APDUINO_ERROR_COSMDUMPOPEN        0x5E02	// Cosm dump file open failed: %s
#define APDUINO_ERROR_BROKENHEXCODE       0x5E03	// Broken hex code in http request
#define APDUINO_ERROR_WWWCLIENTOCCUPIED   0x5EE0	// APDuino Online: web client taken by another process
#define APDUINO_ERROR_WWWCANTCONNECTAO    0x5EE1	// APDuino Online: connection error
#define APDUINO_ERROR_WWWCLIENTBUSY       0x5EE2	// APDuino Online: client busy
#define APDUINO_ERROR_AONOWEBCLIENT       0x5EE3	// APDuino Online: no web client
#define APDUINO_ERROR_AOLOGNOWEBCLIENT    0x5EE4	// APDuino Online: no web client for logger
#define APDUINO_ERROR_AOSELFREG		        0x5EE5	// APDuino Online: error self registering: %s
#define APDUINO_ERROR_CLOGNOWEBCLIENT     0x5EF0	// Cosm Logging: no web client for logger
#define APDUINO_ERROR_CLOGCONNFAIL        0x5EF1	// Cosm Logging: connection failure
#define APDUINO_ERROR_TSLOGCONNFAIL       0x5EF5	// ThingSpeak Logging: connection failure
#define APDUINO_ERROR_PRINTCLIENTOUTOFMEM 0x5EEE	// APDWeb: out of RAM when printing progmem string
#define APDUINO_ERROR_WWWAUTHFAIL         0x5FF0	// WWW Authentication Failed %s
#define APDUINO_ERROR_WWWUNAUTH           0x5FF1	// WWW Unauthorized, authentication requested
#define APDUINO_ERROR_WWWNOCLIENT         0x5FF2	// WWW : no client connection in %s

#define APDUINO_ERROR_UNKNOWNSENSORTYPE   0x5300	// Unknown sensor type
#define APDUINO_ERROR_SAALLOCFAIL         0x5301	// Failed to allocate Sensor Array
#define APDUINO_ERROR_SAEMPTY             0x5302	// No sensors defined
#define APDUINO_ERROR_SAALREADYALLOC      0x5303	// Sensor Array already allocated

#define APDUINO_ERROR_CAALREADYALLOC      0x5400	// Control Array already allocated
#define APDUINO_ERROR_CANOCONTROLS		    0x5402	// Control Array: no controls defined
#define APDUINO_ERROR_CAALLOCFAIL			    0x5403	// Control Array allocation failure
#define APDUINO_ERROR_CAINVALIDCUSTFUNC		0x5404	// Control Array: invalid custom function
#define APDUINO_ERROR_CAMISSINGCUSTFUNC		0x5405	// Control Array: missing custom function
#define APDUINO_ERROR_CADUMPOPENFAIL		  0x5406	// Control Array: error opening dump file

#define APDUINO_ERROR_RAALREADYALLOC      0x5500	// Rule Array already allocated
#define APDUINO_ERROR_RANORULES           0x5501	// Rule Array : no rules defined
#define APDUINO_ERROR_RAALLOCFAIL         0x5502	// Rule Array allocation failure
#define APDUINO_ERROR_RADUMPOPENFAIL		  0x5503	// Rule Array: error opening dump file
#define APDUINO_ERROR_EVALNOTSUPPORTED    0x5550  // Rule Evaluation: Evaluation function not supported

#define APDUINO_ERROR_RDEFINVALID         0x5510	// Invalid Rule definition
#define APDUINO_ERROR_RMETROALLOCFAIL     0x5511	// Failed to allocate Rule Metro
#define APDUINO_ERROR_RACTIONINVALID      0x5512	// Invalid action definition for Rule
#define APDUINO_ERROR_RAOUTOFMEM          0x5513	// Rule out of memory
#define APDUINO_ERROR_NOCRONSPEC          0x5514	// Missing cron specification
#define APDUINO_ERROR_CRONOUTOFRAM        0x5515	// Cron evaluator out of memory

#define APDUINO_ERROR_OWNOADDR						0x5601	// 1-Wire sensor: no address found
#define APDUINO_ERROR_OWNOOBJ						  0x5602	// 1-Wire sensor: missing OneWire object
#define APDUINO_ERROR_OWNOTREADY   			  0x5603	// 1-Wire sensor: not ready
#define APDUINO_ERROR_OWBADCRC    			  0x5604	// 1-Wire sensor: invalid CRC
#define APDUINO_ERROR_OWNOTDS18X20				0x5605	// 1-Wire sensor: not a DS18x20 family chipset
#define APDUINO_ERROR_OWNOADDRESSES				0x5606	// 1-Wire sensor: no addresses found on wire
#define APDUINO_ERROR_OWUNKNOWNSTATE			0x5607	// 1-Wire sensor: unknown state

#define APDUINO_ERROR_ATLAS     				  0x5A01	// AtlasSensor: error "%s"
#define APDUINO_ERROR_ATLAS_DEMUX					0x5A02	// AtlasSensor: DEMUXER config ERROR
#define APDUINO_ERROR_ATLAS_DATA      		0x5A03	// AtlasSensor: DEMUXER config ERROR
#define APDUINO_ERROR_ATLAS_HW  				  0x5A04	// AtlasSensor: hardware serial error "%s"
#define APDUINO_ERROR_ATLAS_SW  				  0x5A05	// AtlasSensor: software serial error "%s"

#define APDUINO_ERROR_DHTINVALIDCLASS     0x5A06  // DHT sensor: invalid sensor class %d

#define APDUINO_ERROR_SONSENCALIBRATION   0x5A07	// Sonar Sensor: calibration error
