#include "expression_parser.h"

int ParseExpression(char* expression, int expression_size, struct expression* result, char** read_only_readable_error)
{
	/* Parse State */

	/* State concerning if we have encountered a low precedence character in the
	 * current expression */


		char* have_encountered_low_pred = (char*)malloc(expression_size * sizeof(char));

		if (have_encountered_low_pred == NULL) {
			THROW_PARSE_ERROR_(OUT_OF_HEAP_P)
		}

		memset(have_encountered_low_pred, 0, expression_size * sizeof(char));
		*(have_encountered_low_pred) = '1';

	int expression_level = 0; /* The expression level we are on in the variable above */

	/* Booleans */
	int expectValue = 1;        /* Are we expecting a value */
	int expectOpenBracket = 1;  /* Are we expecting an open bracket */
	int expectCloseBracket = 0; /* Are we expecting a close bracket */
	int expectOperator = 0;     /* Are we expecting an operator*/

	/* Counts */
	int expectCloseBrackets = 0; /* The number of close brackets we expect for every open bracket*/

	/* Current Value State */

	/* Are we in a value */
	int inValue = 0;        

	/* Have we encountered a decimal point in the current value were in */
	int inValueDecimal = 0;  

	/* Have we encountered a negative sign at the beginning of our value */
	int InValueNegative = 0; 

	/* Have we encountered a digit in our current value */
	int InValueEncounteredDigit = 0; 

	/* If we need to skip the leading zeros of a number */
	int inValueEncounteredNonZero = 0;

	/* If we have encountered a zero in our current value */
	int inValueEncounteredZero = 0;

	/* We have encountered a negative sign but we haven't encountered a digit */
	int InValueNegativeNoDigit = 0; 

		struct last_value_buffer* last_value_buff = calloc(1, sizeof(struct last_value_buffer));

		if (last_value_buff == NULL) {

			free(have_encountered_low_pred);
			THROW_PARSE_ERROR_(OUT_OF_HEAP_P)
		}

	last_value_buff->buffer_size = EXPRESS_OP_START_SIZE;
	last_value_buff->buffer = malloc(sizeof(struct off_st) * EXPRESS_OP_START_SIZE);

	if (last_value_buff->buffer == NULL) {

		free(have_encountered_low_pred);
		free(last_value_buff);

		THROW_PARSE_ERROR_(OUT_OF_HEAP_P)
	}

	memset(last_value_buff->buffer, 0, EXPRESS_OP_START_SIZE);

	/* Creating the number object */
	struct number* current_number = (struct number*)calloc(1, sizeof(struct number));

	if (current_number == NULL) {

		free(have_encountered_low_pred);
		free_last_value_buff(last_value_buff);

		THROW_PARSE_ERROR_(OUT_OF_HEAP_P)
	}

	/* Allocating dynamic memory for the number */
	current_number->malloced_number = (char*)malloc(VALUE_START_MALLOC); /* The raw number we have retreived so far */

	if (current_number->malloced_number == NULL) {

		free(have_encountered_low_pred);
		free_last_value_buff(last_value_buff);
		free(current_number);

		THROW_PARSE_ERROR_(OUT_OF_HEAP_P)
	}

	/* Allocating dynamic memory for the decimal point number */
	current_number->malloced_decimal_number = (char*)malloc(DECIMAL_VALUE_START_MALLOC); /* The raw decimal number we have retreived so far */

	if (current_number->malloced_decimal_number == NULL) {

		free(have_encountered_low_pred);
		free_last_value_buff(last_value_buff);
		free(current_number->malloced_number);
		free(current_number);

		THROW_PARSE_ERROR_(OUT_OF_HEAP_P)
	}

	*current_number->malloced_number = '\0';
	*current_number->malloced_decimal_number = '\0';

	/* Initalizing the expression object */
	struct expression* expr = calloc(1, sizeof(struct expression));

	if (expr == NULL) {

		free(have_encountered_low_pred);
		free_last_value_buff(last_value_buff);
		free_number(current_number);

		THROW_PARSE_ERROR_(OUT_OF_HEAP_P)

	}


	/* Initalizing the operator's in the expression object */
	expr->has_retrevied_result = malloc(sizeof(struct any) * ANY_START_MALLOC);

	if (expr->has_retrevied_result == NULL) {

		free(have_encountered_low_pred);
		free_last_value_buff(last_value_buff);
		free_number(current_number);
		free(expr);

		THROW_PARSE_ERROR_(OUT_OF_HEAP_P)
	}

	memset(expr->has_retrevied_result, 0, (sizeof(struct any) * ANY_START_MALLOC));

	expr->any_size = ANY_START_MALLOC;

	/* Set the sizes of the numbers */
	current_number->number_size = VALUE_START_MALLOC;
	current_number->decimal_number_size = DECIMAL_VALUE_START_MALLOC;

	/* States containing info about if the next value is the first value in a
	 * preceding open or close bracket */
	int if_start_new_exec = 0;

	/* The write offset for the current number */
	int write_pos = 0;

	/* The write offset for the current decimal number */
	int write_dec_pos = 0;

	/* This variable contains whether or not there were precedence conflicts earlier */
	int is_prec_conf = 0;

	/* If we have wrote anything to an existing decimal place */
	int is_curr_exis_dec_num = 0;

	/* If the last value we encountered was a subexpression */
	int last_value_was_subexpr = 0;

	/* If this pointer is not null, then we have encountered a subexpression as our previous value and have stored the any pointer in this */
	struct any* higher_preced_expr = NULL;

	char last_ch = '\0';

	/* Loop over the expression character by character */
	for (; *expression; last_ch = *expression, expression += 1) {

		/* Have we found an open bracket */
		if (*expression == '(') {

			if (expectOpenBracket == 0) {
				
				/* Free malloced memory */
				free(have_encountered_low_pred);
				free_number(current_number);
				free_last_value_buff(last_value_buff);

				
		        THROW_PARSE_ERROR()
			}

			if (expectOperator == 1) {

				/* Free malloced memory */
				free(have_encountered_low_pred);
				free_number(current_number);
				free_last_value_buff(last_value_buff);


				THROW_PARSE_ERROR()
			}

			/* Set State */
			last_value_was_subexpr = 0;

			/* Set the new state */
			expectOpenBracket = 1;
			expectCloseBracket = 0;

			++if_start_new_exec;
			expectValue = 1;
			expectOperator = 0;

			/* Set the new count */
			expectCloseBrackets += 1;

			/* Start a new expression context */
			expression_level += 1;
			*(have_encountered_low_pred + expression_level) = '1';

			/* Add one value to every exec */
			if (last_value_buff->buffer_offset != 0) {
				struct off_st* loopptr = (last_value_buff->buffer + last_value_buff->buffer_offset - 1);
				for (;; loopptr--) {
					loopptr->value += 1;

					/* Break condition */
					if (loopptr == last_value_buff->buffer) {
						break;
					}

				}
			}

			continue;
		}

		/* Have we found a close bracket */
		if (*expression == ')') {

			if (expectCloseBracket == 0) {

				/* Free malloced memory */
				free(have_encountered_low_pred);
				free_number(current_number);
				free_last_value_buff(last_value_buff);


				THROW_PARSE_ERROR()
			}

			if (expectValue == 1) {

				/* Free malloced memory */
				free(have_encountered_low_pred);
				free_number(current_number);
				free_last_value_buff(last_value_buff);


				THROW_PARSE_ERROR()

			}

			/* Set state */
			expectOpenBracket = 0;

			/* Check if we have encapsulted a value inside two meaningless brackets : (56) */
			if (if_start_new_exec > 0) {
				if_start_new_exec--;
			}

			if (expectCloseBrackets > 0) {
				expectCloseBracket = 1;
			}
			else {
				expectCloseBracket = 0;
			}

			/* If we were in a value and have encountered a close bracket declareing the end of the value */
			if (inValue) {

				/* If the value was - : "-" */
				if (inValueEncounteredZero == 0 && *current_number->malloced_decimal_number == '\0' && *current_number->malloced_number == '\0' && current_number->is_negative == 1 && current_number->is_decimal == 0) {

					/* Free malloced memory */
					free(have_encountered_low_pred);
					free_number(current_number);
					free_last_value_buff(last_value_buff);

					THROW_PARSE_ERROR()

				}

				/* If the value was 0 */
				if (inValueEncounteredNonZero == 0 && *current_number->malloced_number == '\0') {
					*current_number->malloced_number = '0';
					*(current_number->malloced_number + 1) = '\0';

					current_number->is_number_dig = 0;
					/* If the current number is "-0" which doesn't make sense
					 * so we just turn it to "0" */

					current_number->is_negative = 0;
				}

				/* If the value had a decimal point with no decimal numbers after it */
				if (current_number->is_decimal == 1 && (*(current_number->malloced_decimal_number) == '\0')) {
					current_number->is_decimal = 0;
				}

				/* Clear State */
				expectValue = 0;
				inValue = 0;
				inValueDecimal = 0;
				InValueNegative = 0;
				InValueNegative = 0;
				InValueEncounteredDigit = 0;
				InValueNegativeNoDigit = 0;

				/* Set new State */
				expectOperator = 1;

				/* We have reached the operator size cap */
				if (expr->any_write_offset + 1 == expr->any_size) {
					struct any* malloced_str = realloc(expr->has_retrevied_result, (expr->any_size += ANY_ADD_MALLOC) * sizeof(struct any));

					if (malloced_str == NULL) {
						
						/* Free malloced memory */
						free(have_encountered_low_pred);
						free_number(current_number);
						free_last_value_buff(last_value_buff);

						THROW_PARSE_ERROR_(OUT_OF_HEAP_P)
					}

					expr->has_retrevied_result = malloced_str;

					memset(expr->has_retrevied_result + (expr->any_size - (sizeof(struct any) * ANY_ADD_MALLOC)), 0, ANY_ADD_MALLOC * sizeof(struct any));
				}

				/* Create a copy of the current number object */
				struct number* add_number = copy_number(current_number, &is_curr_exis_dec_num);

				is_curr_exis_dec_num = 0;

				/* Empty the contents of the current number*/
				reuse_number(&current_number);

				(expr->has_retrevied_result + expr->any_write_offset)->is_number = 1;
				(expr->has_retrevied_result + expr->any_write_offset)->the_number =
					add_number;

				if (if_start_new_exec > 0) {
					(expr->has_retrevied_result + expr->any_write_offset)
						->is_start_new_exec = if_start_new_exec;


					/* If we have reached the end of our buffer size */
					if (last_value_buff->buffer_offset + 1 == last_value_buff->buffer_size) {
						struct off_st* malloced_str = realloc(last_value_buff->buffer , (last_value_buff->buffer_size + EXPRESS_OP_ADD_SIZE) * sizeof(struct off_st));

						if (malloced_str == NULL) {

							/* Free malloced memory */
							free(have_encountered_low_pred);
							free_number(current_number);
							free_last_value_buff(last_value_buff);

							THROW_PARSE_ERROR_(OUT_OF_HEAP_P)
						}

						last_value_buff->buffer = malloced_str;

						last_value_buff->buffer_size += EXPRESS_OP_ADD_SIZE;
					}

					/* Add current expression offset to the buffer */
					(last_value_buff->buffer + last_value_buff->buffer_offset)->offset = expr->any_write_offset;
					(last_value_buff->buffer + last_value_buff->buffer_offset)->value = if_start_new_exec;
					last_value_buff->buffer_offset += 1;


					if_start_new_exec = 0;
				}

				expr->any_write_offset += 1;

				/* If the amount of unused space is over MAX_NUMBER_SIZE */
				if ((add_number->number_size - write_pos) > MAX_NUMBER_SIZE) {
					char* malloced_str = realloc(add_number->malloced_number, write_pos + 1);

					if (malloced_str == NULL) {

						/* Free malloced memory */
						free(have_encountered_low_pred);
						free_number(current_number);
						free_last_value_buff(last_value_buff);

						THROW_PARSE_ERROR_(OUT_OF_HEAP_P)
					}

					add_number->malloced_number = malloced_str;


					add_number->number_size = write_pos + 1;
				}

				/* If the amount of unused space is over MAX_DEC_NUMBER_SIZE */
				if ((add_number->decimal_number_size - write_dec_pos) > MAX_DEC_NUMBER_SIZE) {
					char* malloced_str = realloc(add_number->malloced_decimal_number, write_dec_pos + 1);

					if (malloced_str == NULL) {

						/* Free malloced memory */
						free(have_encountered_low_pred);
						free_number(current_number);
						free_last_value_buff(last_value_buff);

						THROW_PARSE_ERROR_(OUT_OF_HEAP_P)
					}

					add_number->malloced_decimal_number = malloced_str;

					add_number->decimal_number_size = write_dec_pos + 1;
				}

				/* Reset number write offsets */
				write_pos = 0;
				write_dec_pos = 0;

			}

			/* We will always decrement one exec, another exec could be decremented by high precedence operators */
			int value_to_dec = 1;

			/* If we have reached this close bracket and need to close our previous exec that was started by higher precedence operators */
			if (*(have_encountered_low_pred + expression_level) == '0') {

				(expr->has_retrevied_result + (expr->any_write_offset - 1))
					->is_end_new_exec += 1;

				value_to_dec += 1;
			}

			/* Add the normal exec */
			(expr->has_retrevied_result + (expr->any_write_offset - 1))
				->is_end_new_exec += 1;

			/* Remove one value from every exec */
			if (last_value_buff->buffer_offset != 0) {
				int removed = 0;

				struct off_st* loopptr = (last_value_buff->buffer + last_value_buff->buffer_offset - 1);
				for (int i = 0;i < last_value_buff->buffer_offset; --loopptr,++i) {
					loopptr->value -= value_to_dec;

					/* We have closed an expression, set "higher_preced_expr" to its appropriate value */
					if ((loopptr == (last_value_buff->buffer + last_value_buff->buffer_offset - 1)) && loopptr->value == 0) {

						/* Set it */
						higher_preced_expr = (expr->has_retrevied_result + loopptr->offset);

						/* Reuse the current "last_value_buff" element */
						loopptr->value = 0;
						loopptr->offset = 0;
						removed += 1;
					}
				}

				last_value_buff->buffer_offset -= removed;
			}

			/* Close this bracket with a close exec */

			expectValue = 0;
			expectOperator = 1;

			/* Set the new count */
			expectCloseBrackets -= 1;

			/* Start a new expression context */
			*(have_encountered_low_pred + expression_level) = '1';
			expression_level -= 1;

			/* Set new state */
			last_value_was_subexpr = 1;

			continue;
		}

		/* If we expect an operator */
		if (expectOperator == 1) {

			expectOpenBracket = 0;
			expectCloseBracket = 1;

			/* If we expect an operator and the operator is a plus */
			if (*expression == '+') {

			    /* If we were expecting a high precedense operator, then reset the "higher_preced_expr" variable */
				higher_preced_expr = NULL;

				/* Check if we have looped through a series of high precedence operators */
				if (is_prec_conf == 1) {

					(expr->has_retrevied_result + (expr->any_write_offset - 1))->is_end_new_exec += 1;

					/* Remove one value from every exec */
					if (last_value_buff->buffer_offset != 0) {
						int removed = 0;

						struct off_st* loopptr = (last_value_buff->buffer + last_value_buff->buffer_offset - 1);
						for (int i = 0;i < last_value_buff->buffer_offset; --loopptr,++i) {
							loopptr->value -= 1;

							/* We have closed an expression, set "higher_preced_expr" to its appropriate value */
							if ((loopptr == (last_value_buff->buffer + last_value_buff->buffer_offset - 1)) && loopptr->value == 0) {

								/* Set it */
								higher_preced_expr = (expr->has_retrevied_result + loopptr->offset);

								/* Reuse the current "last_value_buff" element */
								loopptr->value = 0;
								loopptr->offset = 0;
								removed += 1;
							}
						}

						last_value_buff->buffer_offset -= removed;
					}
					is_prec_conf = 0;

				}

				/* Set precedence state */
				*(have_encountered_low_pred + expression_level) = '1';

				/* Set new state */
				expectOperator = 0;
				expectValue = 1;

				/* Add the operator to the expression */
				(expr->has_retrevied_result + expr->any_write_offset)->is_operator = 1;

				struct op* add_oper = malloc(sizeof(struct op));

				if (add_oper == NULL) {

					free(have_encountered_low_pred);
					free_number(current_number);
					free_last_value_buff(last_value_buff);

					THROW_PARSE_ERROR_(OUT_OF_HEAP_P)
				}

				add_oper->op = ADD;

				(expr->has_retrevied_result + expr->any_write_offset)->the_op = add_oper;

				expr->any_write_offset += 1;

				/* Set State */
				last_value_was_subexpr = 0;

				expectOpenBracket = 1;
				expectCloseBracket = 0;

				continue;

			}

			/* If we expect an operator and the operator is a minus */
			if (*expression == '-') {

				/* If we were expecting a high precedense operator, then reset the "higher_preced_expr" variable */
				higher_preced_expr = NULL;

				/* Check if we have looped through a series of high precedence operators */
				if (is_prec_conf == 1) {

					(expr->has_retrevied_result + (expr->any_write_offset - 1))->is_end_new_exec += 1;

					/* Remove one value from every exec */
					if (last_value_buff->buffer_offset != 0) {
						int removed = 0;

						struct off_st* loopptr = (last_value_buff->buffer + last_value_buff->buffer_offset - 1);
						for (int i = 0;i < last_value_buff->buffer_offset; --loopptr,++i) {
							loopptr->value -= 1;

							/* We have closed an expression, set "higher_preced_expr" to its appropriate value */
							if ((loopptr == (last_value_buff->buffer + last_value_buff->buffer_offset - 1)) && loopptr->value == 0) {

								/* Set it */
								higher_preced_expr = (expr->has_retrevied_result + loopptr->offset);

								/* Reuse the current "last_value_buff" element */
								loopptr->value = 0;
								loopptr->offset = 0;

								removed += 1;
							}
						}

						last_value_buff->buffer_offset -= removed;
					}
					is_prec_conf = 0;

				}

				/* Set precedence state */
				*(have_encountered_low_pred + expression_level) = '1';

				/* Set new state */
				expectOperator = 0;
				expectValue = 1;

				/* Add the operator to the expression */
				(expr->has_retrevied_result + expr->any_write_offset)->is_operator = 1;

				struct op* add_oper = malloc(sizeof(struct op));

				if (add_oper == NULL) {

					free(have_encountered_low_pred);
					free_number(current_number);
					free_last_value_buff(last_value_buff);

					THROW_PARSE_ERROR_(OUT_OF_HEAP_P)
				}

				add_oper->op = SUBTRACT;

				(expr->has_retrevied_result + expr->any_write_offset)->the_op = add_oper;

				expr->any_write_offset += 1;

				/* Set State */
				last_value_was_subexpr = 0;

				expectOpenBracket = 1;
				expectCloseBracket = 0;

				continue;
			}

			/* If we expect an operator and the operator is a times */
			if (*expression == '*') {

				/* Check for precedence conflicts */
				if (*(have_encountered_low_pred + expression_level) == '1') {

					*(have_encountered_low_pred + expression_level) = '0';

					/* If the last value was a subexpression, then start an exec at the beginning of the previous subexpression */
					if (higher_preced_expr != NULL && last_value_was_subexpr == 1) {

						 /* Add the new exec and reuse "higher_preced_expr" */
						higher_preced_expr->is_start_new_exec += 1;
						higher_preced_expr = NULL;

				    }
				    else {

					(expr->has_retrevied_result + (expr->any_write_offset - 1))->is_start_new_exec += 1;

			        }     

					/* Add one value from every exec */
					if (last_value_buff->buffer_offset != 0) {
						struct off_st* loopptr = (last_value_buff->buffer + last_value_buff->buffer_offset - 1);
						for (;; loopptr--) {
							loopptr->value += 1;

							/* Break condition */
							if (loopptr == last_value_buff->buffer) {
								break;
							}

						}
					}

					is_prec_conf = 1;
				}
				else {

					*(have_encountered_low_pred + expression_level) = '0';
				}

				/* Set new state */
				expectOperator = 0;
				expectValue = 1;

				/* Add the operator to the expression */
				(expr->has_retrevied_result + expr->any_write_offset)->is_operator = 1;

				struct op* add_oper = malloc(sizeof(struct op));

				if (add_oper == NULL) {

					free(have_encountered_low_pred);
					free_number(current_number);
					free_last_value_buff(last_value_buff);

					THROW_PARSE_ERROR_(OUT_OF_HEAP_P)
				}

				add_oper->op = MULTILPLY;

				(expr->has_retrevied_result + expr->any_write_offset)->the_op = add_oper;

				expr->any_write_offset += 1;

				/* Set State */
				last_value_was_subexpr = 0;

				expectOpenBracket = 1;
				expectCloseBracket = 0;

				continue;
			}

			/* If we expect an operator and the operator is a divide */
			if (*expression == '/') {

				/* Check for precedence conflicts */
				if (*(have_encountered_low_pred + expression_level) == '1') {

					*(have_encountered_low_pred + expression_level) = '0';

					/* If the last value was a subexpression, then start an exec at the beginning of the previous subexpression */
					if (higher_preced_expr != NULL && last_value_was_subexpr == 1) {

						/* Add the new exec and reuse "higher_preced_expr" */
						higher_preced_expr->is_start_new_exec += 1;
						higher_preced_expr = NULL;

					}
					else {

						(expr->has_retrevied_result + (expr->any_write_offset - 1))->is_start_new_exec += 1;

					}

					/* Add one value from every exec */
					if (last_value_buff->buffer_offset != 0) {
						struct off_st* loopptr = (last_value_buff->buffer + last_value_buff->buffer_offset - 1);
						for (;; loopptr--) {
							loopptr->value += 1;

							if (loopptr == last_value_buff->buffer) {
								break;
							}
						}
					}
					is_prec_conf = 1;
				}
				else {

					*(have_encountered_low_pred + expression_level) = '0';
				}

				/* Set new state */
				expectOperator = 0;
				expectValue = 1;

				/* Add the operator to the expression */
				(expr->has_retrevied_result + expr->any_write_offset)->is_operator = 1;

				struct op* add_oper = malloc(sizeof(struct op));

				if (add_oper == NULL) {

					free(have_encountered_low_pred);
					free_number(current_number);
					free_last_value_buff(last_value_buff);

					THROW_PARSE_ERROR_(OUT_OF_HEAP_P)
				}

				add_oper->op = DIVIDE;

				(expr->has_retrevied_result + expr->any_write_offset)->the_op = add_oper;

				expr->any_write_offset += 1;

				/* Set State */
				last_value_was_subexpr = 0;

				expectOpenBracket = 1;
				expectCloseBracket = 0;

				continue;
			}

			/* If there is an invalid operator amogus */
			if (*expression != ' ') {

				/* Free malloced memory */
				free(have_encountered_low_pred);
				free_number(current_number);
				free_last_value_buff(last_value_buff);

				THROW_PARSE_ERROR()
			}
		}

		/* We are in a value */
		if (inValue == 1) {

			/* We are on a digit that is part of a number */
			if (isdigit(*expression)) {
				InValueEncounteredDigit = 1;
				InValueNegativeNoDigit = 0;

				/**
				Add the current character to the number.
				If you are in the decimal part of the current number then add the character to the decimal number.
				If not add it to the main number.
				*/
				if (current_number->is_decimal == 1) {

					is_curr_exis_dec_num = 1;

					if (write_dec_pos == current_number->decimal_number_size - 1) {

						// Incresing the numbers value
						current_number->decimal_number_size += DECIMAL_VALUE_ADD_MALLOC;

						// Reallocating and Reinitalizing the new memory
						char* malloced_str = (char*)realloc(current_number->malloced_decimal_number, current_number->decimal_number_size); // The raw number we have retreived so far

						if (malloced_str == NULL) {

							/* Free malloced memory */
							free(have_encountered_low_pred);
							free_number(current_number);
							free_last_value_buff(last_value_buff);

							THROW_PARSE_ERROR_(OUT_OF_HEAP_P)
						}

						current_number->malloced_decimal_number = malloced_str;

						memset(current_number->malloced_decimal_number + (current_number->decimal_number_size - DECIMAL_VALUE_ADD_MALLOC), 0, DECIMAL_VALUE_ADD_MALLOC * sizeof(char));
					}

					*(current_number->malloced_decimal_number + write_dec_pos) = *expression;
					write_dec_pos += 1;
					*(current_number->malloced_decimal_number + write_dec_pos) = '\0';

				}
				else {


					if (write_pos == current_number->number_size - 1) {

						// Incresing the numbers value
						current_number->number_size += VALUE_ADD_MALLOC;

						// Reallocating and Reinitalizing the new memory
						char* malloced_str = (char*)realloc(current_number->malloced_number, current_number->number_size); // The raw number we have retreived so far

						if (malloced_str == NULL) {

							/* Free malloced memory */
							free(have_encountered_low_pred);
							free_number(current_number);
							free_last_value_buff(last_value_buff);

							THROW_PARSE_ERROR_(OUT_OF_HEAP_P)
						}

						current_number->malloced_number = malloced_str;


						memset(current_number->malloced_number + (current_number->number_size - VALUE_ADD_MALLOC), 0, VALUE_ADD_MALLOC * sizeof(char));
					}

					if (inValueEncounteredNonZero == 1) {

						*(current_number->malloced_number + write_pos) = *expression;
						write_pos += 1;
						*(current_number->malloced_number + write_pos) = '\0';

					}
					else if (inValueEncounteredNonZero == 0 && *expression != '0') {

						*(current_number->malloced_number + write_pos) = *expression;
						write_pos += 1;
						*(current_number->malloced_number + write_pos) = '\0';

					}

					/* If we have encountered a zero in our value */
					if (*expression == '0') {
						inValueEncounteredZero = 1;
					}

				}

				continue;
			}
			/* We have encountered a decimal point */
			else if (*expression == '.') {

				if (inValueDecimal == 1) {

					/* Free malloced memory */
					free(have_encountered_low_pred);
					free_number(current_number);
					free_last_value_buff(last_value_buff);

					THROW_PARSE_ERROR()
				}

				inValueDecimal = 1;

				/* If there wasn't any digit previously */
				if (InValueEncounteredDigit == 0) {
					current_number->is_number_dig = 1;
				}

				current_number->is_decimal = 1;
				continue;
			}
			/* We have encountered a space but we haven't encountered any digit
			   inside the value were in. This means that we probably are inbetween a
			   negative sign and a number, this is a special case that should be
			   valid.

			   We will loop to the next character, since all spaces are turned into
			   one space, the next loop should bring us to the next character */
			else if (*expression == ' ' && InValueNegativeNoDigit == 1) {
				continue;
			}
			/* If we have encountered a space, then we have reached the end of the
			   value */
			else if (*expression == ' ') {


			/* If the value was - : "-" */
			if (inValueEncounteredZero == 0 && *current_number->malloced_decimal_number == '\0' && *current_number->malloced_number == '\0' && current_number->is_negative == 1 && current_number->is_decimal == 0) {

				/* Free malloced memory */
				free(have_encountered_low_pred);
				free_number(current_number);
				free_last_value_buff(last_value_buff);

				THROW_PARSE_ERROR()

			}

			/* If the value was 0 */
			if (inValueEncounteredNonZero == 0 && *current_number->malloced_number == '\0') {
				*current_number->malloced_number = '0';
				*(current_number->malloced_number + 1) = '\0';

				current_number->is_number_dig = 0;
				/* If the current number is "-0" which doesn't make sense
				 * so we just turn it to "0" */

				current_number->is_negative = 0;
			}

			/* If the value had a decimal point with no decimal numbers after it */
			if (current_number->is_decimal == 1 && (*(current_number->malloced_decimal_number) == '\0')) {
				current_number->is_decimal = 0;
			}


				/* Clear State */
				expectValue = 0;
				inValue = 0;
				inValueDecimal = 0;
				InValueNegative = 0;
				InValueNegative = 0;
				InValueEncounteredDigit = 0;
				InValueNegativeNoDigit = 0;

				/* Set new State */

				expectOperator = 1;

				expectOpenBracket = 0;
				expectCloseBracket = 1;

				if (expectCloseBrackets > 0) {
					expectCloseBracket = 1;
				}
				else {
					expectCloseBracket = 0;
				}

				/* We have reached the operator size cap */
				if (expr->any_write_offset + 1 == expr->any_size) {
					struct any* malloced_str = realloc(expr->has_retrevied_result, (expr->any_size += ANY_ADD_MALLOC) * sizeof(struct any));

					if (malloced_str == NULL) {

						/* Free malloced memory */
						free(have_encountered_low_pred);
						free_number(current_number);
						free_last_value_buff(last_value_buff);

						THROW_PARSE_ERROR_(OUT_OF_HEAP_P)
					}

					expr->has_retrevied_result = malloced_str;

					memset(expr->has_retrevied_result + (expr->any_size - (sizeof(struct any) * ANY_ADD_MALLOC)), 0, ANY_ADD_MALLOC * sizeof(struct any));
				}

				/* Create a copy of the current number object */
				struct number* add_number = copy_number(current_number, &is_curr_exis_dec_num);

				is_curr_exis_dec_num = 0;

				/* Empty the contents of the current number*/
				reuse_number(&current_number);

				(expr->has_retrevied_result + expr->any_write_offset)->is_number = 1;
				(expr->has_retrevied_result + expr->any_write_offset)->the_number =
					add_number;

				if (if_start_new_exec > 0) {
					(expr->has_retrevied_result + expr->any_write_offset)
						->is_start_new_exec = if_start_new_exec;


					/* If we have reached the end of our buffer size */
					if (last_value_buff->buffer_offset + 1 == last_value_buff->buffer_size) {
						struct off_st* malloced_str = realloc(last_value_buff->buffer, (last_value_buff->buffer_size + EXPRESS_OP_ADD_SIZE) * sizeof(struct off_st));

						if (malloced_str == NULL) {

							/* Free malloced memory */
							free(have_encountered_low_pred);
							free_number(current_number);
							free_last_value_buff(last_value_buff);

							THROW_PARSE_ERROR_(OUT_OF_HEAP_P)
						}

						last_value_buff->buffer = malloced_str;

						last_value_buff->buffer_size += EXPRESS_OP_ADD_SIZE;
					}

					/* Add current expression offset to the buffer */
					(last_value_buff->buffer + last_value_buff->buffer_offset)->offset = expr->any_write_offset;
					(last_value_buff->buffer + last_value_buff->buffer_offset)->value = if_start_new_exec;
					last_value_buff->buffer_offset += 1;


					if_start_new_exec = 0;
				}

				expr->any_write_offset += 1;

				/* If the amount of unused space is over MAX_NUMBER_SIZE */
				if ((add_number->number_size - write_pos) > MAX_NUMBER_SIZE) {
					char* malloced_str = realloc(add_number->malloced_number, write_pos + 1);

					if (malloced_str == NULL) {

						/* Free malloced memory */
						free(have_encountered_low_pred);
						free_number(current_number);
						free_last_value_buff(last_value_buff);

						THROW_PARSE_ERROR_(OUT_OF_HEAP_P)
					}

					add_number->malloced_number = malloced_str;

					add_number->number_size = write_pos + 1;
				}

				/* If the amount of unused space is over MAX_DEC_NUMBER_SIZE */
				if ((add_number->decimal_number_size - write_dec_pos) > MAX_DEC_NUMBER_SIZE) {
					char* malloced_str = realloc(add_number->malloced_decimal_number, write_dec_pos + 1);

					if (malloced_str == NULL) {

						/* Free malloced memory */
						free(have_encountered_low_pred);
						free_number(current_number);
						free_last_value_buff(last_value_buff);

						THROW_PARSE_ERROR_(OUT_OF_HEAP_P)
					}

					add_number->malloced_decimal_number = malloced_str;

					add_number->decimal_number_size = write_dec_pos + 1;
				}

				/* Reset number write offsets */
				write_pos = 0;
				write_dec_pos = 0;

				/* Set State */
				last_value_was_subexpr = 0;

				continue;
			}
			/* If we have encountered an add operator, then we have reached the end
			   of the value */
			else if (*expression == '+') {


			/* If the value was - : "-" */
			if (inValueEncounteredZero == 0 && *current_number->malloced_decimal_number == '\0' && *current_number->malloced_number == '\0' && current_number->is_negative == 1 && current_number->is_decimal == 0) {

				/* Free malloced memory */
				free(have_encountered_low_pred);
				free_number(current_number);
				free_last_value_buff(last_value_buff);

				THROW_PARSE_ERROR()

			}

			/* If the value was 0 */
			if (inValueEncounteredNonZero == 0 && *current_number->malloced_number == '\0') {
				*current_number->malloced_number = '0';
				*(current_number->malloced_number + 1) = '\0';

				current_number->is_number_dig = 0;
				/* If the current number is "-0" which doesn't make sense
				 * so we just turn it to "0" */

				current_number->is_negative = 0;
			}

			/* If the value had a decimal point with no decimal numbers after it */
			if (current_number->is_decimal == 1 && (*(current_number->malloced_decimal_number) == '\0')) {
				current_number->is_decimal = 0;
			}


			/* If we were expecting a high precedense operator, then reset the "higher_preced_expr" variable */
			higher_preced_expr = NULL;

				/* Clear State */
				expectValue = 0;
				inValue = 0;
				inValueDecimal = 0;
				InValueNegative = 0;
				InValueNegative = 0;
				InValueEncounteredDigit = 0;
				InValueNegativeNoDigit = 0;

				/* Set new State */
				expectValue = 1;

				expectOpenBracket = 1;
				expectCloseBracket = 0;

				/* Set precedence state */
				*(have_encountered_low_pred + expression_level) = '1';

				if (expectCloseBrackets > 0) {
					expectCloseBracket = 1;
				}
				else {
					expectCloseBracket = 0;
				}

				/* We have reached the operator size cap */
				if (expr->any_write_offset + 1 == expr->any_size) {
					struct any* malloced_str = realloc(expr->has_retrevied_result, (expr->any_size += ANY_ADD_MALLOC) * sizeof(struct any));

					if (malloced_str == NULL) {

						/* Free malloced memory */
						free(have_encountered_low_pred);
						free_number(current_number);
						free_last_value_buff(last_value_buff);

						THROW_PARSE_ERROR_(OUT_OF_HEAP_P)
					}

					expr->has_retrevied_result = malloced_str;

					memset(expr->has_retrevied_result + (expr->any_size - (sizeof(struct any) * ANY_ADD_MALLOC)), 0, ANY_ADD_MALLOC * sizeof(struct any));
				}

				/* Check if we have looped through a series of high precedence operators */
				if (is_prec_conf == 1) {

					(expr->has_retrevied_result + (expr->any_write_offset))->is_end_new_exec += 1;

					/* Remove one value from every exec */
					if (last_value_buff->buffer_offset != 0) {
						int removed = 0;

						struct off_st* loopptr = (last_value_buff->buffer + last_value_buff->buffer_offset - 1);
						for (int i = 0; i < last_value_buff->buffer_offset; loopptr--,++i) {
							loopptr->value -= 1;

							/* We have closed an expression, set "higher_preced_expr" to its appropriate value */
							if ((loopptr == (last_value_buff->buffer + last_value_buff->buffer_offset - 1)) && loopptr->value == 0) {

								/* Set it */
								higher_preced_expr = (expr->has_retrevied_result + loopptr->offset);

								/* Reuse the current "last_value_buff" element */
								loopptr->value = 0;
								loopptr->offset = 0;

								removed += 1;
							}
						}

						last_value_buff->buffer_offset -= removed;
					}
					is_prec_conf = 0;

				}

				/* Create a copy of the current number object */
				struct number* add_number = copy_number(current_number, &is_curr_exis_dec_num);

				is_curr_exis_dec_num = 0;

				/* Empty the contents of the current number*/
				reuse_number(&current_number);

				(expr->has_retrevied_result + expr->any_write_offset)->is_number = 1;
				(expr->has_retrevied_result + expr->any_write_offset)->the_number =
					add_number;

				if (if_start_new_exec > 0) {
					(expr->has_retrevied_result + expr->any_write_offset)
						->is_start_new_exec = if_start_new_exec;


					/* If we have reached the end of our buffer size */
					if (last_value_buff->buffer_offset + 1 == last_value_buff->buffer_size) {
						struct off_st* malloced_str = realloc(last_value_buff->buffer, (last_value_buff->buffer_size + EXPRESS_OP_ADD_SIZE) * sizeof(struct off_st));

						if (malloced_str == NULL) {

							/* Free malloced memory */
							free(have_encountered_low_pred);
							free_number(current_number);
							free_last_value_buff(last_value_buff);

							THROW_PARSE_ERROR_(OUT_OF_HEAP_P)
						}

						last_value_buff->buffer = malloced_str;

						last_value_buff->buffer_size += EXPRESS_OP_ADD_SIZE;
					}

					/* Add current expression offset to the buffer */
					(last_value_buff->buffer + last_value_buff->buffer_offset)->offset = expr->any_write_offset;
					(last_value_buff->buffer + last_value_buff->buffer_offset)->value = if_start_new_exec;
					last_value_buff->buffer_offset += 1;


					if_start_new_exec = 0;
				}

				expr->any_write_offset += 1;

				/* If the amount of unused space is over MAX_NUMBER_SIZE */
				if ((add_number->number_size - write_pos) > MAX_NUMBER_SIZE) {
					char* malloced_str = realloc(add_number->malloced_number, write_pos + 1);

					if (malloced_str == NULL) {

						/* Free malloced memory */
						free(have_encountered_low_pred);
						free_number(current_number);
						free_last_value_buff(last_value_buff);

						THROW_PARSE_ERROR_(OUT_OF_HEAP_P)
					}

					add_number->malloced_number = malloced_str;

					add_number->number_size = write_pos + 1;
				}

				/* If the amount of unused space is over MAX_DEC_NUMBER_SIZE */
				if ((add_number->decimal_number_size - write_dec_pos) > MAX_DEC_NUMBER_SIZE) {
					char* malloced_str = realloc(add_number->malloced_decimal_number, write_dec_pos + 1);

					if (malloced_str == NULL) {

						/* Free malloced memory */
						free(have_encountered_low_pred);
						free_number(current_number);
						free_last_value_buff(last_value_buff);

						THROW_PARSE_ERROR_(OUT_OF_HEAP_P)
					}

					add_number->malloced_decimal_number = malloced_str;

					add_number->decimal_number_size = write_dec_pos + 1;
				}

				/* Reset number write offsets */
				write_pos = 0;
				write_dec_pos = 0;


				/* Add the operator to the expression */
				(expr->has_retrevied_result + expr->any_write_offset)->is_operator = 1;

				struct op* add_oper = malloc(sizeof(struct op));

				if (add_oper == NULL) {

					free(have_encountered_low_pred);
					free_number(current_number);
					free_last_value_buff(last_value_buff);

					THROW_PARSE_ERROR_(OUT_OF_HEAP_P)
				}

				add_oper->op = ADD;

				(expr->has_retrevied_result + expr->any_write_offset)->the_op = add_oper;

				expr->any_write_offset += 1;

				/* Set State */
				last_value_was_subexpr = 0;

				continue;
			}
			/* If we have encountered a subtract operator, then we have reached the
			   end of the value */
			else if (*expression == '-') {


			
			/* If the value was - : "-" */
			if (inValueEncounteredZero == 0 && *current_number->malloced_decimal_number == '\0' && *current_number->malloced_number == '\0' && current_number->is_negative == 1 && current_number->is_decimal == 0) {

				/* Free malloced memory */
				free(have_encountered_low_pred);
				free_number(current_number);
				free_last_value_buff(last_value_buff);

				THROW_PARSE_ERROR()

			}

			/* If the value was 0 */
			if (inValueEncounteredNonZero == 0 && *current_number->malloced_number == '\0') {
				*current_number->malloced_number = '0';
				*(current_number->malloced_number + 1) = '\0';

				current_number->is_number_dig = 0;
				/* If the current number is "-0" which doesn't make sense
				 * so we just turn it to "0" */

				current_number->is_negative = 0;
			}

			/* If the value had a decimal point with no decimal numbers after it */
			if (current_number->is_decimal == 1 && (*(current_number->malloced_decimal_number) == '\0')) {
				current_number->is_decimal = 0;
			}

			/* If we were expecting a high precedense operator, then reset the "higher_preced_expr" variable */
			higher_preced_expr = NULL;

				/* Clear State */
				expectValue = 0;
				inValue = 0;
				inValueDecimal = 0;
				InValueNegative = 0;
				InValueNegative = 0;
				InValueEncounteredDigit = 0;
				InValueNegativeNoDigit = 0;

				/* Set new State */
				expectValue = 1;

				expectOpenBracket = 1;
				expectCloseBracket = 0;

				/* Set precedence state */
				*(have_encountered_low_pred + expression_level) = '1';

				if (expectCloseBrackets > 0) {
					expectCloseBracket = 1;
				}
				else {
					expectCloseBracket = 0;
				}


				/* We have reached the operator size cap */
				if (expr->any_write_offset + 1 == expr->any_size) {
					struct any* malloced_str = realloc(expr->has_retrevied_result, (expr->any_size += ANY_ADD_MALLOC) * sizeof(struct any));

					if (malloced_str == NULL) {

						/* Free malloced memory */
						free(have_encountered_low_pred);
						free_number(current_number);
						free_last_value_buff(last_value_buff);

						THROW_PARSE_ERROR_(OUT_OF_HEAP_P)
					}

					expr->has_retrevied_result = malloced_str;

					memset(expr->has_retrevied_result + (expr->any_size - (sizeof(struct any) * ANY_ADD_MALLOC)), 0, ANY_ADD_MALLOC * sizeof(struct any));
				}

				/* Check if we have looped through a series of high precedence operators */
				if (is_prec_conf == 1) {

					(expr->has_retrevied_result + (expr->any_write_offset))->is_end_new_exec += 1;

					/* Remove one value from every exec */
					if (last_value_buff->buffer_offset != 0) {
						int removed = 0;

						struct off_st* loopptr = (last_value_buff->buffer + last_value_buff->buffer_offset - 1);
						for (int i = 0;i < last_value_buff->buffer_offset; loopptr--,++i) {
							loopptr->value -= 1;

							/* We have closed an expression, set "higher_preced_expr" to its appropriate value */
							if ((loopptr == (last_value_buff->buffer + last_value_buff->buffer_offset - 1)) && loopptr->value == 0) {

								/* Set it */
								higher_preced_expr = (expr->has_retrevied_result + loopptr->offset);

								/* Reuse the current "last_value_buff" element */
								loopptr->value = 0;
								loopptr->offset = 0;

								removed += 1;
							}
						}

						last_value_buff->buffer_offset -= removed;
					}
					is_prec_conf = 0;

				}

				/* Create a copy of the current number object */
				struct number* add_number = copy_number(current_number, &is_curr_exis_dec_num);

				is_curr_exis_dec_num = 0;

				/* Empty the contents of the current number*/
				reuse_number(&current_number);

				(expr->has_retrevied_result + expr->any_write_offset)->is_number = 1;
				(expr->has_retrevied_result + expr->any_write_offset)->the_number =
					add_number;

				if (if_start_new_exec > 0) {
					(expr->has_retrevied_result + expr->any_write_offset)
						->is_start_new_exec = if_start_new_exec;


					/* If we have reached the end of our buffer size */
					if (last_value_buff->buffer_offset + 1 == last_value_buff->buffer_size) {
						struct off_st* malloced_str = realloc(last_value_buff->buffer, (last_value_buff->buffer_size + EXPRESS_OP_ADD_SIZE) * sizeof(struct off_st));

						if (malloced_str == NULL) {

							/* Free malloced memory */
							free(have_encountered_low_pred);
							free_number(current_number);
							free_last_value_buff(last_value_buff);

							THROW_PARSE_ERROR_(OUT_OF_HEAP_P)
						}

						last_value_buff->buffer = malloced_str;

						last_value_buff->buffer_size += EXPRESS_OP_ADD_SIZE;
					}

					/* Add current expression offset to the buffer */
					(last_value_buff->buffer + last_value_buff->buffer_offset)->offset = expr->any_write_offset;
					(last_value_buff->buffer + last_value_buff->buffer_offset)->value = if_start_new_exec;
					last_value_buff->buffer_offset += 1;


					if_start_new_exec = 0;
				}

				expr->any_write_offset += 1;


				/* If the amount of unused space is over MAX_NUMBER_SIZE */
				if ((add_number->number_size - write_pos) > MAX_NUMBER_SIZE) {
					char* malloced_str = realloc(add_number->malloced_number, write_pos + 1);

					if (malloced_str == NULL) {

						/* Free malloced memory */
						free(have_encountered_low_pred);
						free_number(current_number);
						free_last_value_buff(last_value_buff);

						THROW_PARSE_ERROR_(OUT_OF_HEAP_P)
					}

					add_number->malloced_number = malloced_str;

					add_number->number_size = write_pos + 1;
				}

				/* If the amount of unused space is over MAX_DEC_NUMBER_SIZE */
				if ((add_number->decimal_number_size - write_dec_pos) > MAX_DEC_NUMBER_SIZE) {
					char* malloced_str = realloc(add_number->malloced_decimal_number, write_dec_pos + 1);

					if (malloced_str == NULL) {

						/* Free malloced memory */
						free(have_encountered_low_pred);
						free_number(current_number);
						free_last_value_buff(last_value_buff);

						THROW_PARSE_ERROR_(OUT_OF_HEAP_P)
					}

					add_number->malloced_decimal_number = malloced_str;

					add_number->decimal_number_size = write_dec_pos + 1;
				}

				/* Reset number write offsets */
				write_pos = 0;
				write_dec_pos = 0;


				/* Add the operator to the expression */
				(expr->has_retrevied_result + expr->any_write_offset)->is_operator = 1;

				struct op* add_oper = malloc(sizeof(struct op));

				if (add_oper == NULL) {

					free(have_encountered_low_pred);
					free_number(current_number);
					free_last_value_buff(last_value_buff);

					THROW_PARSE_ERROR_(OUT_OF_HEAP_P)
				}

				add_oper->op = SUBTRACT;

				(expr->has_retrevied_result + expr->any_write_offset)->the_op = add_oper;

				expr->any_write_offset += 1;

				/* Set State */
				last_value_was_subexpr = 0;

				continue;

			}
			/* If we have encountered a times operator, then we have reached the end
			   of the value */
			else if (*expression == '*') {


			/* If the value was - : "-" */
			if (inValueEncounteredZero == 0 && *current_number->malloced_decimal_number == '\0' && *current_number->malloced_number == '\0' && current_number->is_negative == 1 && current_number->is_decimal == 0) {

				/* Free malloced memory */
				free(have_encountered_low_pred);
				free_number(current_number);
				free_last_value_buff(last_value_buff);

				THROW_PARSE_ERROR()

			}

			/* If the value was 0 */
			if (inValueEncounteredNonZero == 0 && *current_number->malloced_number == '\0') {
				*current_number->malloced_number = '0';
				*(current_number->malloced_number + 1) = '\0';

				current_number->is_number_dig = 0;
				/* If the current number is "-0" which doesn't make sense
				 * so we just turn it to "0" */

				current_number->is_negative = 0;
			}

			/* If the value had a decimal point with no decimal numbers after it */
			if (current_number->is_decimal == 1 && (*(current_number->malloced_decimal_number) == '\0')) {
				current_number->is_decimal = 0;
			}


				/* Clear State */
				expectValue = 0;
				inValue = 0;
				inValueDecimal = 0;
				InValueNegative = 0;
				InValueNegative = 0;
				InValueEncounteredDigit = 0;
				InValueNegativeNoDigit = 0;

				/* Set new State */
				expectValue = 1;

				expectOpenBracket = 1;
				expectCloseBracket = 0;

				if (expectCloseBrackets > 0) {
					expectCloseBracket = 1;
				}
				else {
					expectCloseBracket = 0;
				}


				/* We have reached the operator size cap */
				if (expr->any_write_offset + 1 == expr->any_size) {
					struct any* malloced_str = realloc(expr->has_retrevied_result, (expr->any_size += ANY_ADD_MALLOC) * sizeof(struct any));

					if (malloced_str == NULL) {

						/* Free malloced memory */
						free(have_encountered_low_pred);
						free_number(current_number);
						free_last_value_buff(last_value_buff);

						THROW_PARSE_ERROR_(OUT_OF_HEAP_P)
					}

					expr->has_retrevied_result = malloced_str;

					memset(expr->has_retrevied_result + (expr->any_size - (sizeof(struct any) * ANY_ADD_MALLOC)), 0, ANY_ADD_MALLOC * sizeof(struct any));
				}

				/* Check for precedence conflicts */
				if (*(have_encountered_low_pred + expression_level) == '1') {

					*(have_encountered_low_pred + expression_level) = '0';


					/* If the last value was a subexpression, then start an exec at the beginning of the previous subexpression */
					if (higher_preced_expr != NULL && last_value_was_subexpr == 1) {

						/* Add the new exec and reuse "higher_preced_expr" */
						higher_preced_expr->is_start_new_exec += 1;
						higher_preced_expr = NULL;

					}
					else {

						(expr->has_retrevied_result + (expr->any_write_offset))->is_start_new_exec += 1;

					}

					/* Add one value from every exec */
					if (last_value_buff->buffer_offset != 0) {
						struct off_st* loopptr = (last_value_buff->buffer + last_value_buff->buffer_offset - 1);
						for (;; loopptr--) {
							loopptr->value += 1;

							if (loopptr == last_value_buff->buffer) {
								break;
							}
						}
					}
					is_prec_conf = 1;
				}
				else {

					*(have_encountered_low_pred + expression_level) = '0';
				}


				/* Create a copy of the current number object */
				struct number* add_number = copy_number(current_number, &is_curr_exis_dec_num);

				is_curr_exis_dec_num = 0;

				/* Empty the contents of the current number*/
				reuse_number(&current_number);

				(expr->has_retrevied_result + expr->any_write_offset)->is_number = 1;
				(expr->has_retrevied_result + expr->any_write_offset)->the_number =
					add_number;

				if (if_start_new_exec > 0) {
					(expr->has_retrevied_result + expr->any_write_offset)
						->is_start_new_exec = if_start_new_exec;


					/* If we have reached the end of our buffer size */
					if (last_value_buff->buffer_offset + 1 == last_value_buff->buffer_size) {
						struct off_st* malloced_str = realloc(last_value_buff->buffer, (last_value_buff->buffer_size + EXPRESS_OP_ADD_SIZE) * sizeof(struct off_st));

						if (malloced_str == NULL) {

							/* Free malloced memory */
							free(have_encountered_low_pred);
							free_number(current_number);
							free_last_value_buff(last_value_buff);

							THROW_PARSE_ERROR_(OUT_OF_HEAP_P)
						}

						last_value_buff->buffer = malloced_str;

						last_value_buff->buffer_size += EXPRESS_OP_ADD_SIZE;
					}

					/* Add current expression offset to the buffer */
					(last_value_buff->buffer + last_value_buff->buffer_offset)->offset = expr->any_write_offset;
					(last_value_buff->buffer + last_value_buff->buffer_offset)->value = if_start_new_exec;
					last_value_buff->buffer_offset += 1;


					if_start_new_exec = 0;
				}

				expr->any_write_offset += 1;

				/* If the amount of unused space is over MAX_NUMBER_SIZE */
				if ((add_number->number_size - write_pos) > MAX_NUMBER_SIZE) {
					char* malloced_str = realloc(add_number->malloced_number, write_pos + 1);

					if (malloced_str == NULL) {

						/* Free malloced memory */
						free(have_encountered_low_pred);
						free_number(current_number);
						free_last_value_buff(last_value_buff);

						THROW_PARSE_ERROR_(OUT_OF_HEAP_P)
					}

					add_number->malloced_number = malloced_str;

					add_number->number_size = write_pos + 1;
				}

				/* If the amount of unused space is over MAX_DEC_NUMBER_SIZE */
				if ((add_number->decimal_number_size - write_dec_pos) > MAX_DEC_NUMBER_SIZE) {
					char* malloced_str = realloc(add_number->malloced_decimal_number, write_dec_pos + 1);

					if (malloced_str == NULL) {

						/* Free malloced memory */
						free(have_encountered_low_pred);
						free_number(current_number);
						free_last_value_buff(last_value_buff);

						THROW_PARSE_ERROR_(OUT_OF_HEAP_P)
					}

					add_number->malloced_decimal_number = malloced_str;

					add_number->decimal_number_size = write_dec_pos + 1;
				}

				/* Reset number write offsets */
				write_pos = 0;
				write_dec_pos = 0;


				/* Add the operator to the expression */
				(expr->has_retrevied_result + expr->any_write_offset)->is_operator = 1;

				struct op* add_oper = malloc(sizeof(struct op));

				if (add_oper == NULL) {

					free(have_encountered_low_pred);
					free_number(current_number);
					free_last_value_buff(last_value_buff);

					THROW_PARSE_ERROR_(OUT_OF_HEAP_P)
				}

				add_oper->op = MULTILPLY;

				(expr->has_retrevied_result + expr->any_write_offset)->the_op = add_oper;

				expr->any_write_offset += 1;

				/* Set State */
				last_value_was_subexpr = 0;

				continue;
			}
			/* If we have encountered a divide operator, then we have reached the
			   end of the value */
			else if (*expression == '/') {


			/* If the value was - : "-" */
			if (inValueEncounteredZero == 0 && *current_number->malloced_decimal_number == '\0' && *current_number->malloced_number == '\0' && current_number->is_negative == 1 && current_number->is_decimal == 0) {

				/* Free malloced memory */
				free(have_encountered_low_pred);
				free_number(current_number);
				free_last_value_buff(last_value_buff);

				THROW_PARSE_ERROR()

			}

			/* If the value was 0 */
			if (inValueEncounteredNonZero == 0 && *current_number->malloced_number == '\0') {
				*current_number->malloced_number = '0';
				*(current_number->malloced_number + 1) = '\0';

				current_number->is_number_dig = 0;
				/* If the current number is "-0" which doesn't make sense
				 * so we just turn it to "0" */

				current_number->is_negative = 0;
			}

			/* If the value had a decimal point with no decimal numbers after it */
			if (current_number->is_decimal == 1 && (*(current_number->malloced_decimal_number) == '\0')) {
				current_number->is_decimal = 0;
			}


				/* Clear State */
				expectValue = 0;
				inValue = 0;
				inValueDecimal = 0;
				InValueNegative = 0;
				InValueNegative = 0;
				InValueEncounteredDigit = 0;
				InValueNegativeNoDigit = 0;

				/* Set new State */
				expectValue = 1;

				expectOpenBracket = 1;
				expectCloseBracket = 0;

				if (expectCloseBrackets > 0) {
					expectCloseBracket = 1;
				}
				else {
					expectCloseBracket = 0;
				}

				/* We have reached the operator size cap */
				if (expr->any_write_offset + 1 == expr->any_size) {
					struct any* malloced_str = realloc(expr->has_retrevied_result, (expr->any_size += ANY_ADD_MALLOC) * sizeof(struct any));

					if (malloced_str == NULL) {

						/* Free malloced memory */
						free(have_encountered_low_pred);
						free_number(current_number);
						free_last_value_buff(last_value_buff);

						THROW_PARSE_ERROR_(OUT_OF_HEAP_P)
					}

					expr->has_retrevied_result = malloced_str;

					memset(expr->has_retrevied_result + (expr->any_size - (sizeof(struct any) * ANY_ADD_MALLOC)), 0, ANY_ADD_MALLOC * sizeof(struct any));
				}


				/* Check for precedence conflicts */
				if (*(have_encountered_low_pred + expression_level) == '1') {

					*(have_encountered_low_pred + expression_level) = '0';


					/* If the last value was a subexpression, then start an exec at the beginning of the previous subexpression */
					if (higher_preced_expr != NULL && last_value_was_subexpr == 1) {

						/* Add the new exec and reuse "higher_preced_expr" */
						higher_preced_expr->is_start_new_exec += 1;
						higher_preced_expr = NULL;

					}
					else {

						(expr->has_retrevied_result + (expr->any_write_offset))->is_start_new_exec += 1;

					}

					/* Add one value from every exec */
					if (last_value_buff->buffer_offset != 0) {
						struct off_st* loopptr = (last_value_buff->buffer + last_value_buff->buffer_offset - 1);
						for (;; loopptr--) {
							loopptr->value += 1;

							if (loopptr == last_value_buff->buffer) {
								break;
							}
						}
					}
					is_prec_conf = 1;
				}
				else {

					*(have_encountered_low_pred + expression_level) = '0';
				}



				/* Create a copy of the current number object */
				struct number* add_number = copy_number(current_number, &is_curr_exis_dec_num);

				is_curr_exis_dec_num = 0;

				/* Empty the contents of the current number*/
				reuse_number(&current_number);

				(expr->has_retrevied_result + expr->any_write_offset)->is_number = 1;
				(expr->has_retrevied_result + expr->any_write_offset)->the_number =
					add_number;

				if (if_start_new_exec > 0) {
					(expr->has_retrevied_result + expr->any_write_offset)
						->is_start_new_exec = if_start_new_exec;


					/* If we have reached the end of our buffer size */
					if (last_value_buff->buffer_offset + 1 == last_value_buff->buffer_size) {
						struct off_st* malloced_str = realloc(last_value_buff->buffer, (last_value_buff->buffer_size + EXPRESS_OP_ADD_SIZE) * sizeof(struct off_st));

						if (malloced_str == NULL) {

							/* Free malloced memory */
							free(have_encountered_low_pred);
							free_number(current_number);
							free_last_value_buff(last_value_buff);

							THROW_PARSE_ERROR_(OUT_OF_HEAP_P)
						}

						last_value_buff->buffer = malloced_str;

						last_value_buff->buffer_size += EXPRESS_OP_ADD_SIZE;
					}

					/* Add current expression offset to the buffer */
					(last_value_buff->buffer + last_value_buff->buffer_offset)->offset = expr->any_write_offset;
					(last_value_buff->buffer + last_value_buff->buffer_offset)->value = if_start_new_exec;
					last_value_buff->buffer_offset += 1;


					if_start_new_exec = 0;
				}

				expr->any_write_offset += 1;

				/* If the amount of unused space is over MAX_NUMBER_SIZE */
				if ((add_number->number_size - write_pos) > MAX_NUMBER_SIZE) {
					char* malloced_str = realloc(add_number->malloced_number, write_pos + 1);

					if (malloced_str == NULL) {

						/* Free malloced memory */
						free(have_encountered_low_pred);
						free_number(current_number);
						free_last_value_buff(last_value_buff);

						THROW_PARSE_ERROR_(OUT_OF_HEAP_P)
					}

					add_number->malloced_number = malloced_str;

					add_number->number_size = write_pos + 1;
				}

				/* If the amount of unused space is over MAX_DEC_NUMBER_SIZE */
				if ((add_number->decimal_number_size - write_dec_pos) > MAX_DEC_NUMBER_SIZE) {
					char* malloced_str = realloc(add_number->malloced_decimal_number, write_dec_pos + 1);

					if (malloced_str == NULL) {

						/* Free malloced memory */
						free(have_encountered_low_pred);
						free_number(current_number);
						free_last_value_buff(last_value_buff);

						THROW_PARSE_ERROR_(OUT_OF_HEAP_P)
					}

					add_number->malloced_decimal_number = malloced_str;

					add_number->decimal_number_size = write_dec_pos + 1;
				}

				/* Reset number write offsets */
				write_pos = 0;
				write_dec_pos = 0;


				/* Add the operator to the expression */
				(expr->has_retrevied_result + expr->any_write_offset)->is_operator = 1;

				struct op* add_oper = malloc(sizeof(struct op));

				if (add_oper == NULL) {

					free(have_encountered_low_pred);
					free_number(current_number);
					free_last_value_buff(last_value_buff);

					THROW_PARSE_ERROR_(OUT_OF_HEAP_P)
				}

				add_oper->op = DIVIDE;

				(expr->has_retrevied_result + expr->any_write_offset)->the_op = add_oper;

				expr->any_write_offset += 1;

				/* Set State */
				last_value_was_subexpr = 0;

				continue;
			}

			/* If the value doesn't equal to any of the valid buckets above then it is invalid */
			else if (*expression != ' ') {

			/* Free malloced memory */
			free(have_encountered_low_pred);
			free_number(current_number);
			free_last_value_buff(last_value_buff);

			THROW_PARSE_ERROR()
			}
		}

		// We are expecting a value
		if (expectValue == 1) {

			expectOpenBracket = 1;
			expectCloseBracket = 0;
			inValueEncounteredZero = 0;
			inValueEncounteredNonZero = 0;


			/* Check if we are on a potential negative sign */
			if (*expression == '-') {

				/* Set State */
				last_value_was_subexpr = 0;

		       	expectOpenBracket = 0;
				expectCloseBracket = 1;

				/* Clear state */
				expectValue = 0;
				inValueDecimal = 0;

				/* Set New State */
				inValue = 1;
				InValueNegative = 1;
				InValueNegativeNoDigit = 1;

				current_number->is_negative = 1;

				/* Continue looping */
				continue;
			}
			/* Check if we are on a potential digit */
			else if (isdigit(*expression)) {

				/* Set State */
				last_value_was_subexpr = 0;

				expectOpenBracket = 0;
				expectCloseBracket = 1;

				/* Clear state */
				expectValue = 0;
				inValueDecimal = 0;
				InValueNegative = 0;

				/* Set new state */
				inValue = 1;
				InValueEncounteredDigit = 1;

				/* Add a character to the number */
				if (write_pos == current_number->number_size - 1) {

					// Incresing the numbers value
					current_number->number_size += VALUE_ADD_MALLOC;

					// Reallocating and Reinitalizing the new memory
					char* malloced_str = (char*)realloc(current_number->malloced_number, current_number->number_size); // The raw number we have retreived so far

					if (malloced_str == NULL) {

						/* Free malloced memory */
						free(have_encountered_low_pred);
						free_number(current_number);
						free_last_value_buff(last_value_buff);

						THROW_PARSE_ERROR_(OUT_OF_HEAP_P)
					}

					current_number->malloced_number = malloced_str;

					memset(current_number->malloced_number + (current_number->number_size - VALUE_ADD_MALLOC), 0, VALUE_ADD_MALLOC * sizeof(char));
				}


				if (inValueEncounteredNonZero == 1) {

					*(current_number->malloced_number + write_pos) = *expression;
					write_pos += 1;
					*(current_number->malloced_number + write_pos) = '\0';

				}
				else if (inValueEncounteredNonZero == 0 && *expression != '0') {

					*(current_number->malloced_number + write_pos) = *expression;
					write_pos += 1;
					*(current_number->malloced_number + write_pos) = '\0';

				}

				/* If we have encountered a zero in our value */
				if (*expression == '0') {
					inValueEncounteredZero = 1;
				}

				/* Continue looping */
				continue;
			}
			/* Check if we are on a decimal point */
			else if (*expression == '.') {

				/* Set State */
				last_value_was_subexpr = 0;

				expectOpenBracket = 0;
				expectCloseBracket = 1;

				/* Clear state */
				expectValue = 0;
				InValueNegative = 0;

				/* Set new state*/
				inValue = 1;
				inValueDecimal = 1;

				/* If there wasn't any digit previously */
				current_number->is_number_dig = 1;
				current_number->is_decimal = 1;

			}

			/* Check if we are on nothing valid */
			else if (*expression != ' ') {

				/* Free malloced memory */
				free(have_encountered_low_pred);
				free_number(current_number);
				free_last_value_buff(last_value_buff);

				THROW_PARSE_ERROR()

			}
		}
	}


	/* If we were in a value and have encountered the end of the expression string */
	if (inValue) {

		/* If the value was - : "-" */
		if (inValueEncounteredZero == 0 && *current_number->malloced_decimal_number == '\0' && *current_number->malloced_number == '\0' && current_number->is_negative == 1 && current_number->is_decimal == 0) {

			/* Free malloced memory */
			free(have_encountered_low_pred);
			free_number(current_number);
			free_last_value_buff(last_value_buff);

			THROW_PARSE_ERROR()

		}

		/* If the value was 0 */
		if (inValueEncounteredNonZero == 0 && *current_number->malloced_number == '\0') {
			*current_number->malloced_number = '0';
			*(current_number->malloced_number + 1) = '\0';

			current_number->is_number_dig = 0;
			/* If the current number is "-0" which doesn't make sense
			 * so we just turn it to "0" */

			current_number->is_negative = 0;
		}

		/* If the value had a decimal point with no decimal numbers after it */
		if (current_number->is_decimal == 1 && (*(current_number->malloced_decimal_number) == '\0')) {
			current_number->is_decimal = 0;
		}

		/* Set State */
		last_value_was_subexpr = 0;

		/* Clear State */
		expectValue = 0;
		inValue = 0;
		inValueDecimal = 0;
		InValueNegative = 0;
		InValueNegative = 0;
		InValueEncounteredDigit = 0;
		InValueNegativeNoDigit = 0;

		/* Set new State */
		expectOperator = 1;

		/* We have reached the operator size cap */
		if (expr->any_write_offset + 1 == expr->any_size) {
			struct any* malloced_str = realloc(expr->has_retrevied_result, (expr->any_size += ANY_ADD_MALLOC) * sizeof(struct any));

			if (malloced_str == NULL) {

				/* Free malloced memory */
				free(have_encountered_low_pred);
				free_number(current_number);
				free_last_value_buff(last_value_buff);

				THROW_PARSE_ERROR_(OUT_OF_HEAP_P)
			}

			expr->has_retrevied_result = malloced_str;

			memset(expr->has_retrevied_result + ((expr->any_size - ANY_ADD_MALLOC) * sizeof(struct any)), 0, ANY_ADD_MALLOC * sizeof(struct any));
		}

		/* Create a copy of the current number object */
		struct number* add_number = copy_number(current_number, &is_curr_exis_dec_num);

		is_curr_exis_dec_num = 0;

		/* Empty the contents of the current number*/
		reuse_number(&current_number);

		(expr->has_retrevied_result + expr->any_write_offset)->is_number = 1;
		(expr->has_retrevied_result + expr->any_write_offset)->the_number =
			add_number;

		if (if_start_new_exec > 0) {
			(expr->has_retrevied_result + expr->any_write_offset)
				->is_start_new_exec = if_start_new_exec;


			/* If we have reached the end of our buffer size */
			if (last_value_buff->buffer_offset + 1 == last_value_buff->buffer_size) {
				struct off_st* malloced_str = realloc(last_value_buff->buffer, (last_value_buff->buffer_size + EXPRESS_OP_ADD_SIZE) * sizeof(struct off_st));

				if (malloced_str == NULL) {

					/* Free malloced memory */
					free(have_encountered_low_pred);
					free_number(current_number);
					free_last_value_buff(last_value_buff);

					THROW_PARSE_ERROR_(OUT_OF_HEAP_P)
				}

				last_value_buff->buffer = malloced_str;

				last_value_buff->buffer_size += EXPRESS_OP_ADD_SIZE;
			}

			/* Add current expression offset to the buffer */
			(last_value_buff->buffer + last_value_buff->buffer_offset)->offset = expr->any_write_offset;
			(last_value_buff->buffer + last_value_buff->buffer_offset)->value = if_start_new_exec;
			last_value_buff->buffer_offset += 1;


			if_start_new_exec = 0;
		}

		expr->any_write_offset += 1;

		/* If the amount of unused space is over MAX_NUMBER_SIZE */
		if ((add_number->number_size - write_pos) > MAX_NUMBER_SIZE) {
			char* malloced_str = realloc(add_number->malloced_number, write_pos + 1);


			if (malloced_str == NULL) {

				/* Free malloced memory */
				free(have_encountered_low_pred);
				free_number(current_number);
				free_last_value_buff(last_value_buff);

				THROW_PARSE_ERROR_(OUT_OF_HEAP_P)
			}

			add_number->malloced_number = malloced_str;

			add_number->number_size = write_pos + 1;
		}

		/* If the amount of unused space is over MAX_DEC_NUMBER_SIZE */
		if ((add_number->decimal_number_size - write_dec_pos) > MAX_DEC_NUMBER_SIZE) {
			char* malloced_str = realloc(add_number->malloced_decimal_number, write_dec_pos + 1);

			if (malloced_str == NULL) {

				/* Free malloced memory */
				free(have_encountered_low_pred);
				free_number(current_number);
				free_last_value_buff(last_value_buff);

				THROW_PARSE_ERROR_(OUT_OF_HEAP_P)
			}

			add_number->malloced_decimal_number = malloced_str;

			add_number->decimal_number_size = write_dec_pos + 1;
		}

		/* Reset number write offsets */
		write_pos = 0;
		write_dec_pos = 0;

	}

	/* If we have reached the end of the string, and need to close our previous exec that was started by higher precedence operators */
	if (*(have_encountered_low_pred + expression_level) == '0') {

		(expr->has_retrevied_result + (expr->any_write_offset - 1))
			->is_end_new_exec += 1;

	}

	/* If this were a valid expression then it would have ended with a value meaning we would be expecting an operator */
	if (expectOperator != 1) {
		
		/* Free malloced memory */
		free(have_encountered_low_pred);
		free_number(current_number);
		free_last_value_buff(last_value_buff);

		THROW_PARSE_ERROR()
	}

	/* Free malloced memory */
	free(have_encountered_low_pred);
	free_number(current_number);
	free_last_value_buff(last_value_buff);

	if (expectCloseBrackets != 0) {
		THROW_PARSE_ERROR()
	}

	result = expr;
	return 0;
}
