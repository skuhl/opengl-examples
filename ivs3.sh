#!/usr/bin/env bash

# Set IVS_USER to your username on IVS. If your username on IVS
# matches your username on this computer, no changse are necessary
IVS_USER="$USER"
IVS_HOSTNAME="ivs.research.mtu.edu"   # This host is also the DGR relay
IVS_TEMP_DIR="/research/${IVS_USER}/temp-dgr"

# DGR networking settings
MASTER_SEND_PORT=5676
# Each tile runs multiple slaves, each slave on the same tile must listen on different ports!
SLAVE1_LISTEN_PORT=5680
SLAVE2_LISTEN_PORT=5681
SLAVE3_LISTEN_PORT=5682
RELAY_LISTEN_PORT=${MASTER_SEND_PORT}
RELAY_SEND_TO_IP=10.1.255.255     # This is a broadcast address to send packets to all tiles
RELAY_SEND_PORT="5680 5681 5682"


# If we exit unexpectedly, kill all of the background processes.
trap 'cleanup' ERR   # process exits with non-zero exit code
trap 'cleanup' INT   # Ctrl+C
cleanup() {
	echo
	# Removing the ssh socket file should kill any ssh processes that use it!
	rm ./.temp-dgr-*
	sleep .2
	printMessage "Deleting any remaining DGR processes..."
	# Redirect stdout and stderr to /dev/null--- the user doesn't
	# need to see what was killed. Sometimes we'll get a message
	# about 'no such process' from kill. This seems to be that
	# "jobs -p" creates a list of jobs that is a little out of
	# date, or the PIDs die before kill has a chance to kill them.
	kill -TERM `jobs -p` &> /dev/null
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

# Make sure the user isn't running this on a host that it isn't supposed to run on.
for i in ivs.research.mtu.edu tile-0-0.local tile-0-1.local tile-0-2.local tile-0-3.local tile-0-4.local tile-0-5.local tile-0-6.local tile-0-7.local; do
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

# Make sure that the dgr-relay and the program we want to execute exist and are executable.
if [[ ! -x dgr-relay ]]; then
    echo "dgr-relay program is missing or not executable. Have you compiled it?"
    exit 1
fi
if [[ ! -x "$1" ]]; then
    echo "The program you want to run ($1) is missing or not executable. Have you compiled it?"
    exit 1
fi


# Create a persistant ssh connection that we will reuse. This will
# just make it so we have to SSH into ivs once (might be slow, might
# prompt for a password) but then subsequent ssh calls are nearly
# instantanious.
#  Use -x to explicitly disable X forwarding since we don't need it (and the user might have specified it as an option in their own ssh config file)
# Use -S to create/specify an ssh control socket.
# Use -t to force tty allocation.
# Use -q to supress warning/diagnostic messages.
# Use -oBatchMode=yes to cause a failure if a password prompt appears.
printMessage "Connecting to IVS..."
rm -rf ./.temp-dgr-ssh-socket
ssh -oBatchMode=yes -q -t -t -x -M -S ./.temp-dgr-ssh-socket ${IVS_USER}@${IVS_HOSTNAME} "sleep 1d" &
sleep 1
if [[ ! -r ./.temp-dgr-ssh-socket ]]; then
	echo "We failed to establish an SSH control socket."
	echo "You can typically resolve this problem by:"
	echo " (1) Creating an ssh key with no password and the default options (run 'ssh-keygen')"
	echo " (2) Copy the contents of ~/.ssh/id_rsa.pub from your computer and paste it into a file named ~/.ssh/authorized_keys on the IVS machine (if the authorized_keys file exists, paste it at the bottom of the file."

	# If we don't exit in this situation, this script would prompt for
	# the password for every SSH call below.
	exit
fi
printMessage "Connected to IVS."

# Create an ssh command with appropriate arguments that we can use
# repeatedly to run programs on IVS. Use -x to explicitly disable X
# forwarding since we don't need it (and the user might have specified
# it as an option in the ssh config file.
SSH_CMD="ssh -q -t -t -x -S ./.temp-dgr-ssh-socket ${IVS_USER}@${IVS_HOSTNAME}"


#printMessage "Deleting contents of $IVS_TEMP_DIR on IVS..."
#${SSH_CMD} rm -rf "$IVS_TEMP_DIR"
printMessage "Creating directory $IVS_TEMP_DIR on IVS..."
${SSH_CMD} mkdir -p "$IVS_TEMP_DIR"

printMessage "Copying files to $IVS_TEMP_DIR on IVS..."
rsync -ah -e ssh --exclude=.svn --exclude=.git --exclude=CMakeCache.txt --exclude=CMakeFiles --checksum --partial --no-whole-file --inplace --delete . ${IVS_USER}@${IVS_HOSTNAME}:${IVS_TEMP_DIR}

printMessage "Running cmake on IVS..."
${SSH_CMD} "cd \"${IVS_TEMP_DIR}\" && rm -rf CMakeCache.txt CMakeFiles && /export/apps/src/cmake/2.8.9/cmake-2.8.9/bin/cmake ."
printMessage "Compiling $1 and dgr-relay IVS..."
${SSH_CMD} make --quiet --jobs=3 -C "${IVS_TEMP_DIR}" "$1" dgr-relay


printMessage "Starting DGR relay."
printMessage "Relay is listening `hostname`:$RELAY_LISTEN_PORT"
printMessage "Relay is sending to ${RELAY_SEND_TO_IP}:${RELAY_SEND_PORT}"
${SSH_CMD} "${IVS_TEMP_DIR}/dgr-relay" ${RELAY_LISTEN_PORT} ${RELAY_SEND_TO_IP} ${RELAY_SEND_PORT} &


HORIZ_LEFT="-3.09"
HORIZ_RIGHT="3.09"
HORIZ_COUNT="2"
HORIZ_SIZE=$(bc <<< "scale=6; ${HORIZ_RIGHT} - ${HORIZ_LEFT}")
HORIZ_SCREEN_SIZE=$(bc <<< "scale=6; ${HORIZ_SIZE} / ${HORIZ_COUNT}")
VERT_BOT="0.28"
VERT_TOP="2.6"
VERT_COUNT="4"
VERT_SIZE=$(bc <<< "scale=6; ${VERT_TOP} - ${VERT_BOT}")
VERT_SCREEN_SIZE=$(bc <<< "scale=6; ${VERT_SIZE} / ${VERT_COUNT}")
NEAR=3.5
FAR=100
BEZEL=.005

# If VIEWMAT_MODE is not set, set to mouse.
if [[ -z "$VIEWMAT_MODE" ]]; then
	export VIEWMAT_MODE="mouse"
fi

export DGR_MODE="master"
export DGR_MASTER_DEST_PORT=${RELAY_LISTEN_PORT}
export DGR_MASTER_DEST_IP=${IVS_HOSTNAME}
export PROJMAT_FRUSTUM="${HORIZ_LEFT} ${HORIZ_RIGHT} ${VERT_BOT} ${VERT_TOP} ${NEAR} ${FAR}"
export PROJMAT_MASTER_FRUSTUM="${DGR_FRUSTUM}"
export PROJMAT_WINDOW_SIZE="1152 432"
printMessage "Starting DGR master on `hostname`"
printMessage "DGR master is sending packets to ${DGR_MASTER_DEST_IP}:${DGR_MASTER_DEST_PORT}"
"${@}" &

# First argument: Hostname of tile
# Second argument: If this is supposed to be the left=0, middle=1, or right=3 screen for that tile
function getFrustum() {
	PROC_NUM=$2
	WINDOW_LEFT_POS=$(bc <<< "scale=6; 1920*${PROC_NUM}")
	case $1 in
		tile-0-0.local)
			FRUSTUM_L=$(bc <<< "scale=6; ${HORIZ_LEFT}+${HORIZ_SCREEN_SIZE}*${PROC_NUM}/3+${BEZEL}")
			FRUSTUM_R=$(bc <<< "scale=6; ${HORIZ_LEFT}+${HORIZ_SCREEN_SIZE}*(${PROC_NUM}+1)/3-${BEZEL}")
			FRUSTUM_B=$(bc <<< "scale=6; ${VERT_BOT}+${VERT_SCREEN_SIZE}*0+${BEZEL}")
			FRUSTUM_T=$(bc <<< "scale=6; ${VERT_BOT}+${VERT_SCREEN_SIZE}*1-${BEZEL}")
			;;
		tile-0-1.local)
			FRUSTUM_L=$(bc <<< "scale=6; ${HORIZ_LEFT}+${HORIZ_SCREEN_SIZE}*${PROC_NUM}/3+${BEZEL}")
			FRUSTUM_R=$(bc <<< "scale=6; ${HORIZ_LEFT}+${HORIZ_SCREEN_SIZE}*(${PROC_NUM}+1)/3-${BEZEL}")
			FRUSTUM_B=$(bc <<< "scale=6; ${VERT_BOT}+${VERT_SCREEN_SIZE}*1+${BEZEL}")
			FRUSTUM_T=$(bc <<< "scale=6; ${VERT_BOT}+${VERT_SCREEN_SIZE}*2-${BEZEL}")
			;;
		tile-0-2.local)
			FRUSTUM_L=$(bc <<< "scale=6; ${HORIZ_LEFT}+${HORIZ_SCREEN_SIZE}*${PROC_NUM}/3+${BEZEL}")
			FRUSTUM_R=$(bc <<< "scale=6; ${HORIZ_LEFT}+${HORIZ_SCREEN_SIZE}*(${PROC_NUM}+1)/3-${BEZEL}")
			FRUSTUM_B=$(bc <<< "scale=6; ${VERT_BOT}+${VERT_SCREEN_SIZE}*2+${BEZEL}")
			FRUSTUM_T=$(bc <<< "scale=6; ${VERT_BOT}+${VERT_SCREEN_SIZE}*3-${BEZEL}")
			;;
		tile-0-3.local)
			FRUSTUM_L=$(bc <<< "scale=6; ${HORIZ_LEFT}+${HORIZ_SCREEN_SIZE}*${PROC_NUM}/3+${BEZEL}")
			FRUSTUM_R=$(bc <<< "scale=6; ${HORIZ_LEFT}+${HORIZ_SCREEN_SIZE}*(${PROC_NUM}+1)/3-${BEZEL}")
			FRUSTUM_B=$(bc <<< "scale=6; ${VERT_BOT}+${VERT_SCREEN_SIZE}*3+${BEZEL}")
			FRUSTUM_T=$(bc <<< "scale=6; ${VERT_BOT}+${VERT_SCREEN_SIZE}*4-${BEZEL}")
			;;
		tile-0-4.local)
			FRUSTUM_L=$(bc <<< "scale=6; ${HORIZ_LEFT}+${HORIZ_SCREEN_SIZE}*(1+${PROC_NUM}/3)+${BEZEL}")
			FRUSTUM_R=$(bc <<< "scale=6; ${HORIZ_LEFT}+${HORIZ_SCREEN_SIZE}*(1+(${PROC_NUM}+1)/3)-${BEZEL}")
			FRUSTUM_B=$(bc <<< "scale=6; ${VERT_BOT}+${VERT_SCREEN_SIZE}*0+${BEZEL}")
			FRUSTUM_T=$(bc <<< "scale=6; ${VERT_BOT}+${VERT_SCREEN_SIZE}*1-${BEZEL}")
			;;
		tile-0-5.local)
			FRUSTUM_L=$(bc <<< "scale=6; ${HORIZ_LEFT}+${HORIZ_SCREEN_SIZE}*(1+${PROC_NUM}/3)+${BEZEL}")
			FRUSTUM_R=$(bc <<< "scale=6; ${HORIZ_LEFT}+${HORIZ_SCREEN_SIZE}*(1+(${PROC_NUM}+1)/3)-${BEZEL}")
			FRUSTUM_B=$(bc <<< "scale=6; ${VERT_BOT}+${VERT_SCREEN_SIZE}*1+${BEZEL}")
			FRUSTUM_T=$(bc <<< "scale=6; ${VERT_BOT}+${VERT_SCREEN_SIZE}*2-${BEZEL}")
			;;
		tile-0-6.local)
			FRUSTUM_L=$(bc <<< "scale=6; ${HORIZ_LEFT}+${HORIZ_SCREEN_SIZE}*(1+${PROC_NUM}/3)+${BEZEL}")
			FRUSTUM_R=$(bc <<< "scale=6; ${HORIZ_LEFT}+${HORIZ_SCREEN_SIZE}*(1+(${PROC_NUM}+1)/3)-${BEZEL}")
			FRUSTUM_B=$(bc <<< "scale=6; ${VERT_BOT}+${VERT_SCREEN_SIZE}*2+${BEZEL}")
			FRUSTUM_T=$(bc <<< "scale=6; ${VERT_BOT}+${VERT_SCREEN_SIZE}*3-${BEZEL}")
			;;
		tile-0-7.local)
			FRUSTUM_L=$(bc <<< "scale=6; ${HORIZ_LEFT}+${HORIZ_SCREEN_SIZE}*(1+${PROC_NUM}/3)+${BEZEL}")
			FRUSTUM_R=$(bc <<< "scale=6; ${HORIZ_LEFT}+${HORIZ_SCREEN_SIZE}*(1+(${PROC_NUM}+1)/3)-${BEZEL}")
			FRUSTUM_B=$(bc <<< "scale=6; ${VERT_BOT}+${VERT_SCREEN_SIZE}*3+${BEZEL}")
			FRUSTUM_T=$(bc <<< "scale=6; ${VERT_BOT}+${VERT_SCREEN_SIZE}*4-${BEZEL}")
			;;
	esac

}


