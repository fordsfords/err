#!/bin/sh
# tst.sh

echo TST_ERR_F
gcc $* -DTST_ERR_F -Werror -Wall -g -o err_test err.c err_test.c 2>tmp.gcc
ST=$?
if [ $ST -eq 0 ]; then :
  echo "ERROR, should not have compiled clean!" >&2
  exit 1
else :
  echo "OK"
fi
echo ""

./bld.sh

echo "./err_test x x (ignore any abort / core dump message)"
./err_test x x 2>tmp.1
ST=$?; echo "ST=$ST"
egrep -v "Aborted .core dumped" <tmp.1 | perl -e \
' $_=<>; /^ERR_ABRT Failed!$/                                    || die"ERR:$_";
  $_=<>; /^Stack trace:$/                                        || die"ERR:$_";
  $_=<>; /^----------------$/                                    || die"ERR:$_";
  $_=<>; /^File: err_test.c$/                                    || die"ERR:$_";
  $_=<>; /^Line: /                                               || die"ERR:$_";
  $_=<>; /^Code: 2$/                                             || die"ERR:$_";
  $_=<>; /^Mesg: funct_a\(argc, argv\)$/                         || die"ERR:$_";
  $_=<>; /^----------------$/                                    || die"ERR:$_";
  $_=<>; /^File: err_test.c$/                                    || die"ERR:$_";
  $_=<>; /^Line: /                                               || die"ERR:$_";
  $_=<>; /^Code: 2$/                                             || die"ERR:$_";
  $_=<>; /^Mesg: funct_a: rethrow from funct_b$/                 || die"ERR:$_";
  $_=<>; /^----------------$/                                    || die"ERR:$_";
  $_=<>; /^File: err_test.c$/                                    || die"ERR:$_";
  $_=<>; /^Line: /                                               || die"ERR:$_";
  $_=<>; /^Code: 3$/                                             || die"ERR:$_";
  $_=<>; /^Mesg: argc is > 2$/                                   || die"ERR:$_";
  $_=<>; ! defined($_) || die "ERR: extra line: $_"; # check EOF
  print "OK\n";
'
if [ $? -ne 0 ]; then exit 1; fi
echo "Test x OK"

echo "./err_test 0"
./err_test 0 2>tmp.0
perl <tmp.0 -e \
' $_=<>; /^OK$/                                                  || die"ERR:$_";
  $_=<>; ! defined($_) || die "ERR: extra line: $_"; # check EOF
  print "OK\n";
'
if [ $? -ne 0 ]; then exit 1; fi
echo "test 0 ok"

# Test 1 doesn't work as expected on MacOS. Null is never returned from strdup.

echo "./err_test 2"
./err_test 2 2>tmp.2
perl <tmp.2 -e \
' $_=<>; /^funct_c$/                                             || die"ERR:$_";
  $_=<>; /^funct_b: dispose funct_c.s err$/                      || die"ERR:$_";
  $_=<>; /^OK$/                                                  || die"ERR:$_";
  $_=<>; ! defined($_) || die "ERR: extra line: $_"; # check EOF
  print "OK\n";
'
if [ $? -ne 0 ]; then exit 1; fi
echo "test 2 ok"

echo "./err_test 3 (ignore any abort / core dump message)"
./err_test 3 2>tmp.3
egrep -v "Aborted .core dumped" <tmp.3 | perl -e \
' $_=<>; /^funct_c$/                                             || die"ERR:$_";
  $_=<>; /^ERR_ABRT Failed!$/                                    || die"ERR:$_";
  $_=<>; /^Stack trace:$/                                        || die"ERR:$_";
  $_=<>; /^----------------$/                                    || die"ERR:$_";
  $_=<>; /^File: err_test.c$/                                    || die"ERR:$_";
  $_=<>; /^Line: /                                               || die"ERR:$_";
  $_=<>; /^Code: 2$/                                             || die"ERR:$_";
  $_=<>; /^Mesg: funct_a\(argc, argv\)$/                         || die"ERR:$_";
  $_=<>; /^----------------$/                                    || die"ERR:$_";
  $_=<>; /^File: err_test.c$/                                    || die"ERR:$_";
  $_=<>; /^Line: /                                               || die"ERR:$_";
  $_=<>; /^Code: 2$/                                             || die"ERR:$_";
  $_=<>; /^Mesg: funct_a: rethrow from funct_b$/                 || die"ERR:$_";
  $_=<>; /^----------------$/                                    || die"ERR:$_";
  $_=<>; /^File: err_test.c$/                                    || die"ERR:$_";
  $_=<>; /^Line: /                                               || die"ERR:$_";
  $_=<>; /^Code: 51$/                                            || die"ERR:$_";
  $_=<>; /^Mesg: funct_c\(argc, argv\)$/                         || die"ERR:$_";
  $_=<>; /^----------------$/                                    || die"ERR:$_";
  $_=<>; /^File: err_test.c$/                                    || die"ERR:$_";
  $_=<>; /^Line: /                                               || die"ERR:$_";
  $_=<>; /^Code: 51$/                                            || die"ERR:$_";
  $_=<>; /^Mesg: funct_c always throws$/                         || die"ERR:$_";
  $_=<>; ! defined($_) || die "ERR: extra line: $_"; # check EOF
  print "OK\n";
'
if [ $? -ne 0 ]; then exit 1; fi
echo "test 3 ok"
