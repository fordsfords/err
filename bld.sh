#!/bin/sh
# bld.sh

gcc $* -Werror -Wall -g -o err_test err.c err_test.c
if [ $? -ne 0 ]; then exit 1; fi

./tst.sh
if [ $? -ne 0 ]; then exit 1; fi

# Extract examples from README and make sure they build.

sed -n <README.md '/## Example 1/,/## Example 2/p' |
  sed >example1.c '1,/^```c$/d; /^```$/,$d'
gcc $* -Werror -Wall -g -o example1 err.c example1.c
if [ $? -ne 0 ]; then exit 1; fi

sed -n <README.md '/## Example 2/,$p' |
  sed >example2.c '1,/^```c$/d; /^```$/,$d'
gcc $* -Werror -Wall -g -o example2 err.c example2.c
if [ $? -ne 0 ]; then exit 1; fi
