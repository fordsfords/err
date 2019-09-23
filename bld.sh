#!/bin/sh
# bld.sh

gcc $* -Werror -Wall -g -o err_test err.c err_test.c
