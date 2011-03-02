#!/bin/bash
################################################################################
# m
#
#    This is a simple script interface to the prjstart buildsystem.
#
#        usage: ./m [target] [options ...]
#
################################################################################

SCRIPTROOT=$(dirname "$0")
cd "$SCRIPTROOT"

if [ -n "$1" -a -f "targets/${1}.mk" ]; then
	TARGETSPEC="MKTARGET=targets/${1}.mk"
	shift
	make "$TARGETSPEC" "$@" 2>&1 | ./buildsystem/colorizebuild
else
	make "$@" 2>&1 | ./buildsystem/colorizebuild
fi
