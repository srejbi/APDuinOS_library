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
 * APDTime.h
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

#ifndef APDTIME_H_
#define APDTIME_H_
#include <Arduino.h>
#include <Wire.h>                // needed for RTC
#include "RTClib.h"              // needed for RTC
#include "apd_utils.h"
#include <EthernetUdp.h>                          // used for NTP
#include "APDSerial.h"
#include "APDDebugLog.h"

const int NTP_PACKET_SIZE= 48;                   // NTP time stamp is in the first 48 bytes of the message

class APDTime {
public:
	static void begin();
	static int begin(boolean bRTC);

	static boolean started();
	static DateTime now();
	static char *nowS(char *strbuf);
	static void adjust(DateTime dt);
	static unsigned long getUpTime();
	static char *getUpTimeS(char *psz_uptime);

	static void PrintDateTime(DateTime t);

	static void ntpSync();
	static void setupNTPSync(int UDPPort, byte *TimeServer, int iTZ, int iDST );
	static unsigned long sendNTPpacket(byte *address);

	static int time_zone;
	static int dst;
	static byte timeServer[4];
	static int localPort;

private:
	static EthernetUDP *pUdp;                        // todo, make this a pointer
	static byte pb[NTP_PACKET_SIZE];                 // buffer to hold incoming and outgoing NTP packets
	static RTC_DS1307 *pRTC;                         // RTC
	static RTC_Millis *pRTCm;                        // RTC for sw timekeeping
	static unsigned long start_time;
	static unsigned long rollovers;                  // TODO calc rollovers
};

#endif /* APDTIME_H_ */
