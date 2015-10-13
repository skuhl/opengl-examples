#!/usr/bin/env bash

source common.sh
ensureWorkingDir
parameterCheck "$0" $#
# Add local directory to our PATH so user can use "exectuable" instead of "./executable"
PATH=.:$PATH
findExecutable "$1"


#export VIEWMAT_DISPLAY_MODE="hmd"
export VIEWMAT_CONTROL_MODE="orient"
export ORIENT_SENSOR_TTY="/dev/ttyUSB0"
export ORIENT_SENSOR_TYPE="bno055"
"${@}"

