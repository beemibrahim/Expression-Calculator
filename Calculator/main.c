#include "calculator_methods.h"

int main(int argc, char *argv[]) {

  printf("Advanced Calculator v1.0\nPlease type an aritmetic expression to get "
         "started\n\n");

  /* Loop forever */
  while (1) {

    /* This variable will contain the expression */
    char *expression;
    int expression_size = 0;
    int success_get = inputString(stdin, 10, 1, &expression_size, &expression);

    if (success_get == 1) {
        printf("Out of memory");
        return -1;
    }

    /* If the user wants to continue the writing the expression below */
    if (*(expression + expression_size - 1) == '\\') {

        for (;;) {
            char* sub_expression;
            int sub_expression_size = 0;
            int success_get = inputString(stdin, 10, 1, &sub_expression_size, &sub_expression);

            /* If we are out of heap memory */
            if (success_get == 1) {
                printf("Out of memory");
                return -1;
            }

            if (*sub_expression == '\0') {
                free(sub_expression);
                break;
            }

            /* Realloc the expression based on the size of the passed in data */
            expression = realloc(expression, expression_size + sub_expression_size);
            expression_size += (sub_expression_size);

            /* Copying the memory from the subexpression to the original expression */
            /* We are putting "- 1" so that is replaces the previous slash */
            memcpy(expression + (expression_size - sub_expression_size - 1), sub_expression, sub_expression_size);

            /* Put the null byte */
            *(expression + expression_size - 1) = '\0';

            /* Free the sub expressions memory */
            free(sub_expression);

            /* Check if the user doesn't want to continue writing the expression below */
            if (*(expression + expression_size - 1) != '\\') {
                break;
            }
        }
        
    }

    if (*expression == '\0') {
        free(expression);
        continue;
    }

    /* Parse the given expression into an expression object */
    struct expression* parsed_expr = NULL;
    char* parse_error_message = NULL;

    int success_parse = ParseExpression(expression, expression_size,parsed_expr,&parse_error_message);

    /* Check if the parse operator succedded or not */
    if (success_parse != 0) {
        free(expression);
        printf(parse_error_message);
        continue;
    }

    /* Free the allocated string */
    free(expression);

  }
}
