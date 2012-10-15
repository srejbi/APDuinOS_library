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
 * APDControlArray.h
 *
 *  Created on: Apr 6, 2012
 *      Author: George Schreiber
 */

#ifndef APDCONTROLARRAY_H_
#define APDCONTROLARRAY_H_

#include <Arduino.h>
#include "apd_version.h"
#include "apd_utils.h"
#include "APDStorage.h"
#include "APDControl.h"

//void (*pcustfuncs[10])();

class APDControlArray
{
public:
  APDControlArray();
  APDControlArray(void *pcustomfunctions);
  virtual
  ~APDControlArray();

  static void new_control_parser(void *pCA, int iline, char *psz);
  int loadControls();
  int dumpToFile(char *pszFileName);

  APDControl *firstControlByPin(int iPin, int iType);
  APDControl *findReusableControl(CDCONF *cdc);
  APDControl *byIndex(int idx);
private:
  APDControl** pAPDControls;
  int iControlCount;
  //void (*pcustfuncs[10])() ;
  //(void ((*pcustfuncs))())* ;                        // allow 10 custom functions to be called
  void **pcustfuncs;

  friend class APDRuleArray;
  friend class APDuino;                 // FIXME don't want direct acces
};

#endif /* APDCONTROLARRAY_H_ */
