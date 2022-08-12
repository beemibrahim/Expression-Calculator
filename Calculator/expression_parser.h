#pragma once
#include "expression.h"

#define VALUE_START_MALLOC 100
#define VALUE_ADD_MALLOC 100

#define DECIMAL_VALUE_START_MALLOC 100
#define DECIMAL_VALUE_ADD_MALLOC 100

#define ANY_START_MALLOC 100
#define ANY_ADD_MALLOC 100

#define MAX_NUMBER_SIZE 500
#define MAX_DEC_NUMBER_SIZE 50

int ParseExpression(char* expression, int expression_size, struct expression* result);

