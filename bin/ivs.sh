#!/usr/bin/env bash

IVS_USER="$USER"
IVS_TEMP_DIR="~$USER/opengl-examples-ivs-temp"


#NODES_SHORT="node1 node2 node3 node4 node5 node6 node7 node8"
NODES_SHORT="node1 node2 node3 node4 node5 node6 node7 node8"
NODES=""
for i in $NODES_SHORT; do
    NODES="${NODES}${i}.ivs.research.mtu.edu "
done
# Which host should we use to rsync our files to?
RSYNC_DEST="node1.ivs.research.mtu.edu"


# If we exit unexpectedly, kill all of the background processes.
trap 'cleanup' ERR   # process exits with non-zero exit code
trap 'cleanup' INT   # Ctrl+C
cleanup() {
	echo "Jobs running before cleanup:"
	jobs -p -r
	
	echo
	sleep .2

	for i in $NODES; do
		echo "Killing jobs on $i..."
		ssh -q -t -t -x -M ${IVS_USER}@${i} "killall ${1} xterm; sleep .2; echo 'jobs on node after'; ps ux"
	done

	sleep .2
	rm ./.temp-dgr-*
	killall dgr-relay

    # Redirect stdout and stderr to /dev/null--- the user doesn't
    # need to see what was killed. Sometimes we'll get a message
    # about 'no such process' from kill. This seems to be that
    # "jobs -p" creates a list of jobs that is a little out of
    # date, or the PIDs die before kill has a chance to kill them.
	kill `jobs -p -r` &> /dev/null
	sleep .3

	echo "Jobs to be terminated forcefully:"
	jobs -p -r

	kill -TERM `jobs -p -r` &> /dev/null
	stty sane
	echo
	exit 1
}


# Print a message and exit if this process is running on a particular host name.
function denyHostname()
{
    if [[ ${HOSTNAME} == "$1" ]]; then
        echo "This program is not intended to be run on host '${HOSTNAME}'."
        echo "Typically you run this script from the IVS desktop machine in the lab or your laptop."
        exit 1
    fi
}


function printMessage()
{
    echo "$(tput sgr 0)$(tput bold)== $BASH_SOURCE ===> $(tput setaf 1)$@$(tput sgr 0)"
}


for i in ${@}; do
	if [[ $i == "--config" ]]; then
		echo "Do not pass --config options to program when using ivs.sh"
		exit 1
	fi
done

for i in ivs.research.mtu.edu ${NODES}; do
    denyHostname "$i"
done

# Make sure our current directory is the same place as where this script is.
SCRIPT_DIRECTORY="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
if [[ "${SCRIPT_DIRECTORY}" != "${PWD}" ]]; then
    echo "This script is in '${SCRIPT_DIRECTORY}'."
    echo "You are currently in '${PWD}'."
    echo
    echo "Use 'cd $SCRIPT_DIRECTORY' to change into the same directory as this script and try again."
    exit 1
fi

# Ensure that we were called with the correct parameters.
if [[ $# -lt 1 ]]; then
    echo "Usage:"
    echo "$0 opengl-dgr-program arguments to program"
        echo
        echo "If you had a program called 'model' that loaded a model file and had DGR support, you would run:"
        echo "$0 model /path/to/model.file"
    exit 1
fi


# Add local directory to our PATH so user can use "exectuable" instead of "./executable"
PATH=.:$PATH

printMessage "Making sure everything is recompiled on this computer..."
make --quiet "$1" dgr-relay
make --quiet "$1"

# Make sure that the dgr-relay and the program we want to execute exist and are executable.
if [[ ! -x dgr-relay ]]; then
    echo "dgr-relay program is missing or not executable. Have you compiled it?"
    exit 1
fi
if [[ ! -x "$1" ]]; then
    echo "The program you want to run ($1) is missing or not executable. Have you compiled it?"
    exit 1
fi


printMessage "Copying files to $IVS_TEMP_DIR on ${RSYNC_DEST}..."
# We only need to copy this once to our home directory
rsync -ah -e ssh --exclude=.git --exclude=CMakeFiles --exclude=doxygen-docs --exclude=CMakeCache.txt --exclude=CMakeFiles --partial --no-whole-file --inplace --delete .. ${IVS_USER}@${RSYNC_DEST}:${IVS_TEMP_DIR}

printMessage "Compiling program on ${RSYNC_DEST}..."
SCRIPT_FILE=.temp-dgr-compile.sh
cat <<EOF > "${SCRIPT_FILE}"
	cd ${IVS_TEMP_DIR}
	./cleanup.sh
	cmake .
	make -j6 "$1" dgr-relay
EOF
ssh -q -t -t -x -M ${IVS_USER}@${i} "$(cat $SCRIPT_FILE)"



printMessage "Connecting to IVS computers..."

# Start X on each computer
for i in $NODES; do
    printMessage "Starting on X on ${i}"

	EXEC="$1"
	shift # remove IVS from args
	shift # remove exec from args
	
	SCRIPT_FILE=".temp-dgr-run-$i.sh"
	# use \$ to write a literal $ into the file
cat <<EOF > "${SCRIPT_FILE}"
# Kill X if it is already running.
killall xinit
sleep 1

export PATH=.:${PATH}


# Start X
xinit -- :1 &
export DISPLAY=:1
sleep 3

# Disable screen blanking
xset s off -dpms

# Check if we can connect to X
xdpyinfo > /dev/null
if [[ $? != 0 ]]; then
     echo "X failed to start on $HOSTNAME"
	 exit 0
fi

# Test program
#glxgears -fullscreen

# Run our program with the arguments passed to it:
cd ${IVS_TEMP_DIR}/bin
echo ${EXEC} --config config/ivs/${i%%.*}.ini "${@}"
${EXEC} --config config/ivs/${i%%.*}.ini "${@}"

# Kill any running jobs (for example, xinit) after our program exits.
# kill xterm window which automatically gets created by xinit first.
killall xterm
#sleep 1
# X should exit gracefully, if not kill it.
#kill `jobs -r -p` &> /dev/null
EOF

printMessage "Starting node $i"
ssh -q -t -t -x -M ${IVS_USER}@${i} "$(cat $SCRIPT_FILE)" &

#sleep 5   # Sleep between each computer we ssh into.
done

#printMessage "Starting dgr-relay..."
# dgr-relay 5700 141.219.23.98 5060 
# 141.219.23.99 5060
# 141.219.23.100 5060
# 141.219.23.101 5060
# 141.219.23.102 5060 
# 141.219.23.103 5060
# 141.219.23.104 5060
# 141.219.23.105 5060 &

sleep 1

printMessage "Starting master"
${1} --config config/ivs/master.ini "${@:2}"

# Loop until all jobs are completed.
while (( 1 )); do
	sleep 1
#	jobs -r
	echo
	if [[ `jobs -r | wc -l` -le 1 ]]; then
		printMessage "Looks like everything finished successfully, cleaning up..."
	cleanup
	exit 0
    fi
done

