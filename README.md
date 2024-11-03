# err
Err: Light-weight framework for C API error reporting.


## Table of contents

<!-- mdtoc-start -->
&bull; [err](#err)  
&nbsp;&nbsp;&nbsp;&nbsp;&bull; [Table of contents](#table-of-contents)  
&nbsp;&nbsp;&nbsp;&nbsp;&bull; [Introduction](#introduction)  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&bull; [Goals](#goals)  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&bull; [Shortcomings](#shortcomings)  
&nbsp;&nbsp;&nbsp;&nbsp;&bull; [Usage](#usage)  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&bull; [Preparation](#preparation)  
&nbsp;&nbsp;&nbsp;&nbsp;&bull; [Example 1](#example-1)  
&bull; [include <stdio.h>](#include-stdioh)  
&bull; [include "err.h"](#include-errh)  
<!-- TOC created by '../mdtoc/mdtoc.pl README.md' (see https://github.com/fordsfords/mdtoc) -->
<!-- mdtoc-end -->


## Introduction

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
return status, or does check but only prints something barely helpful.

### Goals

The goals of "err" are to be:
* Lightweight.
Low complexity, small size, few (if any) dependencies.
* Low barrier to use.
Implies easy to use.
In particular, make easy to add sanity checks to your code with minimal
effort or disruption.
* Not a Borg.
"err" co-exists with other error handling methodologies in the same codebase.
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

See http://blog.geeky-boy.com/2014/06/error-handling-enemy-of-readability.html
for some long-winded thoughts on error handling and code readability.
My implementation has evolved since then, but is still based on the same
principles.

### Shortcomings

* "err" is disruptive to add it to an existing API.
You have to change the calling signatures.

* Its output is not end-user friendly (only a developer could love it),
and should probably be limited to a log file that the maintainers look
at more than end users.

## Usage

Here are the principles:

1. All functions are declared as returning an "int", which is 0 for
success or a negative error code for faiure.
1. All functions must be passed a pointer to an "err_t";
by convention as the final parameter.
1. An error is returned using the ERR_ASSRT() or ERR_THROW() macro.
If the caller passes in NULL for the err_t, they call "abort".
Otherwise the error information is passed back in the err_t structure,
and the function returns the error code.

### Preparation

Download the "err" repository from https://github.com/fordsfords/err

I predict that most users will end up customizing the source files to
conform to their application's software ecosystem.
I did not develop "err" with the intention that it would be used as-is;
it is a starting point.

That said, I will consider any and all pull requests if you want to
share your improvements.

For Linux systems, there are two scripts:
* bld.sh - build the package and test program.
* tst.sh - invoke the "bld.sh" to build, then run the test program.

## Example 1

This example shows the simplest usage.

```c
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
	    my_err.file, my_err.line, my_err.func, my_err.code, my_err.msg);
    fflush(stdout);
  }

  return 0;
}  /* main */
```

Here's the output:
````
1/4=0.250000
reciprocal error at [example1.c:8 reciprocal()]: code=-1, msg=Assertion failed: input_value != 0
````
In this case, no core file is generated.