for tile in tile-0-0.local tile-0-1.local tile-0-2.local tile-0-3.local tile-0-4.local tile-0-5.local tile-0-6.local tile-0-7.local ; do
	rm -f "./.temp-dgr-${tile}.sh"
	for PROC_NUM in 0 1 2; do
		WINDOW_SIZE='1920 1080'
		SUB_SLAVE_LISTEN_PORT=$(( ${SLAVE1_LISTEN_PORT} + ${PROC_NUM} ))
		
		printMessage "Constructing script to start slave ${PROC_NUM} on ${tile}. Slave will listen on port ${SUB_SLAVE_LISTEN_PORT}"
		getFrustum ${tile} ${PROC_NUM}
		FRUSTUM="${FRUSTUM_L} ${FRUSTUM_R} ${FRUSTUM_B} ${FRUSTUM_T} ${NEAR} ${FAR}"
		WINDOW_POS="${WINDOW_LEFT_POS} 0"
		
# The while loop below is necessary because we can 'cd' into
# ${IVS_TEMP_DIR} on the tile before it has received the new version
# that the IVS master wrote. This results in messages about "Stale NFS
# file handle". If we find that the executable is not readable, we try
# to cd into the directory again to ensure that we are inside of the
# new directory instead of the deleted one.
		echo "cd ${IVS_TEMP_DIR}
