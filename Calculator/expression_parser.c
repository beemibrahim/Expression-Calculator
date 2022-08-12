#include "expression_parser.h"

int ParseExpression(char* expression, int expression_size, struct expression* result)
{
    /* Parse State */

    /* State concerning if we have encountered a low precedence character in the
     * current expression */
    char* have_encountered_low_pred = (char*)malloc(expression_size * sizeof(char));
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
    int inValue = 0;         /* Are we in a value */
    int inValueDecimal = 0;  /* Have we encountered a decimal point in the
                                current value were in */
    int InValueNegative = 0; /* Have we encountered a negative sign at the
                                beginning of our value */
    int InValueEncounteredDigit = 0; /* Have we encountered a digit in our current value */
    int InValueNegativeNoDigit = 0; /* We have encountered a negative sign but
                                       we haven't encountered a digit */

                                       /* Number properties */

                                       /* Creating the number object */
    struct number* current_number = (struct number*)calloc(1, sizeof(struct number));

    /* Allocating dynamic memory for the number */
    current_number->malloced_number = (char*)malloc(VALUE_START_MALLOC); /* The raw number we have retreived so far */

    /* Allocating dynamic memory for the decimal point number */
    current_number->malloced_decimal_number = (char*)malloc(DECIMAL_VALUE_START_MALLOC); /* The raw decimal number we have retreived so far */

    /* Initalizing the expression object */
    struct expression* expr = calloc(1, sizeof(struct expression));


    /* Initalizing the operator's in the expression object */
    expr->has_retrevied_result = malloc(sizeof(struct any) * ANY_START_MALLOC);
    memset(expr->has_retrevied_result, 0, (sizeof(struct any) * ANY_START_MALLOC));

    expr->any_size = ANY_START_MALLOC;

    /* Set the sizes of the numbers */
    current_number->number_size = VALUE_START_MALLOC;
    current_number->decimal_number_size = DECIMAL_VALUE_START_MALLOC;

    /* States containing info about if the next value is the first value in a
     * preceding open or close bracket */
    int if_start_new_exec = 0;
    int if_end_curr_exec = 0;

    /* The write offset for the current number */
    int write_pos = 0;

    /* The write offset for the current decimal number */
    int write_dec_pos = 0;

    /* This variable contains whether or not there were precedence conflicts earlier */
    int is_prec_conf = 0;

    /* If we have wrote anything to an existing decimal place */
    int is_curr_exis_dec_num = 0;

    char last_ch = '\0';

    /* Loop over the expression character by character */
    for (; *expression; last_ch = *expression, expression += 1) {

        /* Have we found an open bracket */
        if (expectOpenBracket == 1 && *expression == '(') {

            /* Set the new state */
            expectOpenBracket = 1;
            expectCloseBracket = 1;
            ++if_start_new_exec;
            expectValue = 1;
            expectOperator = 0;

            /* Set the new count */
            expectCloseBrackets += 1;

            /* Start a new expression context */
            expression_level += 1;
            *(have_encountered_low_pred + expression_level) = '1';

            continue;
        }

        /* Have we found a close bracket */
        if (expectCloseBracket == 1 && *expression == ')') {

            /* Set the new state */
            expectOpenBracket = 0;
            ++if_end_curr_exec;

            /* Check if we have encapsulted a value inside two meaningless brackets : (56) */
            if (if_start_new_exec > 0) {
                if_end_curr_exec--;
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
                    expr->has_retrevied_result = realloc(expr->has_retrevied_result, expr->any_size += (sizeof(struct any) * ANY_ADD_MALLOC));

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

                    if_start_new_exec = 0;
                }
                else if (if_end_curr_exec > 0) {
                    (expr->has_retrevied_result + expr->any_write_offset)
                        ->is_end_new_exec = if_end_curr_exec;

                    if_end_curr_exec = 0;
                }

                expr->any_write_offset += 1;

                /* If the amount of unused space is over MAX_NUMBER_SIZE */
                if ((add_number->number_size - write_pos) > MAX_NUMBER_SIZE) {
                    add_number->malloced_number = realloc(add_number->malloced_number, write_pos + 1);
                    add_number->number_size = write_pos + 1;
                }

                /* If the amount of unused space is over MAX_DEC_NUMBER_SIZE */
                if ((add_number->decimal_number_size - write_dec_pos) > MAX_DEC_NUMBER_SIZE) {
                    add_number->malloced_decimal_number = realloc(add_number->malloced_decimal_number, write_dec_pos + 1);
                    add_number->decimal_number_size = write_dec_pos + 1;
                }

                /* Reset number write offsets */
                write_pos = 0;
                write_dec_pos = 0;

            }

            /* If we aren't in a value then we shall assume that the last operator/value if going to be an exec */
            else {
                (expr->has_retrevied_result + (expr->any_write_offset - 1))
                    ->is_end_new_exec += if_end_curr_exec;
            }

            /* If we have reached this close bracket and need to close our previous exec that was started by higher precedence operators */
            if (*(have_encountered_low_pred + expression_level) == '0') {

                (expr->has_retrevied_result + (expr->any_write_offset - 1))
                    ->is_end_new_exec += 1;

            }

            expectValue = 0;
            expectOperator = 1;

            /* Set the new count */
            expectCloseBrackets -= 1;

            /* Start a new expression context */
            *(have_encountered_low_pred + expression_level) = '1';
            expression_level -= 1;

            continue;
        }

        /* If we expect an operator */
        if (expectOperator == 1) {

            /* If we expect an operator and the operator is a plus */
            if (*expression == '+') {


                /* Check if we have looped through a series of high precedence operators */
                if (is_prec_conf == 1) {

                    (expr->has_retrevied_result + (expr->any_write_offset - 1))->is_end_new_exec += 1;

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

                add_oper->op = ADD;

                (expr->has_retrevied_result + expr->any_write_offset)->the_op = add_oper;

                expr->any_write_offset += 1;

                continue;

            }

            /* If we expect an operator and the operator is a minus */
            if (*expression == '-') {

                /* Check if we have looped through a series of high precedence operators */
                if (is_prec_conf == 1) {

                    (expr->has_retrevied_result + (expr->any_write_offset - 1))->is_end_new_exec += 1;

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

                add_oper->op = SUBTRACT;

                (expr->has_retrevied_result + expr->any_write_offset)->the_op = add_oper;

                expr->any_write_offset += 1;

                continue;
            }

            /* If we expect an operator and the operator is a times */
            if (*expression == '*') {

                /* Check for precedence conflicts */
                if (*(have_encountered_low_pred + expression_level) == '1') {

                    *(have_encountered_low_pred + expression_level) = '0';

                    (expr->has_retrevied_result + (expr->any_write_offset - 1))->is_start_new_exec += 1;

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

                add_oper->op = MULTILPLY;

                (expr->has_retrevied_result + expr->any_write_offset)->the_op = add_oper;

                expr->any_write_offset += 1;

                continue;
            }

            /* If we expect an operator and the operator is a divide */
            if (*expression == '/') {

                /* Check for precedence conflicts */
                if (*(have_encountered_low_pred + expression_level) == '1') {

                    *(have_encountered_low_pred + expression_level) = '0';

                    (expr->has_retrevied_result + (expr->any_write_offset - 1))->is_start_new_exec += 1;

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

                add_oper->op = DIVIDE;

                (expr->has_retrevied_result + expr->any_write_offset)->the_op = add_oper;

                expr->any_write_offset += 1;

                continue;
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
                        current_number->malloced_decimal_number = (char*)realloc(current_number->malloced_decimal_number, sizeof(char)); // The raw number we have retreived so far
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
                        current_number->malloced_number = (char*)realloc(current_number->malloced_number, sizeof(char)); // The raw number we have retreived so far
                        memset(current_number->malloced_number + (current_number->number_size - VALUE_ADD_MALLOC), 0, VALUE_ADD_MALLOC * sizeof(char));
                    }

                    *(current_number->malloced_number + write_pos) = *expression;
                    write_pos += 1;
                    *(current_number->malloced_number + write_pos) = '\0';

                }

                continue;
            }
            /* We have encountered a decimal point */
            else if (*expression == '.' && inValueDecimal == 0) {
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

                if (expectCloseBrackets > 0) {
                    expectCloseBracket = 1;
                }
                else {
                    expectCloseBracket = 0;
                }

                /* We have reached the operator size cap */
                if (expr->any_write_offset + 1 == expr->any_size) {
                    expr->has_retrevied_result = realloc(expr->has_retrevied_result, expr->any_size += (sizeof(struct any) * ANY_ADD_MALLOC));

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

                    if_start_new_exec = 0;
                }
                else if (if_end_curr_exec > 0) {
                    (expr->has_retrevied_result + expr->any_write_offset)
                        ->is_end_new_exec = if_end_curr_exec;

                    if_end_curr_exec = 0;
                }

                expr->any_write_offset += 1;

                /* If the amount of unused space is over MAX_NUMBER_SIZE */
                if ((add_number->number_size - write_pos) > MAX_NUMBER_SIZE) {
                    add_number->malloced_number = realloc(add_number->malloced_number, write_pos + 1);
                    add_number->number_size = write_pos + 1;
                }

                /* If the amount of unused space is over MAX_DEC_NUMBER_SIZE */
                if ((add_number->decimal_number_size - write_dec_pos) > MAX_DEC_NUMBER_SIZE) {
                    add_number->malloced_decimal_number = realloc(add_number->malloced_decimal_number, write_dec_pos + 1);
                    add_number->decimal_number_size = write_dec_pos + 1;
                }

                /* Reset number write offsets */
                write_pos = 0;
                write_dec_pos = 0;

                continue;
            }
            /* If we have encountered an add operator, then we have reached the end
               of the value */
            else if (*expression == '+') {

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
                    expr->has_retrevied_result = realloc(expr->has_retrevied_result, expr->any_size += (sizeof(struct any) * ANY_ADD_MALLOC));

                    memset(expr->has_retrevied_result + (expr->any_size - (sizeof(struct any) * ANY_ADD_MALLOC)), 0, ANY_ADD_MALLOC * sizeof(struct any));
                }

                /* Check if we have looped through a series of high precedence operators */
                if (is_prec_conf == 1) {

                    (expr->has_retrevied_result + (expr->any_write_offset))->is_end_new_exec += 1;

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

                    if_start_new_exec = 0;
                }
                else if (if_end_curr_exec > 0) {
                    (expr->has_retrevied_result + expr->any_write_offset)
                        ->is_end_new_exec = if_end_curr_exec;

                    if_end_curr_exec = 0;
                }

                expr->any_write_offset += 1;

                /* If the amount of unused space is over MAX_NUMBER_SIZE */
                if ((add_number->number_size - write_pos) > MAX_NUMBER_SIZE) {
                    add_number->malloced_number = realloc(add_number->malloced_number, write_pos + 1);
                    add_number->number_size = write_pos + 1;
                }

                /* If the amount of unused space is over MAX_DEC_NUMBER_SIZE */
                if ((add_number->decimal_number_size - write_dec_pos) > MAX_DEC_NUMBER_SIZE) {
                    add_number->malloced_decimal_number = realloc(add_number->malloced_decimal_number, write_dec_pos + 1);
                    add_number->decimal_number_size = write_dec_pos + 1;
                }

                /* Reset number write offsets */
                write_pos = 0;
                write_dec_pos = 0;


                /* Add the operator to the expression */
                (expr->has_retrevied_result + expr->any_write_offset)->is_operator = 1;

                struct op* add_oper = malloc(sizeof(struct op));

                add_oper->op = ADD;

                (expr->has_retrevied_result + expr->any_write_offset)->the_op = add_oper;

                expr->any_write_offset += 1;

                continue;
            }
            /* If we have encountered a subtract operator, then we have reached the
               end of the value */
            else if (*expression == '-') {

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
                    expr->has_retrevied_result = realloc(expr->has_retrevied_result, expr->any_size += (sizeof(struct any) * ANY_ADD_MALLOC));

                    memset(expr->has_retrevied_result + (expr->any_size - (sizeof(struct any) * ANY_ADD_MALLOC)), 0, ANY_ADD_MALLOC * sizeof(struct any));
                }

                /* Check if we have looped through a series of high precedence operators */
                if (is_prec_conf == 1) {

                    (expr->has_retrevied_result + (expr->any_write_offset))->is_end_new_exec += 1;

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

                    if_start_new_exec = 0;
                }
                else if (if_end_curr_exec > 0) {
                    (expr->has_retrevied_result + expr->any_write_offset)
                        ->is_end_new_exec = if_end_curr_exec;

                    if_end_curr_exec = 0;
                }

                expr->any_write_offset += 1;


                /* If the amount of unused space is over MAX_NUMBER_SIZE */
                if ((add_number->number_size - write_pos) > MAX_NUMBER_SIZE) {
                    add_number->malloced_number = realloc(add_number->malloced_number, write_pos + 1);
                    add_number->number_size = write_pos + 1;
                }

                /* If the amount of unused space is over MAX_DEC_NUMBER_SIZE */
                if ((add_number->decimal_number_size - write_dec_pos) > MAX_DEC_NUMBER_SIZE) {
                    add_number->malloced_decimal_number = realloc(add_number->malloced_decimal_number, write_dec_pos + 1);
                    add_number->decimal_number_size = write_dec_pos + 1;
                }

                /* Reset number write offsets */
                write_pos = 0;
                write_dec_pos = 0;


                /* Add the operator to the expression */
                (expr->has_retrevied_result + expr->any_write_offset)->is_operator = 1;

                struct op* add_oper = malloc(sizeof(struct op));

                add_oper->op = SUBTRACT;

                (expr->has_retrevied_result + expr->any_write_offset)->the_op = add_oper;

                expr->any_write_offset += 1;

                continue;

            }
            /* If we have encountered a times operator, then we have reached the end
               of the value */
            else if (*expression == '*') {

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

                if (expectCloseBrackets > 0) {
                    expectCloseBracket = 1;
                }
                else {
                    expectCloseBracket = 0;
                }


                /* We have reached the operator size cap */
                if (expr->any_write_offset + 1 == expr->any_size) {
                    expr->has_retrevied_result = realloc(expr->has_retrevied_result, expr->any_size += (sizeof(struct any) * ANY_ADD_MALLOC));

                    memset(expr->has_retrevied_result + (expr->any_size - (sizeof(struct any) * ANY_ADD_MALLOC)), 0, ANY_ADD_MALLOC * sizeof(struct any));
                }

                /* Check for precedence conflicts */
                if (*(have_encountered_low_pred + expression_level) == '1') {

                    *(have_encountered_low_pred + expression_level) = '0';

                    (expr->has_retrevied_result + (expr->any_write_offset))->is_start_new_exec += 1;

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

                    if_start_new_exec = 0;
                }
                else if (if_end_curr_exec > 0) {
                    (expr->has_retrevied_result + expr->any_write_offset)
                        ->is_end_new_exec = if_end_curr_exec;

                    if_end_curr_exec = 0;
                }

                expr->any_write_offset += 1;

                /* If the amount of unused space is over MAX_NUMBER_SIZE */
                if ((add_number->number_size - write_pos) > MAX_NUMBER_SIZE) {
                    add_number->malloced_number = realloc(add_number->malloced_number, write_pos + 1);
                    add_number->number_size = write_pos + 1;
                }

                /* If the amount of unused space is over MAX_DEC_NUMBER_SIZE */
                if ((add_number->decimal_number_size - write_dec_pos) > MAX_DEC_NUMBER_SIZE) {
                    add_number->malloced_decimal_number = realloc(add_number->malloced_decimal_number, write_dec_pos + 1);
                    add_number->decimal_number_size = write_dec_pos + 1;
                }

                /* Reset number write offsets */
                write_pos = 0;
                write_dec_pos = 0;


                /* Add the operator to the expression */
                (expr->has_retrevied_result + expr->any_write_offset)->is_operator = 1;

                struct op* add_oper = malloc(sizeof(struct op));

                add_oper->op = MULTILPLY;

                (expr->has_retrevied_result + expr->any_write_offset)->the_op = add_oper;

                expr->any_write_offset += 1;

                continue;
            }
            /* If we have encountered a divide operator, then we have reached the
               end of the value */
            else if (*expression == '/') {

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

                if (expectCloseBrackets > 0) {
                    expectCloseBracket = 1;
                }
                else {
                    expectCloseBracket = 0;
                }
            }


            /* We have reached the operator size cap */
            if (expr->any_write_offset + 1 == expr->any_size) {
                expr->has_retrevied_result = realloc(expr->has_retrevied_result, expr->any_size += (sizeof(struct any) * ANY_ADD_MALLOC));

                memset(expr->has_retrevied_result + (expr->any_size - (sizeof(struct any) * ANY_ADD_MALLOC)), 0, ANY_ADD_MALLOC * sizeof(struct any));
            }


            /* Check for precedence conflicts */
            if (*(have_encountered_low_pred + expression_level) == '1') {

                *(have_encountered_low_pred + expression_level) = '0';

                (expr->has_retrevied_result + (expr->any_write_offset))->is_start_new_exec += 1;

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

                if_start_new_exec = 0;
            }
            else if (if_end_curr_exec > 0) {
                (expr->has_retrevied_result + expr->any_write_offset)
                    ->is_end_new_exec = if_end_curr_exec;

                if_end_curr_exec = 0;
            }

            expr->any_write_offset += 1;

            /* If the amount of unused space is over MAX_NUMBER_SIZE */
            if ((add_number->number_size - write_pos) > MAX_NUMBER_SIZE) {
                add_number->malloced_number = realloc(add_number->malloced_number, write_pos + 1);
                add_number->number_size = write_pos + 1;
            }

            /* If the amount of unused space is over MAX_DEC_NUMBER_SIZE */
            if ((add_number->decimal_number_size - write_dec_pos) > MAX_DEC_NUMBER_SIZE) {
                add_number->malloced_decimal_number = realloc(add_number->malloced_decimal_number, write_dec_pos + 1);
                add_number->decimal_number_size = write_dec_pos + 1;
            }

            /* Reset number write offsets */
            write_pos = 0;
            write_dec_pos = 0;


            /* Add the operator to the expression */
            (expr->has_retrevied_result + expr->any_write_offset)->is_operator = 1;

            struct op* add_oper = malloc(sizeof(struct op));

            add_oper->op = DIVIDE;

            (expr->has_retrevied_result + expr->any_write_offset)->the_op = add_oper;

            expr->any_write_offset += 1;

            continue;
        }

        // We are expecting a value
        if (expectValue == 1) {

            /* Check if we are on a potential negative sign */
            if (*expression == '-') {

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
                    current_number->malloced_number = (char*)realloc(current_number->malloced_number, sizeof(char)); // The raw number we have retreived so far
                    memset(current_number->malloced_number + (current_number->number_size - VALUE_ADD_MALLOC), 0, VALUE_ADD_MALLOC * sizeof(char));
                }

                *(current_number->malloced_number + write_pos) = *expression;
                write_pos += 1;
                *(current_number->malloced_number + write_pos) = '\0';

                /* Continue looping */
                continue;
            }
            /* Check if we are on a decimal point */
            else if (*expression == '.') {

                /* Clear state */
                expectValue = 0;
                InValueNegative = 0;

                /* Set new state*/
                inValue = 1;
                inValueDecimal = 1;

                /* If there wasn't any digit previously */
                current_number->is_number_dig = 1;

            }
        }
    }


    /* If we were in a value and have encountered the end of the expression string */
    if (inValue) {

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
            expr->has_retrevied_result = realloc(expr->has_retrevied_result, expr->any_size += (sizeof(struct any) * ANY_ADD_MALLOC));

            memset(expr->has_retrevied_result + (expr->any_size - ( sizeof(struct any) * ANY_ADD_MALLOC)), 0, ANY_ADD_MALLOC * sizeof(struct any));
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

            if_start_new_exec = 0;
        }
        else if (if_end_curr_exec > 0) {
            (expr->has_retrevied_result + expr->any_write_offset)
                ->is_end_new_exec = if_end_curr_exec;

            if_end_curr_exec = 0;
        }

        expr->any_write_offset += 1;

        /* If the amount of unused space is over MAX_NUMBER_SIZE */
        if ((add_number->number_size - write_pos) > MAX_NUMBER_SIZE) {
            add_number->malloced_number = realloc(add_number->malloced_number, write_pos + 1);
            add_number->number_size = write_pos + 1;
        }

        /* If the amount of unused space is over MAX_DEC_NUMBER_SIZE */
        if ((add_number->decimal_number_size - write_dec_pos) > MAX_DEC_NUMBER_SIZE) {
            add_number->malloced_decimal_number = realloc(add_number->malloced_decimal_number, write_dec_pos + 1);
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
        return 1;
    }

    /* Free malloced memory */
    free(have_encountered_low_pred);
    free_number(current_number);

    result = expr;
    return 0;
}
