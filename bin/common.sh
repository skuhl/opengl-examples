#!/usr/bin/env bash
#
# This file contains bash shell functions that are used by other scripts in this folder.


# Prints a highlighted message to stdout. Can be used the same way as the "echo" command.
function printMessage()
{
    echo "$(tput sgr 0)$(tput bold)== $BASH_SOURCE ===> $(tput setaf 1)$@$(tput sgr 0)"
}


# Make sure our current directory is the same place as where this script is.
function ensureWorkingDir()
{
	SCRIPT_DIRECTORY="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
	if [[ "${SCRIPT_DIRECTORY}" != "${PWD}" ]]; then
		echo "This script is in '${SCRIPT_DIRECTORY}'."
		echo "You are currently in '${PWD}'."
		echo
		echo "Use 'cd $SCRIPT_DIRECTORY' to change into the same directory as this script and try again."
		exit 1
	fi
}

# Ensure that we were called with the correct parameters.
# @param The name of the script currently being run.
# @param The number of parameters passed to the program
function parameterCheck()
{
	if [[ $2 -lt 1 ]]; then
		echo "Usage:"
		echo "$1 opengl-dgr-program arguments to program"
		echo
		echo "If you had a program called 'model' that loaded a model file and had DGR support, you would run:"
		echo "$1 model /path/to/model.file"
		exit 1
	fi
}

# Find an executable in the user-specified path.
# @param The executable to find.
function findExecutable()
{
	MISSING=0
	which "$1" > /dev/null || MISSING=1
	
	if [[ ${MISSING} -eq 1 ]]; then
		echo "The program '$1' is missing or not executable. Have you compiled it?"
		exit 1
	fi
}



# The scripts which set up the software on IVS should only be run on
# certain computers. For example they should not be run on tile nodes
# or on the main IVS machine. It should be run on CCSR or your own
# computer---and the script will manage running the programs on the
# other machines.
function ivs_hostcheck()
{
	# ALL_TILES variable needs to be set prior to calling this function!
	for i in ivs.research.mtu.edu ${ALL_TILES}; do
		if [[ ${HOSTNAME} == "$i" ]]; then
			echo "This program is not intended to be run on host '${HOSTNAME}'."
			echo "Typically you run this script from the IVS desktop machine in the lab or your laptop."
			exit 1
		fi
	done
}