export PATH=.:\$PATH;
export DGR_MODE='slave'
export DGR_SLAVE_LISTEN_PORT='${SUB_SLAVE_LISTEN_PORT}'
export VIEWMAT_MODE='${VIEWMAT_MODE}'
export PROJMAT_WINDOW_POS='${WINDOW_POS}'
export PROJMAT_WINDOW_SIZE='${WINDOW_SIZE}'
export PROJMAT_FRUSTUM='${FRUSTUM}'
export PROJMAT_MASTER_FRUSTUM='${DGR_MASTER_FRUSTUM}'
export DISPLAY=':0.0'
export __GL_SYNC_TO_VBLANK=1
export __GL_SYNC_DISPLAY_DEVICE='DFP-0'
while [[ ! -r $1 ]]; do
   echo '${tile} does not have $1 yet, waiting...'
   sleep .5
   cd ${IVS_TEMP_DIR}
done
${@}  &> ${tile}.log &" >> "./.temp-dgr-${tile}.sh"
		chmod u+x "./.temp-dgr-${tile}.sh"

#export __GL_SYNC_DISPLAY_DEVICE='${SYNC}'

		
	done
done




printMessage "Copying scripts to $IVS_TEMP_DIR on IVS..."
rsync -ah -e ssh --checksum --partial --no-whole-file --inplace ./.temp-dgr*.sh ${IVS_USER}@${IVS_HOSTNAME}:${IVS_TEMP_DIR}

# wait for master and relay to start.
printMessage "Giving time for DGR relay and master to start..."
sleep 1;



printMessage "Running scripts on each IVS slave..."
for tile in tile-0-0.local tile-0-1.local tile-0-2.local tile-0-3.local tile-0-4.local tile-0-5.local tile-0-6.local tile-0-7.local; do
	${SSH_CMD} ssh $tile "${IVS_TEMP_DIR}/.temp-dgr-${tile}.sh" &
done



# Loop until all jobs are completed. If there is only one job
# remaining, it is probably the ssh master connection. We can kill
# that by deleting the socket.
while (( 1 )); do
    sleep 1
#    jobs
#	echo
    if [[ `jobs | wc -l` -eq 1 ]]; then
	printMessage "Looks like everything finished successfully, cleaning up..."
	cleanup
	exit 0
    fi
done
