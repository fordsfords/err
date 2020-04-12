# err
Err: methodology for C API error reporting.

The C language does not have a well-established methodology for
APIs to report errors.
Java has exceptions, but C does not.
The closest thing that C has is the Unix common practice in which a
function returns certain valid values that represent success
(the values can vary according to the API function),
and a certain invalid value for failure.
Callers are expected to check the return value for validity,
and refer to "errno" for information about the error.

The goals of "err" are:
* Lightweight.
Low complexity, small size.
* Not a Borg.
"Err" co-exists with other error handling methodologies in the same codebase.
(Naturally, err's benefits, like backtraces,
become more helpful as participation increases.)
* Consistency.
Different APIs use a consistent coding method.
* Non-ephemeral.
If a Unix API call returns -1 (or NULL, or some other failure indicator),
it typically (but not always) sets "errno".
But be careful to save a copy!
Making any other API call will probably change the value of  "errno".
In contrast, "err" returns an error object
(conceptually similar to a Java exception object)
which saves the original error information until it is explicitly deleted.
No chance of accidentally overwriting the error code.
* Backtrace.
"Err" has a primitive backtracing capability that can transition threads.
I.e. thread A sends a request to thread B, which detects an error,
the error object (with backtrace) can be transferred back to thread A,
and backtracing can be resumed.

Note that "err" does not address the higher-level question of how an
application should react to errors,
nor does it address how end users are informed of errors.
"Err" is a low-level coding method for C APIs.
