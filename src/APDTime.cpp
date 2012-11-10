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
 * APDTime.cpp
 *
 *  Created on: Mar 27, 2012
 *      Author: George Schreiber
 *
 * APDTime implements APDuino timekeeping with RTC hardware or software clock.
 * It is based on the RTCLib from JeeLabs - http://news.jeelabs.org/code/
 *
 * Credits:
 * * RTCLib - Copyright JeeLabs - http://news.jeelabs.org/code/
 * * NTP syncing is based on NTP2RTC - code in the public domain - http://www.arduino.cc/playground/Main/DS1307OfTheLogshieldByMeansOfNTP
 *   original credits:
 *      "based upon Udp NTP Client, by Michael Margolis, mod by Tom Igoe
 *       uses the RTClib from adafruit (based upon Jeelabs)
 *       Thanx!
 *       mod by Rob Tillaart, 10-10-2010"
 *
 */

#include "APDTime.h"
int APDTime::time_zone = 0;
int APDTime::dst = 0;
byte APDTime::timeServer[4] = {0,0,0,0};
int APDTime::localPort = 0;

//private:
EthernetUDP *APDTime::pUdp;                        // todo, make this a pointer
byte APDTime::pb[NTP_PACKET_SIZE];                 // buffer to hold incoming and outgoing NTP packets
RTC_DS1307 *APDTime::pRTC;                         // RTC
RTC_Millis *APDTime::pRTCm;                        // RTC for sw timekeeping
unsigned long APDTime::start_time = millis();
unsigned long APDTime::rollovers = 0;

int APDTime::begin(boolean bRTC) {
  begin();
  if (bRTC) {
    if (pRTC == NULL) {
      pRTC = new RTC_DS1307();

      if (pRTC != NULL) {
        // not powering from A2&3
        //pinMode(A3, OUTPUT);  //digitalWrite(A3, HIGH);
        //pinMode(A2, OUTPUT);  //digitalWrite(A2, LOW);

        Wire.begin();
        pRTC->begin();

        if (! pRTC->isrunning()) {
        	APDDebugLog::log(APDUINO_WARN_RTCNOTRUNNING,NULL);
          free(pRTC);
          pRTC = NULL;
          //  SerPrintP("RTC HW NEEDS TESTING!\n");
          //RTC.adjust(DateTime(__DATE__, __TIME__));
        } else {          // we have RTC
        	APDDebugLog::log(APDUINO_LOG_HWRTCOK,NULL);
        }
      } else {
      	APDDebugLog::log(APDUINO_ERROR_RTCALLOCFAIL,NULL);
        pRTC = NULL;
      }
    }else{
    	APDDebugLog::log(APDUINO_ERROR_RTCALREADYINIT,NULL);
    }
  } else {
      // millis based already by default
      if (pRTCm == NULL) {
      	APDDebugLog::log(APDUINO_ERROR_SWRTCFAIL,NULL);
      } else {
      	APDDebugLog::log(APDUINO_LOG_SWRTCOK,NULL);
      }
  }
}

boolean APDTime::started() {
	return start_time != 0;
}

void APDTime::begin() {
  rollovers = 0;
  start_time = millis();
  // todo log this when enabled log levels ("APDTime Starting at "); Serial.print(start_time,DEC); SerPrintP(" millis...\n");
  pRTC = NULL;
  pRTCm = new RTC_Millis();
  if (pRTCm) {
  	APDDebugLog::log(APDUINO_MSG_SWRTCSTART,NULL);
		// TODO check & load time from APDLOG.TXT last message, if exists
		pRTCm->begin(DateTime(__DATE__, __TIME__));           // SW clock always start @ last build time - adjusted later if RTC | NTP
		log_datetime(pRTCm->now());
  } else {
  	APDDebugLog::log(APDUINO_ERROR_SWRTCSETUPFAIL,NULL);
  	// todo log this when enabled log levels ("Seems SW clock setup failed. :(\n");
  }
  memcpy(timeServer,0,4*sizeof(byte));
  localPort = 0;
  pUdp = NULL;
  // todo log this when enabled log levels ("SW DateTime: "); (this->now())
}

