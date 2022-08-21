#pragma once
#include "expression_parser.h"


#define NUM1 1
#define NUM2 2

int add_number(struct number* number1, struct number* number2, struct number* result, char** error_message);
int subtract_number(struct number* number1, struct number* number2, struct number* result, char** error_message);
int multiply_number(struct number* number1, struct number* number2, struct number* result, char** error_message);
int divide_number(struct number* number1, struct number* number2, struct number* result, char** error_message);