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
 * APDSensorArray.cpp
 *
 *  Created on: Apr 3, 2012
 *      Author: George Schreiber
 */

#include "APDSensorArray.h"



APDSensorArray::APDSensorArray()
{
  // TODO Auto-generated constructor stub
  pAPDSensors = NULL;
  iSensorCount = 0;
  iNextSensor = 0;
  pfruleeval = NULL;
  pRA = NULL;
}

APDSensorArray::~APDSensorArray()
{
  // TODO Auto-generated destructor stub
  if (this->pAPDSensors != NULL) {
      for (int i=0; i<this->iSensorCount; i++) {
          if (this->pAPDSensors[i] != NULL) {
          	// todo log this when enabled log levels ("\nX S"); Serial.print(i); delay(50);
              delete((this->pAPDSensors[i]));				// each APDSensor was new by 'new_sensor_parser'
              // todo log this when enabled log levels
              this->pAPDSensors[i] = NULL;
          }
      }
      free(this->pAPDSensors);										// APDSensor pointers array was malloc
      this->pAPDSensors = NULL;
      this->iSensorCount=0;
  }
}

void APDSensorArray::enableRuleEvaluation(void (*pfunc)(void*, APDSensor *),void *pra) {
	this->pfruleeval = pfunc;
	this->pRA = pra;
}

// todo actualize format string and arguments!
int APDSensorArray::dumpToFile(char * pszFileName) {
  // make a string for assembling the data to log:
	// todo log this when enabled log levels ("Dumping Sensor Array Config...");
    if (APDStorage::p_sd->exists(pszFileName)) {
          APDStorage::p_sd->remove(pszFileName);
        }
  SdFile dataFile(pszFileName, O_WRITE | O_CREAT );
  if (dataFile.isOpen()) {
    for (int i=0; i<iSensorCount; i++) {
      char line[RCV_BUFSIZ]="";
      APDSensor *ps = pAPDSensors[i];
      // TODO update with recent fields
      sprintf_P(line,PSTR("%s %i,%i,%i,%i,%i,%i,%i,%s"),ps->config.label,ps->config.sensor_type,ps->config.sensor_class,ps->config.sensor_subtype,ps->config.sensor_pin,ps->config.sensor_secondary_pin,ps->config.sensor_freq,ps->config.sensor_log,ps->config.extra_data);
      dataFile.println(line);
    }
    dataFile.close();
    // todo log this when enabled log levels ("Sensor Array Config dumped.");
  }
  else {
  	// TODO log this ("E305 ('"); Serial.print(pszFileName); SerPrintP("')\n");			// E305 - error opening dump file
  }
}




