/* err_test.c - self-test.
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

#define E(e__test) do { \
  err_t *e__err = (e__test); \
  if (e__err != ERR_OK) { \
    printf("ERROR [%s:%d]: '%s' returned error\n", __FILE__, __LINE__, #e__test); \
    ERR_ABRT_ON_ERR(e__err, stdout); \
    exit(1); \
  } \
} while (0)

#define ASSRT(assrt__cond) do { \
  if (! (assrt__cond)) { \
    printf("ERROR [%s:%d]: assert '%s' failed\n", __FILE__, __LINE__, #assrt__cond); \
    exit(1); \
  } \
} while (0)


/* Options */
int o_testnum;


char usage_str[] = "Usage: err_test [-h] [-t testnum]";
void usage(char *msg) {
  if (msg) fprintf(stderr, "\n%s\n\n", msg);
  fprintf(stderr, "%s\n", usage_str);
  exit(1);
}  /* usage */

void help() {
  printf("%s\n"
    "where:\n"
    "  -h - print help\n"
    "  -t testnum - Specify which test to run [1].\n"
    "For details, see https://github.com/fordsfords/err\n",
    usage_str);
  exit(0);
}  /* help */


void parse_cmdline(int argc, char **argv) {
  int i;

  /* Since this is Unix and Windows, don't use getopts(). */
  for (i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-h") == 0) {
      help();  exit(0);

    } else if (strcmp(argv[i], "-t") == 0) {
      if ((i + 1) < argc) {
        i++;
        o_testnum = atoi(argv[i]);
      } else { fprintf(stderr, "Error, -t requires test number\n");  exit(1); }

    } else { fprintf(stderr, "Error, unknown option '%s'\n", argv[i]);  exit(1); }
  }  /* for i */
}  /* parse_cmdline */


ERR_F funct_c()
{
  ERR_THROW(ERR_ERR_PARAM, "funct_c always throws %s", ERR_ERR_PARAM);
}


ERR_F funct_b(int b)
{
  err_t *err;

  if (b == 0) {
    return ERR_OK;
  }

  if (b == 1) {
    ERR_THROW(ERR_ERR_NOMEM, "b is %d", b);
  }

  if (b == 2) {
    char *big_mesg = (char *)malloc(65536*1024+1);
    memset(big_mesg, 'x', 65536*1024);  big_mesg[65536*1024] = '\0';

    ERR_THROW(ERR_ERR_INTERNAL, big_mesg);
  }

  if (b == 3) {
    err = funct_c();
    if (err) { ERR_RETHROW(err, "b=%d", b); }
  }

#ifdef TST_ERR_F
  /* ERR_F uses __attribute__ ((__warn_unused_result__)) which generates a
   * warning if the caller doesn't use the return value.
   */
  funct_c();  /* compiler warning. */
#endif

  ASSRT(err_asprintf("should not get here") == NULL);
  return ERR_OK;
}  /* funct_b */


void test1() {
  err_t *err;

  char *start_msg = err_asprintf("%s: %s\n", "err_test", "starting");
  ASSRT(strcmp(start_msg, "err_test: starting\n") == 0);
  free(start_msg);

  /* success */
  E(funct_b(0));

  /* printf-style mesg */
  err = funct_b(1);
  ASSRT(strcmp(err->func, "funct_b") == 0);
  ASSRT(err->code == ERR_ERR_NOMEM);
  ASSRT(strcmp(err->mesg, "b is 1") == 0);
  ASSRT(err->stacktrace == NULL);
  err_dispose(err);  /* Since we are handling, delete the err object. */

  /* big mesg */
  err = funct_b(2);
  ASSRT(strcmp(err->func, "funct_b") == 0);
  ASSRT(err->code == ERR_ERR_INTERNAL);
  ASSRT(strlen(err->mesg) == 65536*1024);
  ASSRT(err->mesg[0] == 'x' && err->mesg[65536*1024 - 1] == 'x');
  ASSRT(err->stacktrace == NULL);
  err_dispose(err);

  /* rethrow */
  err = funct_b(3);
  ASSRT(strcmp(err->func, "funct_b") == 0);
  ASSRT(err->code == ERR_ERR_PARAM);
  ASSRT(strcmp(err->mesg, "b=3") == 0);
  ASSRT(err->stacktrace);
  ASSRT(strcmp(err->stacktrace->func, "funct_c") == 0);
  ASSRT(err->stacktrace->code == ERR_ERR_PARAM);
  ASSRT(strcmp(err->stacktrace->mesg, "funct_c always throws ERR_ERR_PARAM") == 0);
  ASSRT(err->stacktrace->stacktrace == NULL);
  err_dispose(err);  /* Since we are handling, delete the err object. */
}  /* test1 */


void test2() {
  /* rethrow abort test */
  ERR_ABRT_ON_ERR(funct_b(3), stderr);

  ASSRT(err_asprintf("should not get here") == NULL);
}  /* test2 */


int main(int argc, char **argv) {
  parse_cmdline(argc, argv);

  if (o_testnum == 0 || o_testnum == 1) {
    test1();
    printf("test1: success\n");
  }

  if (o_testnum == 0 || o_testnum == 2) {
    test2();
    printf("test1: success\n");
  }

  return 0;
}  /* main */