// return current system time in DateTime
DateTime APDTime::now() {
  if (pRTCm != NULL && pRTC->isrunning()) {
      return pRTC->now();
  } else if (pRTCm != NULL) {
      return pRTCm->now();
  }
  return DateTime(1970,01,01,0,0,0);            // otherwise
}

// returns time printed to the char array. no checks, you must make sure the ptr to the char array is valid
// and that the buffer is long enough to hold the result (otherwise bad things WILL happen)
char *APDTime::nowS(char *strbuf) {
  DateTime tnow = now();
  sprintf_P(strbuf,PSTR("%04d-%02d-%02d %02d:%02d:%02d"),tnow.year(),tnow.month(),tnow.day(),tnow.hour(),tnow.minute(),tnow.second());
  return strbuf;
}

// returns uptime in millis
unsigned long APDTime::getUpTime() {
  return (millis() - start_time);
}

// prints uptime in a char buffer, days,hours,minutes,seconds
// the buffer must be large enough (>13 chars) to hold the string that is (13chars+\0)
// returns pointer to the char buffer or EOF if an error occurred (sprintf)
char *APDTime::get_uptime_str(char *psz_uptime) {
  unsigned long ut = getUpTime() / 1000;
  // calculate uptime positions
  int updays = (ut / 3600) / 24;
  int uphours = (ut / 3600) % 24;
  int upmins = (ut % 3600) / 60;
  int upsec = (ut % 3600) % 60;
  // print uptime to the string
  sprintf_P(psz_uptime,PSTR("%03dd%02dh%02dm%02ds"),updays,uphours,upmins,upsec);
  return psz_uptime;
}

// TODO return whatever udp would return
void APDTime::setup_ntp_sync(int UDPPort, byte *TimeServer, int iTZ, int iDST ) {
  // check if not in use...
	APDDebugLog::log(APDUINO_MSG_SETUPUDPFORNTP,NULL);
	// todo log this when enabled log levels ("SETUP UDP 4 NTP\n");
  if (pUdp==NULL) {
    memcpy(timeServer,TimeServer,4);
    dst = iDST;
    time_zone = iTZ;
    localPort = UDPPort;
    pUdp = new EthernetUDP;
    if (pUdp != NULL) {				// if we have UDP
      if (pUdp->begin(localPort)) {			// if opening localPort for UDP succeeds
      	APDDebugLog::log(APDUINO_MSG_UDPFORNTPOK,NULL);
      } else {													// failed to start UDP on localport
      	APDDebugLog::log(APDUINO_WARN_NOUDPFORNTP,NULL);
				free(pUdp);			// free the EthernetUDP
				pUdp = NULL;		// reset the UDP ptr.
      }
    }else{
    	APDDebugLog::log(APDUINO_ERROR_UDPNETINITFAIL,NULL);
    }
  }
}


///////////////////////////////////////////
//
// NTP2RTC MISC
//
void APDTime::log_datetime(DateTime t)
{
    char datestr[32] = "";
    // TODO implement format string, input by user
    sprintf_P(datestr, PSTR("%04d-%02d-%02d %02d:%02d:%02d"), t.year(), t.month(), t.day(), t.hour(), t.minute(), t.second());
    APDDebugLog::log(APDUINO_LOG_TIMESTAMP,datestr);
}

// callback for SdBaseFile to get current date time
void APDTime::SdDateTimeCallback(uint16_t* date, uint16_t* time) {
	DateTime dtnow = APDTime::now();
	if (date && time) {
		*date = FAT_DATE(dtnow.year(),dtnow.month(),dtnow.day());
		*time = FAT_TIME(dtnow.hour(),dtnow.minute(),dtnow.second());
	} // todo log error? for now let's trust SDFatLib to provide date and time buffers
}

