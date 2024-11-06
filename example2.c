/* example2.c */

#include <stdio.h>
#include "err.h"
#include "err_codes.h"


ERR_F reciprocal(double *result_rtn, double input_value)
{
  /* Sanity checks: assert that things are true that must be true. */
  ERR_ASSRT(input_value != 0, ERR_CODE_PARAM);  /* Division by zero not allowed. */

  /* For whatever reason, input value should never be 1. */
  if (input_value == 1) {
    ERR_THROW(ERR_CODE_INTERNAL, "input_value == 1");
  }

  *result_rtn = 1.0 / input_value;

  return ERR_OK;
}  /* reciprocal */


ERR_F try_one_reciprocol(double input)
{
  double result;
  err_t *err;

  err = reciprocal(&result, input);
  if (err == ERR_OK) {
    printf("Reciprocol of %f is %f\n", input, result);
    return ERR_OK;
  }

  /* An error was returned, handle it. */

  if (err->code == ERR_CODE_PARAM) {
    printf("division by zero not allowed. Try again.\n");  fflush(stdout);
    err_dispose(err);  /* Since we are handling, delete the err object. */
    return ERR_OK;
  }

  /* Unrecognized error. */
  ERR_RETHROW(err, err->code);
}  /* try_one_reciprocol */


ERR_F math_example(int argc, char **argv)
{
  double input;

  printf("Input (floating point number)? ");  fflush(stdout);
  while (scanf("%lf", &input) == 1) {
    ERR(try_one_reciprocol(input));

    printf("Input? ");  fflush(stdout);
  }
  printf("No valid input found, exiting.\n");  fflush(stdout);

  return ERR_OK;
}  /* math_example */


int main(int argc, char **argv)
{
  err_t *err;

  /* If error returns to outer-most main, abort. */
  err = math_example(argc, argv);
  if (err) {
    FILE *log_fp = fopen("err.log", "w");
    if (log_fp == NULL) {
      fprintf(stderr, "ERROR: could not open 'err.log'\n");
      log_fp = stderr;
    }
    else {
      fprintf(stderr, "UNHANDLED ERROR. See 'err.log' for details.\n");
    }

    ERR_ABRT(err, log_fp);  /* Print stack trace and dupm core. */
  }

  return 0;
}  /* main */
