# err
Err: Light-weight framework for C API error reporting.

The C language does not have a well-established methodology for
APIs to report errors.
Java has exceptions, but C does not.
The closest thing that C has to a common methodology is the Unix common
practice in which a function returns certain valid values that represent
success (the values can vary according to the API function),
and a certain invalid value for failure.
Callers are expected to check the return value for validity,
and refer to "errno" (when available) for information about the error.

In my experience, that kind of error reporting methodology is a recipe
for unreliable programs that are hard to debug and fix.
Thousands of lines of code written that call APIs without checking the
return status,
or does check but only prints something unintelligible even to the
code maintainers.

# Goals

The goals of "err" are to be:
* Lightweight.
Low complexity, small size, few (if any) dependencies.
* Low barrier to use.
Implies easy to use.
In particular, make easy to add sanity checks to your code with minimal
effort or disruption.
* Not a Borg.
"Err" co-exists with other error handling methodologies in the same codebase.
Naturally, "err"'s benefits, like stack traces,
become more helpful as participation increases.
* Consistency.
Different APIs can use a consistent coding method.
* Non-ephemeral.
If a Unix API call returns -1 (or NULL, or some other failure indicator),
it typically (but not always) sets "errno".
But be careful to save a copy!
Making any other API call will probably change the value of  "errno".
In contrast, "err" returns an error object
(conceptually similar to a Java exception object)
which saves the original error information until it is explicitly deleted.
No chance of accidentally overwriting the error code.
* Stack trace.
"Err" has a primitive stack tracing capability, that can even transition thread
boundaries.
I.e. thread A sends a request to thread B, which detects an error,
the error object (with stack trace) can be transferred back to thread A,
and stack tracing can be resumed.

I wish I could say that "err" is perfect.
One negative to it is that it is VERY disruptive to add it to an existing
API.
You have to change the calling signatures; all APIs return an error type.
This is pretty much off the table for established APIs.
But I think that all my new APIs will use it, at least for my own projects.

Note that "err" does not address the higher-level question of how an
application should react to errors,
nor does it address how end users are informed of errors.
Its output is not end-user friendly (only a developer could love it),
and should probably be limited to a log file that the maintainers look
at more than end users.

But I believe that "err" has already improved my own code reliability.

See http://blog.geeky-boy.com/2014/06/error-handling-enemy-of-readability.html
for some long-winded thoughts on error handling and code readability.
My implementation has evolved since then, but is still based on the same
principles.

# Usage

Here are the principles:

1. All functions are declared as returning an "err" object using ERR_F.
Functions that need to return a value must do so via output parameters.
1. A successful return consists of returning ERR_OK (which is defined as
NULL).
1. An error is returned using the ERR_ASSRT() or ERR_THROW() macro,
which create an "err" object and returns it.
1. If your code calls a function that returns an "err" object,
there's a shortcut ERR() if you just want to return that error to your caller
(analogous to an uncaught exception).
Otherwise, you can examine the err object to decide how you want to handle it.
1. If an "err" object is returned all the way to the outer-most
level of a program (i.e. an unhandled "err"), the program can use ERR_ABRT()
to print a stack trace to standard error and invoke "abort()" to dump core.
1. The "err" system supports an integer error code that is intended to
allow a caller to easily check for specific error cases.
The definition of the error codes is the responsibility of the application.
The "err" system does not establish a convention.

## Preparation

Download the "err" repository from https://github.com/fordsfords/err

I predict that most users will end up customizing the source files to
conform to their application's software ecosystem.
I did not develop "err" with the intention that it would be used as-is;
it is a starting point.

That said, I will consider any and all pull requests if you want to
share your improvements.

For Linux systems, you should be able to run the shell script "bld.sh".
This will build and run the self-test,
and will also extract and build (but not run) this README.md's example programs.

## Example 1

This example shows the simplest usage, complete with a code bug.

