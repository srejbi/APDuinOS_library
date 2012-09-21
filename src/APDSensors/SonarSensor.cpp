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
 * SonarSensor.cpp
 *
 *  Created on: Apr 5, 2012
 *      Author: George Schreiber
 */

#include "SonarSensor.h"

SonarSensor::SonarSensor(SDCONF *sdc)
{
  this->initSensor(sdc);
  // TODO Auto-generated constructor stub
  this->sensor = (SONSENS*)malloc(sizeof(SONSENS));
  if (sscanf(this->config.extra_data, "%f", &(this->sensor->calibration_value)) != 1) {
    SerPrintP("SonarSensor: calibration error.\n");
    this->sensor->calibration_value = 1 / 58;                           // DEFAULT VALUE FIXME define in header
  }
#ifdef DEBUG
  else {
      SerPrintP("SONAR: CALIBRATIONDATA:"); Serial.println(this->sensor->calibration_value);
  }
#endif
  this->sensor->value = 0;
  this->fvalue = 0;
}

SonarSensor::~SonarSensor()
{
  free(this->sensor);
  this->sensor = NULL;
  delete(this->pmetro);
  this->pmetro = NULL;
}

boolean SonarSensor::perform_check()
{
  boolean retcode = (this->sensor->value = this->read_sonar());
  this->fvalue = this->sensor->value;
  return retcode;
}

float SonarSensor::read_sonar() {
  SerPrintP("SRF05 READING");
  const int numOfReadings = 10;                   // number of readings to take/ items in the array
  int readings[numOfReadings];                    // stores the distance readings in an array
  int arrayIndex = 0;                             // arrayIndex of the current item in the array
  int total = 0;                                  // stores the cumlative total
  float averageDistance = 0;                        // stores the average value

  // setup pins and variables for SRF05 sonar device
  int echoPin = this->config.sensor_pin;                    // SRF05 echo pin - we read
  int initPin = this->config.sensor_secondary_pin;          // SRF05 trigger pin - we write
  unsigned long pulseTime = 0;                    // stores the pulse in Micro Seconds
  unsigned long distance = 0;                     // variable for storing the distance (cm)

  pinMode(initPin, OUTPUT);                     // set init pin as output
  pinMode(echoPin, INPUT);                      // set echo pin as input

  // create array loop to iterate over every item in the array

  for (int thisReading = 0; thisReading < numOfReadings; thisReading++) {
    readings[thisReading] = 0;
   }

  int iDelay = 10;
  int numValidReadings = 0;
   for (int i=0; i < numOfReadings; i++) {
#ifdef DEBUG
    SerPrintP("TRIGGER AND WAIT ECHO...");
#endif
    digitalWrite(initPin, HIGH);                    // send 10 microsecond pulse
    delayMicroseconds(10);                  // wait 10 microseconds before turning off
    digitalWrite(initPin, LOW);                     // stop sending the pulse
    pulseTime = pulseIn(echoPin, HIGH);             // Look for a return pulse, it should be high as the pulse goes low-high-low
    if (pulseTime > 0 || i+1 >= numOfReadings) {                // only the last reading is taken as valid if 0
      numValidReadings++;
#ifdef DEBUG
      SerPrintP("...OK: "); Serial.print(pulseTime); SerPrintP("ms ");
#endif
      distance = pulseTime * this->sensor->calibration_value;        // Distance = pulse time * calibration value to convert to cm.
#ifdef DEBUG
      SerPrintP("..calc dist: "); Serial.print(distance); SerPrintP(" cm");
#endif
      total= total - readings[arrayIndex];           // subtract the last distance
      readings[arrayIndex] = distance;                // add distance reading to array
      total= total + readings[arrayIndex];            // add the reading to the total
      arrayIndex = arrayIndex + 1;                    // go to the next item in the array
      // At the end of the array (10 items) then start again
      if (arrayIndex >= numOfReadings)  {
          arrayIndex = 0;
        }

    } else {    // wait for silence

        iDelay += 5;
    }
    delay(iDelay);
#ifdef DEBUG
     SerPrintP(" reading cycle done.\n");
#endif
   }
    //averageDistance = total / numOfReadings;      // calculate the average distance
     averageDistance = total / numValidReadings;      // calculate the average distance
#ifdef DEBUG
    SerPrintP("Average distance measured: "); Serial.println(averageDistance, DEC);         // print out the average distance to the debugger
#endif
    return averageDistance;
}


