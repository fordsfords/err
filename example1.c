/* example1.c */

#include <stdio.h>
#include "err.h"

ERR_F reciprocal(double *result_rtn, double input_value)
{
  /* Sanity checks: assert that things are true that must be true. */
  ERR_ASSRT(input_value != 0, ERR_ERR_PARAM);  /* Division by zero not allowed. */

  *result_rtn = 1.0 / input_value;

  return ERR_OK;
}  /* reciprocal */


ERR_F math_example()
{
  double result;

  ERR(reciprocal(&result, 4));
  printf("1/4=%f\n", result);  fflush(stdout);

  ERR(reciprocal(&result, 0));  /* Division by 0! */
  printf("Should not get here.\n");  fflush(stdout);

  return ERR_OK;
}  /* math_example */


int main(int argc, char **argv)
{
  (void)argc;  (void)argv;  /* Suppress unused parameter warnings. */

  /* If error returns to outer-most main, abort. */
  ERR_ABRT_ON_ERR(math_example(), stderr);

  printf("Exiting\n");  fflush(stdout);

  return 0;
}  /* main */
