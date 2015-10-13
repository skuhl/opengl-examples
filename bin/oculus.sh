#!/usr/bin/env bash

source common.sh
ensureWorkingDir
parameterCheck "$0" $#
# Add local directory to our PATH so user can use "exectuable" instead of "./executable"
PATH=.:$PATH
findExecutable "$1"



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