/* * send an NTP request to the time server at the given address
 * NTP syncing is based on NTP2RTC - code in the public domain - http://www.arduino.cc/playground/Main/DS1307OfTheLogshieldByMeansOfNTP
 *   original credits:
 *      "based upon Udp NTP Client, by Michael Margolis, mod by Tom Igoe
 *       uses the RTClib from adafruit (based upon Jeelabs)
 *       Thanx!
 *       mod by Rob Tillaart, 10-10-2010""
*/
unsigned long APDTime::send_ntp_packet(byte *address)
{
  unsigned long ulret = 0;
  if (pUdp != NULL) {
  	APDDebugLog::log(APDUINO_MSG_NTPUDPPACKPREP,NULL);

  	memset(pb, 0, NTP_PACKET_SIZE);		// blank packet
    // Initialize values needed to form NTP request
    // (see URL above for details on the packets)
    pb[0] = 0b11100011;   // LI, Version, Mode
    pb[1] = 0;     // Stratum, or type of clock
    pb[2] = 6;     // Polling Interval
    pb[3] = 0xEC;  // Peer Clock Precision
    // 8 bytes of zero for Root Delay & Root Dispersion
    pb[12]  = 49;
    pb[13]  = 0x4E;
    pb[14]  = 49;
    pb[15]  = 52;

    APDDebugLog::log(APDUINO_MSG_NTPUDPPACKSEND,NULL);
    // all NTP fields have been given values, now
    // you can send a packet requesting a timestamp:
  #if ARDUINO >= 100
    // IDE 1.0 compatible:
    if (pUdp->beginPacket(address, 123)) {
    	pUdp->write(pb,NTP_PACKET_SIZE);
    	pUdp->endPacket();
    } else {
    	APDDebugLog::log(APDUINO_ERROR_NTPUDPSTARTFAIL,NULL);
    }
  #else
    pUdp->sendPacket( pb,NTP_PACKET_SIZE,  address, 123); //NTP requests are to port 123
  #endif
  } else {
  	APDDebugLog::log(APDUINO_ERROR_NTPNOUDP,NULL);
  }
  return ulret;
}

// set time to a given DateTime
void APDTime::adjust(DateTime dt) {
	APDDebugLog::log(APDUINO_MSG_TIMEADJUST,NULL);
  if (pRTC != NULL && pRTC->isrunning()) {
  // todo log this when enabled log levelsSerPrintP("RTC,");

      pRTC->adjust(dt);
  }
  // todo log this when enabled log levels ("SW.");
  pRTCm->adjust(dt);
	APDDebugLog::log(APDUINO_MSG_TIMEADJUSTED,NULL);		// TODO include datetime string in log
}


/* ntpSync() will sync hw/sw clock to a network time server
 * NTP syncing is based on NTP2RTC - code in the public domain - http://www.arduino.cc/playground/Main/DS1307OfTheLogshieldByMeansOfNTP
 *   original credits:
 *      "based upon Udp NTP Client, by Michael Margolis, mod by Tom Igoe
 *       uses the RTClib from adafruit (based upon Jeelabs)
 *       Thanx!
 *       mod by Rob Tillaart, 10-10-2010"
 *
 * integrated into APDuinOS by George Schreiber 27-03-2012
*/

