/* example1.c - Demonstration of "err" usage. */
/*
# This code and its documentation is Copyright 2024-2024 Steven Ford, http://geeky-boy.com
# and licensed "public domain" style under Creative Commons "CC0": http://creativecommons.org/publicdomain/zero/1.0/
# To the extent possible under law, the contributors to this project have
# waived all copyright and related or neighboring rights to this work.
# In other words, you can use this code for any purpose without any
# restrictions.  This work is published from: United States.  The project home
# is https://github.com/fordsfords/err
*/

#include <stdio.h>
#include "err.h"


char *reciprocal(double *result_rtn, double input_value, err_t *err)
{
  /* Sanity checks: assert that things are true that must be true. */
  ERR_ASSRT(input_value != 0, ERR_CODE_PARAM, err);  /* Division by zero not allowed. */

  *result_rtn = 1.0 / input_value;

  ERR_RTN_OK(err);
}  /* reciprocal */


int main(int argc, char **argv)
{
  double result;
  err_t my_err;

  reciprocal(&result, 4, NULL);  /* abort if error. */
  printf("1/4=%f\n", result);  fflush(stdout);

  if (reciprocal(&result, 0, &my_err)) {
  	printf("reciprocal error at [%s:%d %s()]: code=%s, msg=%s\n",
	    my_err.file, my_err.line, my_err.func, my_err.code, my_err.msg);
    fflush(stdout);
  }

  return 0;
}  /* main */
