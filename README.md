# err
Light-weight framework for C API error reporting.


## Table of contents

<!-- mdtoc-start -->
&bull; [err](#err)  
&nbsp;&nbsp;&nbsp;&nbsp;&bull; [Table of contents](#table-of-contents)  
&nbsp;&nbsp;&nbsp;&nbsp;&bull; [Quick Start](#quick-start)  
&nbsp;&nbsp;&nbsp;&nbsp;&bull; [Core Features](#core-features)  
&nbsp;&nbsp;&nbsp;&nbsp;&bull; [Key Macros and Functions](#key-macros-and-functions)  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&bull; [Error Handling](#error-handling)  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&bull; [Error Management](#error-management)  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&bull; [Helper Functions](#helper-functions)  
&nbsp;&nbsp;&nbsp;&nbsp;&bull; [Best Practices](#best-practices)  
&nbsp;&nbsp;&nbsp;&nbsp;&bull; [Design Rationale](#design-rationale)  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&bull; [Goals](#goals)  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&bull; [Downsides](#downsides)  
&nbsp;&nbsp;&nbsp;&nbsp;&bull; [Usage](#usage)  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&bull; [Error Codes](#error-codes)  
&nbsp;&nbsp;&nbsp;&nbsp;&bull; [Integration](#integration)  
&nbsp;&nbsp;&nbsp;&nbsp;&bull; [Example 1](#example-1)  
&nbsp;&nbsp;&nbsp;&nbsp;&bull; [Example 2](#example-2)  
&nbsp;&nbsp;&nbsp;&nbsp;&bull; [Random Details](#random-details)  
&nbsp;&nbsp;&nbsp;&nbsp;&bull; [License](#license)  
<!-- TOC created by '../mdtoc/mdtoc.pl README.md' (see https://github.com/fordsfords/mdtoc) -->
<!-- mdtoc-end -->

## Quick Start

```c
// Function that may fail
ERR_F calculate_result(int *result, int input) {
    ERR_ASSRT(input != 0, ERR_ERR_PARAM);  // Parameter validation
    *result = 100 / input;
    return ERR_OK;
}

// Function that uses error handling
ERR_F process_data(int input) {
    int result;
    
    // Call function and propagate any error
    ERR(calculate_result(&result, input));
    
    printf("Result: %d\n", result);
    return ERR_OK;
}

int main() {
    // Abort on unhandled errors with stack trace
    ERR_ABRT_ON_ERR(process_data(0), stderr);
    return 0;
}
```
In the above example, the `ERR_ASSRT()` in calculate_result() will test the condition and "throw" an error that
assertion failed.

The `ERR(calculate_result(&result, input));` in process_data() essentially re-throws the error.

The `ERR_ABRT_ON_ERR(process_data(0), stderr);` in main() essentially catches any uncaught
errors, prints a stack trace and aborts the program, generating a core file.
The idea is that if errors are detected and not caught and handled, you want as much diagnostic
data as possible for the support team.
You can alternatively use `ERR_EXIT_ON_ERR(process_data(0), stderr);` which prints the stack
trace and just does an `exit(1);`

## Core Features

- **Stack Traces**: Captures file, line number, and function names as errors propagate.
- **Unique Error Codes**: String-based error codes prevent collisions between modules.
- **Memory Safe**: Error information persists until explicitly freed.
- **Thread Safe**: Uses no globals.
- **Compiler Enforced**: Warning if error return values are ignored.

## Key Macros and Functions

### Error Handling

- `ERR_F`: Use in your function declarations. Defines return type and attribute for error-returning functions.
  ```c
  ERR_F my_function(int *output, int input) {
    ...
  }  /* my_function */
  ```

- `ERR_THROW(error_code, message)`: Create and return new error. Note that printf-style error messages can be provided.
  ```c
  if (in_state > MAX_STATE) {
    ERR_THROW(ERR_ERR_INTERNAL, "Invalid state: %d", in_state);  /* This returns an err_t. */
  }
  ```

- `ERR_ASSRT(condition, error_code)`: Assert. If the condition is true, continue; else return ("throw") the error.
This leverages `ERR_THROW()` passing the stringified condition in the message.
This is typically used for sanity checks that "should" never fail (the messages are cryptic to end users).
  ```c
  ERR_ASSRT(ptr != NULL, ERR_ERR_PARAM);  /* This returns an err_t on assert failure. */
  ```

- `ERR(function_call)`: Call a function declared with `ERR_F` and check for error. If so, return ("rethrow") the error.
This is the case where you don't expect errors and wouldn't know how to handle them if they happened.
  ```c
  ERR(some_function(args));  /* This returns an err_t on error. */
  ```

- `ERR_RETHROW(err, "message")`: Used when checking a returned error and deciding that you can't handle the error.
  ```c
  err_t *err;
  ...
  err = some_function(args);  /* Not using ERR(). */
  if (err) {
    if (err->code == MY_ERR_RETRY) {
      retry_request = 1;
      err_dispose(err);  /* Necessary since handling it. */
    }
    else {
      ERR_RETHROW(err, "more contextual information");  /* This returns an err_t. */
    }
  }
  ```

### Error Management
- `ERR_ABRT_ON_ERR(function_call, stream)`: Print stack trace and abort (generate core) on error.
- `ERR_EXIT_ON_ERR(function_call, stream)`: Print stack trace and exit(1) on error.
- `ERR_WARN_ON_ERR(function_call, stream)`: Print stack trace and continue.
- `err_dispose(err)`: Free error object when handling error.

### Helper Functions
These wrap common calls with err handling.
I don't want to go overboard, trying to wrap all standard functions;
only a few, especially where I can value besides error handling.
- `err_strdup(char **dst, const char *src)`: Safe string duplication.
- `err_calloc(void **ptr, size_t count, size_t size)`: Safe memory allocation.
- `err_asprintf(char **rtn_str, const char *fmt, ...)`: Safe printf to allocated string.
Caller is responsible for freeing the returned string.
- `err_atol(const char *str, long *value)`: Safe string to long conversion. Leverages `strtol()`.


## Best Practices

* Always use `ERR_F` for functions that can fail. The actual return type is `err_t *`.
* Functions return `ERR_OK` for success (ERR_OK is NULL).
* Return application values through parameters, reserve return value for errors.
* Use `ERR_ASSRT` for parameter validation and other sanity checking.
* Handle or propagate all errors - never ignore them.
* Free error objects with `err_dispose` when handling them.
* Use `ERR_ABRT_ON_ERR` in main() for unhandled errors.

## Design Rationale

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
or does check but only prints something unhelpful even to the
code maintainers.
This leads to:
- Easy-to-ignore errors.
- Lost error context.
- Difficult debugging.
- Error code conflicts between modules.


### Goals

The goals of "err" are to be:
* **Stack Traces**: captures the call stack when errors are detected.
* **Lightweight**: no execution/memory expense during normal (non-error) processing.
* **Low complexity**: small size, no dependencies, low barrier to use.
In particular, make it easy to add sanity checks to your code with minimal
effort or disruption.
* **Not a Borg**:
"Err" co-exists with other error handling methodologies in the same codebase.
You don't have to get married to err.
Naturally, "err"'s benefits, like stack traces,
become more helpful as participation increases.
* **Consistent Pattern**: Unified approach across all error handling
* **Non-ephemeral**:
If a Unix API call returns -1 (or NULL, or some other failure indicator),
it typically (but not always) sets "errno".
But be careful to save a copy!
Making any other API call will probably change the value of  "errno".
In contrast, "err" returns an error object
(conceptually similar to a Java exception object)
which saves the original error information until it is explicitly deleted.
No chance of accidentally overwriting the error code.

### Downsides

* Err monopolizes function return values, forcing you to return values via
input parameters.
* Different underlying values. If I link the ERR system in programs A and B,
the underlying pointer values between the two will be different.
In contrast, simple integer values will be the same.
I don't find this significant because you don't use the underlying value
except within the context of the program itself,
and even then it is used for equality checks.
* Pointers cannot be used in switch statements.
This one did surprise me; I didn't know that raw pointers could not be used
in switch statements.
I'm still not too bothered because I don't think I've ever put an error code
into a switch statement except in the case where I'm translating it into a
string.
With these codes, you already have the string of the identifier,
but you might want to also include a more user-readable sentence.
I still don't find this downside compelling since I would use a little
database for that, keyed by the stringified error code ID.
That approach supports internationalization.
* Disruptive to add it to an existing API.
You have to change the calling signatures; all APIs return an error type.

Note that "err" does not address the higher-level question of how an
application should react to errors,
nor does it address how end users are informed of errors.
Its output is not end-user friendly (only a developer could love it),
and should probably be limited to a log file that the maintainers look
at more than end users.

See http://blog.geeky-boy.com/2014/06/error-handling-enemy-of-readability.html
for some long-winded thoughts on error handling and code readability.
My implementation has evolved since then, but is still based on the same
motivations.


## Usage

Here are the principles:

1. All functions are declared as returning an "err" object using ERR_F.
Functions that need to return a value must do so via output parameters.
1. A successful return consists of returning ERR_OK (which is defined as
NULL).
1. An error is returned using the ERR_ASSRT() or ERR_THROW() macro,
which create an "err" object and returns it.
1. If your code calls a function that returns an "err" object,
you can compare the return value against "ERR_OK" to test for success.
If it's not ok, then you can either handle the error, or you can
ERR_RETHROW() the same error.
1. There's a shortcut, ERR(), that checks for error and rethrows it in one step.
It is very common to call a function using ERR().
1. If an "err" object is returned all the way to the outer-most
level of a program (i.e. an unhandled "err"), the program can use
ERR_ABRT_ON_ERR() to detect the unhandled error, print a stack dump,
and call "abort()", which generates a core file.
Alternatively, you can call "ERR_EXIT_ON_ERR()" to get the stack dump
but call "exit(1)" instead of aborting.
1. The "err" system supports an error code that is intended to
allow a caller to easily check for specific error cases.
See [Error Codes](#error-codes) for details.

### Error Codes

In most error handling approaches, an error code is an integer,
often represented symbolically with a "#define".
This follows the model of Unix where a function might return an
error indicator and set "errno" to a numeric value describing the
error. Those numeric values are assigned symbols in "errno.h".
So the symbol "NOMEM" has the value 12.

One big problem with this approach is that there's no good way to manage those
values across projects.
So, for example, if the "lsim" project uses the "cfg" and "hmap" sub-projects,
each one will define its own set of error code values, and they can alias.
In other words, the number 12 from a "cfg" function might be `CFG_ERR_BADFILE`
while an "hmap" function might treat 12 as `HMAP_ERR_NOTFOUND`.

Also, there is also not an easy way to translate the numeric value into
something more human-readable.

I've established a different approach to error codes.
Instead of an integer, it is a character pointer.
And what does it point at?
A string buffer containing the text of the symbolic name.
So, for example, if a function calls a function from the
"err" project and it returns an error, the error code might be ERR_ERR_NOMEM,
but instead of being an integer, it's a pointer to the string "ERR_ERR_NOMEM".

This is implemented with a big of macro magic.
For example, in "err.h" are the lines:
```
#ifdef ERR_C  /* If "err.c" is being compiled. */
#  define ERR_CODE(err__code) ERR_API char *err__code = #err__code  /* Define them. */
#else
#  define ERR_CODE(err__code) ERR_API extern char *err__code  /* Declare them. */
#endif

ERR_CODE(ERR_ERR_PARAM);
ERR_CODE(ERR_ERR_NOMEM);
ERR_CODE(ERR_ERR_INTERNAL);
#undef ERR_CODE
```

Most users of the "err" project just include the "err.h" header file,
which declares the character pointers external.
But the "err.c" file defines "ERR_C", which defines the pointers and
their content.

So, what does this extra complexity buy you?

If the "cfg" system has its own set of error codes,
and the "hmap" system has its own set of error codes,
they will not alias. In other words, if you get an error code,
it is guaranteed unique since the value is a pointer to a unique string.
You can compare it to expected codes, and if it doesn't match one of
your expected errors, you can at least print it.

## Integration

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

[example1.c](example1.c) shows the simplest usage.

Notice the lines:
````c
  ERR(reciprocal(&result, 0));  /* BUG IN THE CODE!!! */
  printf("Should not get here.\n");  fflush(stdout);
````
Since the reciprocal() function asserts that its input must not be zero,
that call to reciprocal returns an active "err" object.
The ERR() macro sees the returned "err" object,
adds some information to the stack trace,
and returns to *its* caller.

Here's the output from running this example:
````
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
````

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

[example2.c](example2.c) is an interactive example shows more user-friendly
error handling:

Here's the output from running this example and inputting 3, 0, 1:
````
$ ./example2
Input (floating point number)? 3
Reciprocal of 3.000000 is 0.333333
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
Mesg: try_one_reciprocal(input)
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
````


## Random Details

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


## License

I want there to be NO barriers to using this code, so I am releasing it to the public domain.
But "public domain" does not have an internationally agreed upon definition, so I use CC0:

Copyright 2019-2024 Steven Ford http://geeky-boy.com and licensed
"public domain" style under
[CC0](http://creativecommons.org/publicdomain/zero/1.0/):
![CC0](https://licensebuttons.net/p/zero/1.0/88x31.png "CC0")

To the extent possible under law, the contributors to this project have
waived all copyright and related or neighboring rights to this work.
In other words, you can use this code for any purpose without any
restrictions.  This work is published from: United States.  The project home
is https://github.com/fordsfords/err

To contact me, Steve Ford, project owner, you can find my email address
at http://geeky-boy.com.  Can't see it?  Keep looking.
