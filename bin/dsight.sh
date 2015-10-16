#!/usr/bin/env bash

source common.sh
ensureWorkingDir
parameterCheck "$0" $#
# Add local directory to our PATH so user can use "exectuable" instead of "./executable"
PATH=.:$PATH
findExecutable "$1"


export PROJMAT_DSIGHT="1"
export VIEWMAT_DISPLAY_MODE="hmd"

#export VIEWMAT_DISPLAY_MODE="dsight"
#export VIEWMAT_DSIGHT_FILE="/dev/ttyACM0"
"${@}"

