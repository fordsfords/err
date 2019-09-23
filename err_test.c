/* err_tst.c - self-test.
 *
 * This code and its documentation is Copyright 2019, 2019 Steven Ford,
 * http://geeky-boy.com and licensed "public domain" style under Creative
 * Commons "CC0": http://creativecommons.org/publicdomain/zero/1.0/
 * To the extent possible under law, the contributors to this project have
 * waived all copyright and related or neighboring rights to this work.
 * In other words, you can use this code for any purpose without any
 * restrictions. This work is published from: United States. The project home
 * is https://github.com/fordsfords/err
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "err.h"


ERR_F funct_c(int argc, char **argv)
{
  fprintf(stderr, "funct_c\n"); fflush(stdout); fflush(stderr);
  ERR_THROW(*argv[1], "funct_c always throws");
}


ERR_F funct_b(int argc, char **argv)
{
  err_t *err;
  if (argc > 2) ERR_THROW(argc, "argc is > 2");


  if (*argv[1] == '0') {
    return NULL;
  }
  else if (*argv[1] == '1') {
    char *big_mesg = (char *)malloc(65536*1024+1);
    memset(big_mesg, 'x', 65536*1024);  big_mesg[65536*1024] = '\0';

    err = err_throw(__FILE__, __LINE__, 65536, big_mesg);
    while (1) {
      err = err_rethrow(__FILE__, __LINE__, err, 65535, big_mesg);
    }
  }
  else if (*argv[1] == '2') {
    err = funct_c(argc, argv);
    fprintf(stderr, "funct_b: dispose funct_c's err\n"); fflush(stderr);
    err_dispose(err);
  }
  else {
    err = funct_c(argc, argv); ERR_R(err);
  }

  /* ERR_F uses __attribute__ ((__warn_unused_result__)) which generates a
   * warning if the caller doesn't use the return value.
   */
#ifdef TST_ERR_F
  funct_c(argc, argv);  /* compiler warning. */
#endif

  return NULL;
}


ERR_F funct_a(int argc, char **argv)
{
  err_t *err = NULL;

  if (argc == 1) ERR_THROW(1, "argc is 1");

  err = funct_b(argc, argv);
  if (err) ERR_RETHROW(err, 2, "funct_a: rethrow from funct_b");

  return NULL;
}  /* funct_a */

int main(int argc, char **argv)
{
  err_t *err;

  err = funct_a(argc, argv); ERR_A(err);

  fprintf(stderr, "OK\n"); fflush(stderr);

  return 0;
}  /* main */
