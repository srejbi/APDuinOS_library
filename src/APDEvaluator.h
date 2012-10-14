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
 * APDEvaluator.h
 *
 *  Created on: Jul 30, 2012
 *      Author: George Schreiber
 *
 * APDEvaluator implements a mixed float and boolean arithmetic evaluator
 * inspired by / based on http://rosettacode.org/wiki/Arithmetic_Evaluator/C
 * released under the GNU Free Documentation License 1.2
 *
 * Credits: original C evaluator implementation Copyright by RosettaCode 2010
 *
 * Integrated into APDuinOS and further improvements by George Schreiber 30-07-2012
 *
 */

#ifndef APDEVALUATOR_H_
#define APDEVALUATOR_H_

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "APDSerial.h"
#include "APDSensorArray.h"
#include "APDControlArray.h"
//TODO deprecate the next line when done debugging
#include <MemoryFree.h>          // checking free ram


typedef union {
  float term;	// iso int
  struct expression* expr[2];
} ExprData;

typedef struct expression {
  char op;
  ExprData data;
} Expr;

class APDEvaluator {
public:
	APDEvaluator(APDSensorArray *pSA, APDControlArray *pCA);
	virtual ~APDEvaluator();

	void parse_error(const char* string);

	char proc_c(const char* string, char c);
	int proc_i(const char* string);
	float proc_f(const char *string);

	Expr* factor(const char* string, Expr* expr);
	Expr* factor_right(const char* string, Expr* expr);
	Expr* term(const char* string, Expr* expr);
	Expr* term_right(const char* string, Expr* expr);
	Expr* expression(const char* string);

	/* Runs through the AST, evaluating and freeing
	 * the tree as it goes.
	 */
	float evaluate(Expr* expr);

	int dothatstuff(const char *str);
	float feval(const char *str);
	int iError;

private:
  unsigned int G_STRING_ITERATOR;
  APDSensorArray *pSensors;
  APDControlArray *pControls;

/*
 * expr        := term term_right
 * term_tail   := add_op term term_right | e
 * term        := factor factor_right
 * factor_tail := mult_op factor factor_right | e
 * factor      := ( expr ) | number
 * add_op      := + | -
 * mult_op     := * | /
 */

};

#endif /* APDEVALUATOR_H_ */
