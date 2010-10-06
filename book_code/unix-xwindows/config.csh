#! /bin/csh -f
#! File: config.csh
#! Usage: csh config.csh flag command . . .
#! Last modified on Sat Oct  1 12:54:21 1994 by eroberts
#! -----------------------------------------------------
#! This shell script is used inside Makefiles to parameterize
#! compilations for different Unix systems.  The effect of the
#! script is to execute the command and its arguments silently,
#! throwing away any output to stdout or stderr.  If the command
#! succeeds, the config.csh program echoes the flag parameter; if not,
#! it generates no output.  The typical use of the program is
#! within backquotes, as follows:
#! 
#!     gcc `csh config.csh -DCFLAG gcc -E -DCFLAG testfile.c` -c file.c
#! 
#! If the internal command
#! 
#!     gcc -E -DCFLAG testfile.c
#! 
#! succeeds, the outer compilation will define the CFLAG macro;
#! if not, the macro will be undefined.  Thus, if the option
#! flag works correctly in the test case, it is used in the
#! other compilations as well.
#!
eval $argv[2-] >& /dev/null
if ($status == 0) echo $1
