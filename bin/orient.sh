#!/usr/bin/env bash

# Ensure that we were called with the correct parameters.
if [[ $# -lt 1 ]]; then
	echo "Usage:"
	echo "$0 opengl-dgr-program arguments to program"
	echo
	echo "If you had a program called 'model' that loaded a model file and had DGR support, you would run:"
	echo "$0 model /path/to/model.file"
	exit
fi

# Make sure our current directory is the same place as where this script is.
SCRIPT_DIRECTORY="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
if [[ "${SCRIPT_DIRECTORY}" != "${PWD}" ]]; then
    echo "This script is in '${SCRIPT_DIRECTORY}'."
    echo "You are currently in '${PWD}'."
    echo
    echo "Use 'cd $SCRIPT_DIRECTORY' to change into the same directory as this script and try again."
    exit 1
fi

if [[ ! -x "$1" ]]; then
    echo "The program you want to run ($1) is missing or not executable. Have you compiled it?"
    exit 1
fi


# Add local directory to our PATH so user can use "exectuable" instead of "./executable"
PATH=.:$PATH

#export VIEWMAT_DISPLAY_MODE="hmd"
export VIEWMAT_CONTROL_MODE="orient"
export ORIENT_SENSOR_TTY="/dev/ttyUSB0"
export ORIENT_SENSOR_TYPE="bno055"
"${@}"