```c
#include <stdio.h>
#include "err.h"

ERR_F reciprocal(double *result_rtn, double input_value)
{
  /* Sanity checks: assert that things are true that must be true. */
  ERR_ASSRT(input_value != 0, 0);  /* Division by zero now allowed. */

  *result_rtn = 1.0 / input_value;

  return ERR_OK;
}  /* reciprocal */


ERR_F math_example(int argc, char **argv)
{
  double result;

  ERR(reciprocal(&result, 4));
  printf("1/4=%f\n", result);  fflush(stdout);

  ERR(reciprocal(&result, 0));  /* BUG IN THE CODE!!! */
  printf("Should not get here.\n");  fflush(stdout);

  return ERR_OK;
}  /* math_example */


int main(int argc, char **argv)
{
  /* If error returns to outer-most main, abort. */
  ERR_ABRT(math_example(argc, argv), stderr);

  printf("Exiting\n");  fflush(stdout);

  return 0;
}  /* main */
```

Notice the lines:
```c
  ERR(reciprocal(&result, 0));  /* BUG IN THE CODE!!! */
  printf("Should not get here.\n");  fflush(stdout);
```
Since the reciprocal() function asserts that its input must not be zero,
that call to reciprocal returns an active "err" object.
The ERR() macro sees the returned "err" object,
adds some information to the stack trace,
and returns to *its* caller.

Here's the output from running this example:
```
$ ./example1
1/4=0.250000
ERR_ABRT Failed!
Stack trace:
----------------
File: example.c
Line: 32
Code: 0
Mesg: math_example(argc, argv)
----------------
File: example.c
Line: 22
Code: 0
Mesg: reciprocal(&result, 0)
----------------
File: example.c
Line: 7
Code: 0
Mesg: input_value != 0
Abort trap: 6
```

As you can see, you really need the source code to make much sense out of
the stack trace (i.e. not user-friendly).
But for a maintainer, the stack trace is very useful.
Also, the order of the stack trace may be reversed from what you are used to.
The top entry is the outer-most level,
and the bottom entry is where the error was detected.
Notice that the stack trace includes the buggy line `reciprocal(&result, 0)`.

This example doesn't do a user-friendly job of *handling*
the error.
It just bubbles the errors up to main,
which lets ERR_ABRT() print the stack trace and produce a core dump.
That's fine for internal errors that represent code bugs,
but not for user errors, which should produce user-friendly responses.

## Example 2

This interactive example shows more user-friendly error handling,
with a different code bug:

```c
#include <stdio.h>
#include "err.h"

/* Error codes used by the reciprocol API. */
#define E_INTERNAL_ERROR -1
#define E_DIV_ZERO 1

ERR_F reciprocal(double *result_rtn, double input_value)
{
  /* Sanity checks: assert that things are true that must be true. */
  ERR_ASSRT(input_value != 0, E_DIV_ZERO);  /* Division by zero now allowed. */

  if (input_value == 1) {  /* CODE BUG!!! */
    ERR_THROW(E_INTERNAL_ERROR, "Internal consistency check failed");
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

  if (err->code == E_DIV_ZERO) {
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
```

Here's the output from running this example and inputting 3, 0, 1:
```
$ ./example2
Input (floating point number)? 3
Reciprocol of 3.000000 is 0.333333
Input? 0
division by zero not allowed. Try again.
Input? 1
Input? 1
UNHANDLED ERROR. See 'err.log' for details.
Abort trap: 6
$ cat err.log
ERR_ABRT Failed!
Stack trace:
----------------
File: example2.c
Line: 79
Code: -1
Mesg: err
----------------
File: example2.c
Line: 53
Code: -1
Mesg: try_one_reciprocol(input)
----------------
File: example2.c
Line: 43
Code: -1
Mesg: Re-throwing err
----------------
File: example2.c
Line: 14
Code: -1
Mesg: Internal consistency check failed
```

# Random Details

There are a few miscellaneous details.

1. If you think "err" has the feel of a Unix-centric package,
you're right.
Certainly the "bld.sh" and "tst.sh" shell scripts are Unix-only.
I'm a Unix-centric programmer; so sue me.
However, I did invest effort to make the package usable with Windows.
I have at least one (as of this writing) Windows program that uses it.
1. There are a few places where a call to one of "err"s functions or
macros can trigger writes to standard error and/or calling abort() to
trigger a core dump.
This is normally if a dynamic memory call (malloc(), strdup()) returns
NULL.
In my experience,
it makes no sense for a library to simply return an error if out of memory;
I've never seen a program that can recover from that.
1. I've had at least one inquiry whether a Java version could be made
of "err".
My opinion is: why bother?
The "err" package is an attempt to provide some of the benefits of
exceptions to a C program.
If you're using Java, use exceptions.
I'm not sure "err" has any advantages over Java exceptions.
