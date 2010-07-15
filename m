#!/bin/bash
################################################################################
# m
#
#    This is a simple script to colorize the output of the build. It can be
#    used the same as make.
#
#        ./m
#        ./m clean
#        ./m VERBOSE=1
#    etc.
################################################################################

SCRIPTROOT=$(dirname "$0")
cd "$SCRIPTROOT"
make "$@" 2>&1 | ./buildsystem/colorizebuild
