/* err.h - Error handling infrastructure.
 * See https://github.com/fordsfords/err for documentation.
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

#ifndef ERR_H
#define ERR_H

#include <string.h>
#include <stdlib.h>

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/* For a Windows build of err.c, set the preprocessor symbol ERR_EXPORTS
 * if you want this subsystem to be available from a DLL.
 */
#if defined(_WIN32)
#  ifdef ERR_EXPORTS
#    define ERR_API __declspec(dllexport)
#  else
#    define ERR_API __declspec(dllimport)
#  endif
#else
#  define ERR_API
#endif

#define ERR_OK NULL

/* Simple macro to skip past the dir name of a full path (if any). */
#if defined(_WIN32)
#  define ERR_BASENAME(_p)\
    ((strrchr(_p, '\\') == NULL) ? (_p) : (strrchr(_p, '\\')+1))
#else
#  define ERR_BASENAME(_p)\
    ((strrchr(_p, '/') == NULL) ? (_p) : (strrchr(_p, '/')+1))
#endif

/* Applications that return an err_t should be declared with this macro. */
#define ERR_F __attribute__ ((__warn_unused_result__)) err_t *


/* Throwing an error means creating an err object and returning it. */
#define ERR_THROW(_code, _mesg) do { \
  return err_throw(__FILE__, __LINE__, _code, _mesg); \
} while (0)


/* Assert/throw combines sanity test with throw. */
#define ERR_ASSRT(_cond_expr, _code) do { \
  if (!(_cond_expr)) { \
    return err_throw(__FILE__, __LINE__, _code, #_cond_expr); \
  } \
} while (0)


/* Shortcut abort-on-error macro. Prints stack trace to stderr. */
#define ERR_ABRT(_funct_call, _stream) do { \
  err_t *_err = (_funct_call); \
  if (_err) { \
    _err = err_rethrow(__FILE__, __LINE__, _err, _err->code, #_funct_call); \
    fprintf(_stream, "ERR_ABRT Failed!\nStack trace:\n----------------\n"); \
    err_print(_err, _stream); \
    fflush(_stream); \
    abort(); \
  } \
} while (0)


/* Implicit re-throw. */
#define ERR(_funct_call) do { \
  err_t *_err = (_funct_call); \
  if (_err) { \
    return err_rethrow(__FILE__, __LINE__, _err, _err->code, #_funct_call); \
  } \
} while (0)


/* Explicit re-throw. */
#define ERR_RETHROW(_err, _code) do { \
  return err_rethrow(__FILE__, __LINE__, _err, _err->code, \
                     "Re-throwing " #_err); \
} while (0)


struct err_s;  /* Forward def to allow self-reference. */


/* Internal structure of err object. Application is allowed to look. */
typedef struct err_s {
  int code;
  char *file;
  int line;
  char *mesg;  /* Separately malloced. */
  struct err_s *stacktrace;  /* Linked list. */
} err_t;


/* Print stack trace to an open FILE pointer (like stderr). */
ERR_API void err_print(err_t *err, FILE *stream);

/* If an error is handled and not re-thrown, the err object must be deleted. */
ERR_API void err_dispose(err_t *err);


/* These generally should not be called directly by applications. The
 * macro forms are preferred.
 */
ERR_API err_t *err_throw(char *file, int line, int code, char *msg);
ERR_API err_t *err_rethrow(char *file, int line, err_t *in_err, int code, char *msg);


#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif  /* ERR_H */