void APDSensorArray::new_sensor_parser(void *pSA, int iline, char *psz) {
  SDCONF sdc;
  memcpy(&sdc,0,sizeof(SDCONF));
  char szExtra[24] = "";
  // todo log this when enabled log levels ("\nSDCONF READS: \""); Serial.print(psz); SerPrintP("\"");
  int iscand = sscanf_P( psz, SENSOR_PARSERSTRING,
      (sdc.label),
      &(sdc.sensor_type),
      &(sdc.sensor_class),
      &(sdc.sensor_subtype),
      &(sdc.sensor_pin),
      &(sdc.sensor_secondary_pin),
      &(sdc.sensor_freq),
      &(sdc.sensor_log),
      (sdc.extra_data));

  if (iscand < 9) {
  	// todo log this when enabled log levels ("no extra data");
      memset(sdc.extra_data,0,sizeof(char)*24);
  }
  // todo log this when enabled log levels ("CONF SCANNED: ") \
  		(sdc.label); SerPrintP(","); \
  		(sdc.sensor_type); SerPrintP(","); \
			(sdc.sensor_class); SerPrintP(","); \
			(sdc.sensor_subtype); SerPrintP(","); \
			(sdc.sensor_pin); SerPrintP(","); \
			(sdc.sensor_secondary_pin); SerPrintP(","); \
			(sdc.sensor_freq); SerPrintP(","); \
			(sdc.sensor_log); SerPrintP(","); \
			(sdc.extra_data);

  //TODO add compatibility code (so other options can follow, even if not parsed)

  APDSensor *newsensor = NULL;
  APDSensor *reusablesensor = ((APDSensorArray *)pSA)->findReusableSensor(&sdc);

  //(((APDuino *)pAPD)->pAPDSensors[iline]) = new APDSensor(&sdc);
  switch (sdc.sensor_type) {
  case ANALOG_SENSOR:
  	// todo log this when enabled log levels ("ANALOG");
    newsensor = new AnalogSensor(&sdc);
    break;
  case DIGITAL_SENSOR:
  	// todo log this when enabled log levels ("DIGITAL");
    newsensor = new DigitalSensor(&sdc);
    break;
  case DHT_SENSOR:
  	// todo log this when enabled log levels ("DHT");
    newsensor = new DHTSensor(&sdc, reusablesensor);
    break;
  case ONEWIRE_SENSOR:
  	// todo log this when enabled log levels ("1-WIRE");
    newsensor = new OneWireSensor(&sdc, reusablesensor);
    break;
  case SONAR_SENSOR:
  	// todo log this when enabled log levels ("SONAR");
    newsensor = new SonarSensor(&sdc);
    break;
  case BMP085_SENSOR:
  	// todo log this when enabled log levels ("BMP085");
    newsensor = new BMPSensor(&sdc, reusablesensor);
    break;
  case VIBRATION_SENSOR:
  	// todo log this when enabled log levels("VIBRATION");
    newsensor = new VibrationSensor(&sdc);
    break;
  case ATLASSCIENTIFIC_SENSOR:
  	// todo log this when enabled log levels ("ATLASSCIENTIFIC");
      newsensor = new AtlasScientificSensor(&sdc, reusablesensor);
      break;
  default:
  	APDDebugLog::log(APDUINO_ERROR_UNKNOWNSENSORTYPE,NULL);
  }
  ((APDSensorArray*)pSA)->pAPDSensors[iline] = newsensor;
  // done with sensor definition line
}

APDSensor *APDSensorArray::firstSensorByPin(int iPin, int iType) {
  APDSensor *psens = NULL;
  for (int i=0; i<this->iSensorCount && this->pAPDSensors[i] != NULL; i++) {
      if (this->pAPDSensors[i]->config.sensor_type == iType &&
          this->pAPDSensors[i]->config.sensor_pin == iPin) {
          psens = this->pAPDSensors[i];
          break;
      }
  }
  return psens;
}

int APDSensorArray::indexBySensor(APDSensor *pSensor) {
	int idx = -2;
  if (pSensor != NULL) {
  	idx = -1;
		for (int i=0; i<this->iSensorCount && this->pAPDSensors[i] != NULL; i++) {
				if (this->pAPDSensors[i] == pSensor) {
						idx = i;
						break;
				}
		}
  }
  return idx;
}


APDSensor *APDSensorArray::findReusableSensor(SDCONF *sdc) {
  APDSensor *preusable = NULL;
  switch (sdc->sensor_type) {
  // TODO: put all reusable sensors below
  case ONEWIRE_SENSOR:
  case DHT_SENSOR:
  case BMP085_SENSOR:
  case ATLASSCIENTIFIC_SENSOR:
    preusable = this->firstSensorByPin(sdc->sensor_pin, sdc->sensor_type);
    // todo log this when enabled log levels (below VERBOSEs)
#ifdef VERBOSE
    if (preusable) {
        SerPrintP("REUSING @");
        Serial.print((unsigned int)preusable,HEX);
    }
#endif
    break;
  default:
  	;
#ifdef VERBOSE
    SerPrintP("NOT ");
#endif
  }
#ifdef VERBOSE
  SerPrintP("REUSABLE");
#endif
  return preusable;
}

APDSensor *APDSensorArray::byIndex(int idx) {
	APDSensor *ret = NULL;
  if (idx >= 0 && idx <= this->iSensorCount) {
  	ret = this->pAPDSensors[idx];
  }
  return ret;
}



