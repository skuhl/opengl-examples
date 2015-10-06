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

if [[ `hostname` == "aurora" && ${USER} == "kuhl" ]]; then
	# Scott - desktop
	nvidia-settings --assign CurrentMetaMode="DVI-I-1: nvidia-auto-select {ForceFullCompositionPipeline=On}, DP-0: nvidia-auto-select +1600+0 {ForceFullCompositionPipeline=On}"
	export __GL_SYNC_TO_VBLANK=1
	export __GL_SYNC_DISPLAY_DEVICE=DFP-1
	sleep 1
	export PROJMAT_WINDOW_POS="1500 0"
	export PROJMAT_FULLSCREEN="1"
elif [[ `hostname` == "newell" && ${USER} == "kuhl" ]]; then
	# Scott - office
	#nvidia-settings --assign CurrentMetaMode="DPY-1: nvidia-auto-select @1920x1200 +0+240 {ForceFullCompositionPipeline=On, ViewPortIn=1920x1200, ViewPortOut=1920x1200+0+0}, DPY-3: nvidia-auto-select @1280x800 +2970+0 {ForceFullCompositionPipeline=On, ViewPortIn=1280x800, ViewPortOut=1280x800+0+0}, DPY-4: nvidia-auto-select @1050x1680 +1920+0 {ForceFullCompositionPipeline=On, ViewPortIn=1050x1680, ViewPortOut=1680x1050+0+0, Rotation=90}"
#	nvidia-settings --assign CurrentMetaMode="DPY-1: nvidia-auto-select @1920x1200 +0+240 {ForceFullCompositionPipeline=On, ViewPortIn=1920x1200, ViewPortOut=1920x1200+0+0}, DPY-3: nvidia-auto-select @1280x1920 +2970+0 {ForceFullCompositionPipeline=On, ViewPortIn=1080x1920, ViewPortOut=1090x1920+0+0}, DPY-4: nvidia-auto-select @1050x1680 +1920+0 {ForceFullCompositionPipeline=On, ViewPortIn=1050x1680, ViewPortOut=1680x1050+0+0, Rotation=90}"
	
	nvidia-settings --assign CurrentMetaMode="nvidia-auto-select @1920x1200 +0+0 {ForceFullCompositionPipeline=On, ViewPortIn=1920x1200, ViewPortOut=1920x1200+0+0}, DPY-3: nvidia-auto-select @1080x1920 +3600+150 {ForceFullCompositionPipeline=On, ViewPortIn=1080x1920, ViewPortOut=1080x1920+0+0}, DPY-4: nvidia-auto-select @1680x1050 +1920+150 {ForceFullCompositionPipeline=On, ViewPortIn=1680x1050, ViewPortOut=1680x1050+0+0}"

	# Assign primary monitor (that the panel should appear on)
	nvidia-settings --assign XineramaInfoOrder="DPY-1"
	sleep .2
	export PROJMAT_WINDOW_POS="2970 0"
	export PROJMAT_FULLSCREEN="1"
elif [[ `hostname` == "humility" && ${USER} == "kuhl" ]]; then
	# Scott - laptop
	export PROJMAT_WINDOW_POS="1366 0"
	export PROJMAT_FULLSCREEN="1"
fi


export VIEWMAT_DISPLAY_MODE="oculus"

# Different control options
export VIEWMAT_CONTROL_MODE="oculus"  # oculus orientation sensor
# export VIEWMAT_CONTROL_MODE="mouse" # mouse

#export VIEWMAT_CONTROL_MODE="orient"  # other orientation sensor
#export ORIENT_SENSOR_TTY="/dev/ttyUSB0"
#export ORIENT_SENSOR_TYPE="bno055"

"${@}"
