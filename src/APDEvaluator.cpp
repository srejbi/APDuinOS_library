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
 * APDEvaluator.cpp
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
 */

#include "APDEvaluator.h"

APDEvaluator::APDEvaluator(APDSensorArray *pSA, APDControlArray *pCA) {
	this->G_STRING_ITERATOR = 0;
	this->iError = 0;
	this->pSensors = pSA;
	this->pControls = pCA;
	// TODO Auto-generated constructor stub

}

APDEvaluator::~APDEvaluator() {
	// TODO Auto-generated destructor stub

}



void APDEvaluator::parse_error(const char* string) {
  unsigned int i;
  char szMessage[128];
  sprintf_P(szMessage, PSTR("Unexpected symbol '%c' at position %u.\n\n"), string[G_STRING_ITERATOR], G_STRING_ITERATOR);
  Serial.print(szMessage);
  sprintf_P(szMessage, PSTR("String: '%s'\n"), string);
  Serial.print(szMessage);
  sprintf_P(szMessage, PSTR("Problem: "));
  Serial.print(szMessage);
  for(i = 0; i < G_STRING_ITERATOR; ++i) {
    SerPrintP(" ");
  }
  SerPrintP("^\n");
  iError++;
  //while (true) ;; // exit 1
}

/* Will "consume" a character from the input,
 * (such as +, -, *, etc.) and return it.
 * By consume, I'm really just moving the pointer
 * forward and disregarding the character for
 * future purposes.
 */
char APDEvaluator::proc_c(const char* string, char c) {
  if(string[G_STRING_ITERATOR] != c) {
    parse_error(string);
  } else {
    ++G_STRING_ITERATOR;
  }
  return c;
}

/* Same as consume_char, except for integers.
 */
int APDEvaluator::proc_i(const char* string) {
  int i;

  if(!isdigit(string[G_STRING_ITERATOR])) {
    parse_error(string);
  } else {

    /* I don't have to pass in the start of the string
     * into atoi, but only where I want it to start
     * scanning for an integer.
     */
    i = atoi(string + G_STRING_ITERATOR);
    while(isdigit(string[G_STRING_ITERATOR])) {
      ++G_STRING_ITERATOR;
    }
  }
  return i;
}

/* Same as consume_int, except for floats.
 */
float APDEvaluator::proc_f(const char* string) {
  float f=0;

  if(!isdigit(string[G_STRING_ITERATOR])) {
    parse_error(string);
  } else {

    /* I don't have to pass in the start of the string
     * into atoi, but only where I want it to start
     * scanning for an integer.
     */
    //f = atof(string + G_STRING_ITERATOR);
    sscanf(string+G_STRING_ITERATOR,"%f",&f);
    while(isdigit(string[G_STRING_ITERATOR])||string[G_STRING_ITERATOR]=='.') { // TODO what about end of string??
      ++G_STRING_ITERATOR;
    }
  }
  return f;
}


Expr* APDEvaluator::factor(const char* string, Expr* expr) {
  if(string[G_STRING_ITERATOR] == '(') {
    expr->op = proc_c(string, '(');
    expr->data.expr[0] = expression(string);
    proc_c(string, ')');
  } else if(string[G_STRING_ITERATOR] == 'c') {		//control
  	expr->op = proc_c(string, 'c');						// consume 'c' (control) operand
  	expr->op = 'd';																	// set operand to digit
  	int iControl = proc_i(string);							// parse the control index
  	APDControl *pC = NULL;													// will get the control
		if ((pC = this->pControls->byIndex(iControl)) != NULL ) {
			expr->data.term = pC->iValue;
			//expr->data.terminal = consume_int(string);
		} else {
			parse_error(string);		// TODO parse_error must return
			expr->data.term = 0;
		}
  } else if(string[G_STRING_ITERATOR] == 's') {		//sensor
   	expr->op = proc_c(string, 's');						// consume 's' (sensor) operand
   	expr->op = 'd';
   	int iSensor = proc_i(string);
   	APDSensor *pS = NULL;
   	if ((pS = this->pSensors->byIndex(iSensor)) != NULL ) {
   		expr->data.term = pS->fValue();
   		//expr->data.terminal = consume_int(string);
   	} else {
   		parse_error(string);
   		expr->data.term = 0;
   	}
  } else if(isdigit(string[G_STRING_ITERATOR])) {
    expr->op = 'd';
    expr->data.term = proc_f(string);
  }
  return expr;
}