// sync clock to NTP server
void APDTime::sync_to_ntp()
{
	if (pUdp == NULL) {
		APDDebugLog::log(APDUINO_ERROR_NTPNOUDP,NULL);
		return;
	}
	// todo log this when enabled log levels ("\nNTP SYNC!\n");
  if (pRTCm != NULL && timeServer[0] != 0) {
  	// todo log this when enabled log levels0("\nSW:");  PrintDateTime(pRTCm->now()); \
    if (pRTC != NULL && pRTC->isrunning()) { \
        SerPrintP("\nHW:"); \
        PrintDateTime(pRTC->now()); \
    } ; SerPrintP(".NTP REQ:");  SerDumpIP(timeServer);

    // send an NTP packet to a time server
    send_ntp_packet(timeServer);
    // todo log this when enabled log levels ("Packet sent.\n");
    // wait to see if a reply is available
    //delay(3000);                // TODO iso loop and check if there is a response, bail out after soft timeout
    for (int i =0; i < 1000; i++) {	// approx 10 secs.
    	if (!pUdp->available()) {
    		delay(10);
    	} else {
    		break;
    	}
    }

    if ( pUdp->available() ) {
    	// todo log this when enabled log levels ("Comm...");
      // read the packet into the buffer
  #if ARDUINO >= 100
      pUdp->read(pb, NTP_PACKET_SIZE);      // New from IDE 1.0,
  #else
      pUdp->readPacket(pb, NTP_PACKET_SIZE);
  #endif
      // todo log this when enabled log levels ("Processing NTP response ...");
      // NTP contains four timestamps with an integer part and a fraction part
      // we only use the integer part here
      unsigned long t1, t2, t3, t4;
      t1 = t2 = t3 = t4 = 0;
      for (int i=0; i< 4; i++)
      {
        t1 = t1 << 8 | pb[16+i];
        t2 = t2 << 8 | pb[24+i];
        t3 = t3 << 8 | pb[32+i];
        t4 = t4 << 8 | pb[40+i];
      }

      // part of the fractional part
      // could be 4 bytes but this is more precise than the 1307 RTC
      // which has a precision of ONE second
      // in fact one byte is sufficient for 1307
      float f1,f2,f3,f4;
      f1 = ((long)pb[20] * 256 + pb[21]) / 65536.0;
      f2 = ((long)pb[28] * 256 + pb[29]) / 65536.0;
      f3 = ((long)pb[36] * 256 + pb[37]) / 65536.0;
      f4 = ((long)pb[44] * 256 + pb[45]) / 65536.0;

      // NOTE:
      // one could use the fractional part to set the RTC more precise
      // 1) at the right (calculated) moment to the NEXT second!
      //    t4++;
      //    delay(1000 - f4*1000);
      //    RTC.adjust(DateTime(t4));
      //    keep in mind that the time in the packet was the time at
      //    the NTP server at sending time so one should take into account
      //    the network latency (try ping!) and the processing of the data
      //    ==> delay (850 - f4*1000);
      // 2) simply use it to round up the second
      //    f > 0.5 => add 1 to the second before adjusting the RTC
      //   (or lower threshold eg 0.4 if one keeps network latency etc in mind)
      // 3) a SW RTC might be more precise, => ardomic clock :)
      // todo log this when enabled log levels("NTP->UNIX...");

      // convert NTP to UNIX time, differs seventy years = 2208988800 seconds
      // NTP starts Jan 1, 1900
      // Unix time starts on Jan 1 1970.
      const unsigned long seventyYears = 2208988800UL;
      t1 -= seventyYears;
      t2 -= seventyYears;
      t3 -= seventyYears;
      t4 -= seventyYears;

      /*
      Serial.println("T1 .. T4 && fractional parts");
      PrintDateTime(DateTime(t1)); Serial.println(f1,4);
      PrintDateTime(DateTime(t2)); Serial.println(f2,4);
      PrintDateTime(DateTime(t3)); Serial.println(f3,4);
      */
      log_datetime(DateTime(t4)); Serial.println(f4,4);

      // Adjust timezone and DST
      t4 += (( time_zone + dst )* 3600L);     // Notice the L for long calculations!
      t4 += 1;               									// adjust the delay(1000) at begin of loop!
      if (f4 > 0.4) t4++;   								  // adjust fractional part, see above
      adjust(DateTime(t4));

      // todo log this when enabled log levels \
         if (pRTC != NULL){ \
      				SerPrintP("RTC after : ");  PrintDateTime(pRTC->now());  \
						}  ; if (pRTC != NULL) { \
							SerPrintP("RTC after : "); \
							PrintDateTime(pRTC->now()); \
						} \
						SerPrintP("APDTime will give time:"); \
						PrintDateTime(now()); \
						SerPrintP("\ndone ...\n"); \

    }
    else
    {
    	APDDebugLog::log(APDUINO_ERROR_NTPNOUDP,NULL);
    }
  } else {
  	APDDebugLog::log(APDUINO_ERROR_NTPNORTC,NULL);
  }
}

char *apduino_fullversion(char *szbuf) {
	DateTime dtt(__DATE__,__TIME__);
	sprintf_P(szbuf,APDUINO_VERSION_STRING,
			dtt.year(),dtt.month(),dtt.day(),
			dtt.hour(), dtt.minute(), dtt.second());
	return szbuf;
}
