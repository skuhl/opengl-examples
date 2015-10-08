#!/usr/bin/env bash

# If we exit unexpectedly, kill all of the background processes.
trap 'cleanup' ERR   # process exits with non-zero exit code
trap 'cleanup' INT   # Ctrl+C
cleanup() {
	echo
	echo "Exiting, killing all DGR processes..."
	kill -TERM `jobs -p` &> /dev/null
}

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


# Make sure that the dgr-relay and the program we want to execute exist and are executable.
if [[ ! -x dgr-relay ]]; then
    echo "dgr-relay program is missing or not executable. Have you compiled it?"
    exit 1
fi
if [[ ! -x "$1" ]]; then
    echo "The program you want to run ($1) is missing or not executable. Have you compiled it?"
    exit 1
fi


# Add local directory to our PATH so user can use "exectuable" instead of "./executable"
PATH=.:$PATH

#Connect to IVS tracking system?
#export VIEWMAT_DISPLAY_MODE="ivs"
#export VIEWMAT_CONTROL_MODE="vrpn"
#export VIEWMAT_VRPN_OBJECT="Wand"

#export VIEWMAT_CONTROL_MODE="orient"
#export ORIENT_SENSOR_TTY="/dev/ttyUSB0"
#export ORIENT_SENSOR_TYPE="bno055"


# Start the master process
export DGR_MODE="master"
export DGR_MASTER_DEST_PORT="5676"
export DGR_MASTER_DEST_IP="127.0.0.1"
# Use same aspect ratio as display wall
export PROJMAT_WINDOW_SIZE="720 270"
export PROJMAT_WINDOW_POS="100 100"
#export PROJMAT_FRUSTUM="-1 1 -0.5 0.5 1 100"
#export PROJMAT_MASTER_FRUSTUM="${DGR_FRUSTUM}"

# These frustum values are relative to the origin of the tracked
# space. The code in viewmat.c will adjust the frustum based on the
# eye position (either from the tracking system or based on a typical
# eye position).
BOTTOM=0.28
TOP=2.6
NEAR=3.5
FAR=100
export PROJMAT_FRUSTUM="-3.09 3.09 ${BOTTOM} ${TOP} ${NEAR} ${FAR}"
export PROJMAT_MASTER_FRUSTUM="${PROJMAT_FRUSTUM}"
"${@}" &

sleep .5;

# Start the DGR relay
./dgr-relay 5676 127.0.0.1 5701 5702 &

sleep .5;

# Start the slave processes
# Note that the view frustums are different shapes.
export DGR_MODE="slave"
export DGR_SLAVE_LISTEN_PORT="5701"
export PROJMAT_WINDOW_SIZE="360 270"
export PROJMAT_WINDOW_POS="100 450"
#export PROJMAT_FRUSTUM="-1 0 -.5 .5 1 100"
export PROJMAT_FRUSTUM="-3.09 0 ${BOTTOM} ${TOP} ${NEAR} ${FAR}"
export MSG_LOGFILE="log-ivs-left.txt"
# PROJMAT_MASTER_FRUSTUM is still set.
"${@}" &

sleep .2

export DGR_MODE="slave"
export DGR_SLAVE_LISTEN_PORT="5702"
export PROJMAT_WINDOW_SIZE="360 270"
export PROJMAT_WINDOW_POS="460 450"
#export PROJMAT_FRUSTUM="0 1 -.5 .5 1 100"
export PROJMAT_FRUSTUM="0 3.09 ${BOTTOM} ${TOP} ${NEAR} ${FAR}"
export MSG_LOGFILE="log-ivs-right.txt"
# PROJMAT_MASTER_FRUSTUM is still set.
"${@}" &



# Don't exit shell script until we finish all background processes.
wait