int APDSensorArray::loadSensors() {
  if (pAPDSensors == NULL) {    // if no sensor array
  	char szConfFile[32] = "";
  	strcpy_P(szConfFile,PSTR("SENSORS.CFG"));
  	APDDebugLog::log(APDUINO_MSG_LOADINGSENSORS,NULL);						// debug
    // TODO check if SD is available!
    iSensorCount =  get_line_count_from_file(szConfFile);

    char sztemp[11]="";		// supports unsigned longs
    APDDebugLog::log(APDUINO_MSG_SENSORCOUNT,itoa(iSensorCount,sztemp,10));

    if (iSensorCount > -1) {
    	// todo log this when enabled log levels ("Sensor Array: allocating "); Serial.print(sizeof(APDSensor *)*iSensorCount,DEC); SerPrintP(" bytes of RAM\n")

      pAPDSensors = (APDSensor**)malloc(sizeof(APDSensor*)*iSensorCount);

      if (pAPDSensors != NULL) {
        memset(pAPDSensors,0,sizeof(APDSensor*)*iSensorCount);
        APDDebugLog::log(APDUINO_MSG_SENSORSLOADING,szConfFile);

        APDStorage::read_file_with_parser((char *)szConfFile,&new_sensor_parser, (void*)this);

        APDDebugLog::log(APDUINO_MSG_SENSORSLOADED,NULL);
        // TODO add any postprocessing

        iNextSensor = 0;                // first sensor to read
      } else {
      	APDDebugLog::log(APDUINO_ERROR_SAALLOCFAIL,NULL);
      }
    } else {
    	APDDebugLog::log(APDUINO_ERROR_SAEMPTY,NULL);
    }
  } else {
  	APDDebugLog::log(APDUINO_ERROR_SAALREADYALLOC,NULL);
  }
  return iSensorCount;
}




int APDSensorArray::count() {
  return this->iSensorCount;
}

char *APDSensorArray::labelS(int iSensorIdx, char *szlabel) {
  //SerPrintP("labelS\n"); Serial.print(iSensorIdx);
  szlabel[0]='\0';
  if (iSensorIdx >= 0 && iSensorIdx < this->iSensorCount) {
  	// todo log this when enabled log levels ("calling getlabel"); Serial.println(iSensorIdx);
      strncpy(szlabel,this->pAPDSensors[iSensorIdx]->config.label,12);  // FIXME setup DEFAULT label len in header
      // todo log this when enabled log levels (szlabel);
  }
  return szlabel;
}

char *APDSensorArray::valueS(int iSensorIdx, char *szvalue) {
  //SerPrintP("valueS\n"); Serial.print(iSensorIdx);
  strcpy(szvalue,"");
  if (iSensorIdx >= 0 && iSensorIdx < this->iSensorCount) {
  	// todo log this when enabled log levels ("calling getval"); Serial.println(iSensorIdx);
    this->pAPDSensors[iSensorIdx]->get_value_str(szvalue);
    // todo log this when enabled log levels (szvalue);
  }
  return szvalue;
}


void APDSensorArray::pollSensors(boolean bProcessRules) {
	// todo log this when enabled log levels ("POLL APD SENSOR..."); Serial.print(iNextSensor); \
  Serial.print(iSensorCount); SerPrintP(" SENSORS IN TOTAL, CHECKING "); Serial.println(iNextSensor);

	if (iNextSensor >= 0 && iNextSensor < this->iSensorCount && pAPDSensors != NULL)                // check if we have sensors
    if (pAPDSensors[iNextSensor]->check()) {
    	if (bProcessRules == true && this->pfruleeval != NULL && this->pRA != NULL) {
    		// todo log this when enabled log levels ("early eval");
    	  if (pAPDSensors[iNextSensor]->fvalue != NAN) {	// todo add an option to run rules on NAN?
    	  	(*this->pfruleeval)(this->pRA,pAPDSensors[iNextSensor]);
    	  } else {
    	  	// no valid value, don't evaluate rules
    	  }
      }
    }
  iNextSensor = iNextSensor + 1 < this->iSensorCount ? iNextSensor + 1 : 0;    // set next sensor
}



void APDSensorArray::diagnostics() {
  for (int i=0; i<this->iSensorCount; i++) {
      this->pAPDSensors[i]->diagnostics();
  }
}