Expr* APDEvaluator::factor_right(const char* string, Expr* expr) {
  Expr* new_expr;

  switch(string[G_STRING_ITERATOR]) {
  case '*':
  case '/':
  case '%':
    if(NULL == (new_expr = (Expr*)malloc(sizeof(Expr)))) {
      exit(1);
    }
    if(NULL == (new_expr->data.expr[1] = (Expr*)malloc(sizeof(Expr)))) {
      exit(1);
    }
    new_expr->op = proc_c(string, string[G_STRING_ITERATOR]);
    new_expr->data.expr[0] = expr;

    new_expr->data.expr[1] = factor(string, new_expr->data.expr[1]);
    new_expr = factor_right(string, new_expr);
    return new_expr;
  case '+':
  case '-':
  case '[':
  case ']':
  case '<':
  case '>':
  case '=':
  case '&':
  case '|':
  case ')':
  case 's':
  case 'c':
  case 0:
    return expr;
  default:
    parse_error(string);
  }
}
// TODO add lookahead for sensor and control expressions
Expr* APDEvaluator::term(const char* string, Expr* expr) {
  if(string[G_STRING_ITERATOR] == '(' || isdigit(string[G_STRING_ITERATOR])|| string[G_STRING_ITERATOR]=='c'|| string[G_STRING_ITERATOR]=='s' ) {
    expr = factor(string, expr);
    expr = factor_right(string, expr);
    return expr;
  } else {
    parse_error(string);
  }
}

Expr* APDEvaluator::term_right(const char* string, Expr* expr) {
  Expr* new_expr;

  switch(string[G_STRING_ITERATOR]) {
  case '+':
  case '-':
  case '[':
  case ']':
  case '<':
  case '>':
  case '=':
  case '&':
  case '|':
    if(NULL == (new_expr = (Expr*)malloc(sizeof(Expr)))) {
      exit(1);
    }
    if(NULL == (new_expr->data.expr[1] = (Expr*)malloc(sizeof(Expr)))) {
      exit(1);
    }
    new_expr->op = proc_c(string, string[G_STRING_ITERATOR]);
    new_expr->data.expr[0] = expr;

    new_expr->data.expr[1] = term(string, new_expr->data.expr[1]);
    new_expr = term_right(string, new_expr);
    return new_expr;
  case ')':
  case 's':
  case 'c':
  case 0:
    return expr;
  default:
    parse_error(string);
  }
}
// TODO add lookahead for sensor and control expressions
Expr* APDEvaluator::expression(const char* string) {
  Expr* expr;

  if(string[G_STRING_ITERATOR] == '(' || isdigit(string[G_STRING_ITERATOR])|| string[G_STRING_ITERATOR]=='c'|| string[G_STRING_ITERATOR]=='s') {
    if(NULL == (expr = (Expr*)malloc(sizeof(Expr)))) {
      exit(1);			// TODO this is not the way to handle this
    }

    expr = term(string, expr);
    expr = term_right(string, expr);
    return expr;
  } else {
    parse_error(string);
  }
}

/* Runs through the AST, evaluating and freeing
 * the tree as it goes.
 */
