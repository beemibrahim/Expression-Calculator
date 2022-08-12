#include "expression_parser.h"

int main(int argc, char *argv[]) {

  printf("Advanced Calculator v1.0\nPlease type an aritmetic expression to get "
         "started\n\n");

  /* Loop forever */
  while (1) {

    /* This variable will contain the expression */
    char *expression;
    int expression_size = 0;
    expression = inputString(stdin, 10, 1, &expression_size);

    /* Parse the given expression into an expression object */
    struct expression* parsed_expr = NULL;
    int success = ParseExpression(expression, expression_size,parsed_expr);
  }
}
