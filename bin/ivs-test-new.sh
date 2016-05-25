#!/usr/bin/env bash

PROGRAM="${1}"
ARGS="${@:2}"

# If we exit unexpectedly, kill all of the background processes.
trap 'cleanup' ERR   # process exits with non-zero exit code
trap 'cleanup' INT   # Ctrl+C
cleanup() {
	echo
	echo "Exiting, killing all DGR processes..."
	kill -TERM `jobs -p` &> /dev/null
}


echo "Starting master process"
"${PROGRAM}" --config config/ivs-test-master.ini ${ARGS} &

echo "Starting dgr relay"
./dgr-relay 5676 127.0.0.1 5701 5702 &

sleep .1

echo "Starting slave 1"
"${PROGRAM}" --config config/ivs-test-slave1.ini ${ARGS} &
echo "Starting slave 2"
"${PROGRAM}" --config config/ivs-test-slave2.ini ${ARGS} &


wait