//int APDEvaluator::evaluate(Expr* expr) {
float APDEvaluator::evaluate(Expr* expr) {
  //int ret;
	float ret;

  switch(expr->op) {
  case '(':
    ret = evaluate(expr->data.expr[0]);
    free(expr->data.expr[0]);
    break;
  case '*':
    ret =
      evaluate(expr->data.expr[0])
      *
      evaluate(expr->data.expr[1])
      ;
    free(expr->data.expr[0]);
    free(expr->data.expr[1]);
    break;
  case '/':
    ret =
      evaluate(expr->data.expr[0])
      /
      evaluate(expr->data.expr[1])
      ;
    free(expr->data.expr[0]);
    free(expr->data.expr[1]);
    break;
  case '%':
    ret = fmod(evaluate(expr->data.expr[0]), evaluate(expr->data.expr[1]));
    free(expr->data.expr[0]);
    free(expr->data.expr[1]);
    break;
  case '+':
    ret =
      evaluate(expr->data.expr[0])
      +
      evaluate(expr->data.expr[1])
      ;
    free(expr->data.expr[0]);
    free(expr->data.expr[1]);
    break;
  case '-':
    ret =
      evaluate(expr->data.expr[0])
      -
      evaluate(expr->data.expr[1])
      ;
    free(expr->data.expr[0]);
    free(expr->data.expr[1]);
    break;
  case '&':					// boolean AND
		{									// requires pre-evaluation of the expressions, as in C if the first expr is false, the second would not evaluate (hence free)
			boolean e1 = evaluate(expr->data.expr[0]);
			boolean e2 = evaluate(expr->data.expr[1]);
			ret = e1 &&	e2;
#ifdef DEBUG
			SerPrintP("'&' evals "); Serial.print(ret);
			SerPrintP(" - freeing "); Serial.print(expr->op); SerPrintP("\n");
#endif
			free(expr->data.expr[0]);
#ifdef DEBUG
			SerPrintP("freed expr0");
			Serial.print(freeMemory()); SerPrintP(" bytes free.\n");
#endif
			free(expr->data.expr[1]);
#ifdef DEBUG
			SerPrintP("freed expr1");
			Serial.print(freeMemory()); SerPrintP(" bytes free.\n");
#endif
		}
    break;
  case '|':			// boolean OR
		{							// requires pre-evaluation of the expressions to make sure expressions are freed
			boolean e1 = evaluate(expr->data.expr[0]);
			boolean e2 = evaluate(expr->data.expr[1]);
			ret = e1 ||	e2;
			free(expr->data.expr[0]);
			free(expr->data.expr[1]);
		}
		break;
  case '=':
		ret =
			(float)(evaluate(expr->data.expr[0])
			==
			evaluate(expr->data.expr[1]))
			;
		free(expr->data.expr[0]);
		free(expr->data.expr[1]);
		break;
  case '<':
		ret = (float)
			(evaluate(expr->data.expr[0])
			<
			evaluate(expr->data.expr[1]))
			;
		free(expr->data.expr[0]);
		free(expr->data.expr[1]);
		break;
  case '>':
		ret =
			evaluate(expr->data.expr[0])
			>
			evaluate(expr->data.expr[1])
			;
		free(expr->data.expr[0]);
		free(expr->data.expr[1]);
		break;
  case '[':
		ret =
			evaluate(expr->data.expr[0])
			<=
			evaluate(expr->data.expr[1])
			;
		free(expr->data.expr[0]);
		free(expr->data.expr[1]);
		break;
  case ']':
		ret =
			evaluate(expr->data.expr[0])
			>=
			evaluate(expr->data.expr[1])
			;
		free(expr->data.expr[0]);
		free(expr->data.expr[1]);
		break;
  case 'd':
    ret = expr->data.term;
    break;
  default:
    exit(1);
  }
  return ret;
}

int APDEvaluator::dothatstuff(const char *str) {
  Expr* expr = NULL;

  if(str != NULL) {
  	G_STRING_ITERATOR = 0;
  	Serial.print(str); SerPrintP(" = ");
    expr = expression(str);
    //printf("%d\n", evaluate(expr));
    Serial.println(evaluate(expr));
    free(expr);
  }
  return 0;
}

float APDEvaluator::feval(const char *str) {
  Expr* expr = NULL;
  float fret;

  if(str != NULL) {
  	G_STRING_ITERATOR = 0;
#ifdef DEBUG
  	Serial.print(str); SerPrintP(" = ");
#endif
    expr = expression(str);

    fret = evaluate(expr);
#ifdef DEBUG
    Serial.println(fret);
#endif
    free(expr);
  }
  return fret;
}
