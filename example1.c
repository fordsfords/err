#include <stdio.h>
#include "err.h"


int reciprocal(double *result_rtn, double input_value, err_t *err)
{
  /* Sanity checks: assert that things are true that must be true. */
  ERR_ASSRT(input_value != 0, ERR_CODE_PARAM, err);  /* Division by zero not allowed. */

  *result_rtn = 1.0 / input_value;

  return ERR_OK;
}  /* reciprocal */


int main(int argc, char **argv)
{
  double result;
  err_t my_err;

  reciprocal(&result, 4, NULL);  /* abort if error. */
  printf("1/4=%f\n", result);  fflush(stdout);

  if (reciprocal(&result, 0, &my_err) != ERR_OK) {
  	printf("reciprocal error at [%s:%d %s()]: code=%d, msg=%s\n",
	    my_err.file, my_err.line, my_err.func, (int)my_err.code, my_err.msg);
    fflush(stdout);
  }

  return 0;
}  /* main */
