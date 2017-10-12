#!/usr/bin/env bash

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


function getFrustum() {
	case $1 in
		node8)
			FRUSTUM_L=$(bc <<< "scale=6; ${HORIZ_LEFT}+${HORIZ_SCREEN_SIZE}*0")
			FRUSTUM_R=$(bc <<< "scale=6; ${HORIZ_LEFT}+${HORIZ_SCREEN_SIZE}*1")
			FRUSTUM_B=$(bc <<< "scale=6; ${VERT_BOT}+${VERT_SCREEN_SIZE}*0")
			FRUSTUM_T=$(bc <<< "scale=6; ${VERT_BOT}+${VERT_SCREEN_SIZE}*1")
			;;
		node7)
			FRUSTUM_L=$(bc <<< "scale=6; ${HORIZ_LEFT}+${HORIZ_SCREEN_SIZE}*0")
			FRUSTUM_R=$(bc <<< "scale=6; ${HORIZ_LEFT}+${HORIZ_SCREEN_SIZE}*1")
			FRUSTUM_B=$(bc <<< "scale=6; ${VERT_BOT}+${VERT_SCREEN_SIZE}*1")
			FRUSTUM_T=$(bc <<< "scale=6; ${VERT_BOT}+${VERT_SCREEN_SIZE}*2")
			;;
		node6)
			FRUSTUM_L=$(bc <<< "scale=6; ${HORIZ_LEFT}+${HORIZ_SCREEN_SIZE}*0")
			FRUSTUM_R=$(bc <<< "scale=6; ${HORIZ_LEFT}+${HORIZ_SCREEN_SIZE}*1")
			FRUSTUM_B=$(bc <<< "scale=6; ${VERT_BOT}+${VERT_SCREEN_SIZE}*2")
			FRUSTUM_T=$(bc <<< "scale=6; ${VERT_BOT}+${VERT_SCREEN_SIZE}*3")
			;;
		node5)
			FRUSTUM_L=$(bc <<< "scale=6; ${HORIZ_LEFT}+${HORIZ_SCREEN_SIZE}*0")
			FRUSTUM_R=$(bc <<< "scale=6; ${HORIZ_LEFT}+${HORIZ_SCREEN_SIZE}*1")
			FRUSTUM_B=$(bc <<< "scale=6; ${VERT_BOT}+${VERT_SCREEN_SIZE}*3")
			FRUSTUM_T=$(bc <<< "scale=6; ${VERT_BOT}+${VERT_SCREEN_SIZE}*4")
			;;
		node4)
			FRUSTUM_L=$(bc <<< "scale=6; ${HORIZ_LEFT}+${HORIZ_SCREEN_SIZE}*1")
			FRUSTUM_R=$(bc <<< "scale=6; ${HORIZ_LEFT}+${HORIZ_SCREEN_SIZE}*2")
			FRUSTUM_B=$(bc <<< "scale=6; ${VERT_BOT}+${VERT_SCREEN_SIZE}*0")
			FRUSTUM_T=$(bc <<< "scale=6; ${VERT_BOT}+${VERT_SCREEN_SIZE}*1")
			;;
		node3)
			FRUSTUM_L=$(bc <<< "scale=6; ${HORIZ_LEFT}+${HORIZ_SCREEN_SIZE}*1")
			FRUSTUM_R=$(bc <<< "scale=6; ${HORIZ_LEFT}+${HORIZ_SCREEN_SIZE}*2")
			FRUSTUM_B=$(bc <<< "scale=6; ${VERT_BOT}+${VERT_SCREEN_SIZE}*1")
			FRUSTUM_T=$(bc <<< "scale=6; ${VERT_BOT}+${VERT_SCREEN_SIZE}*2")
			;;
		node2)
			FRUSTUM_L=$(bc <<< "scale=6; ${HORIZ_LEFT}+${HORIZ_SCREEN_SIZE}*1")
			FRUSTUM_R=$(bc <<< "scale=6; ${HORIZ_LEFT}+${HORIZ_SCREEN_SIZE}*2")
			FRUSTUM_B=$(bc <<< "scale=6; ${VERT_BOT}+${VERT_SCREEN_SIZE}*2")
			FRUSTUM_T=$(bc <<< "scale=6; ${VERT_BOT}+${VERT_SCREEN_SIZE}*3")
			;;
		node1)
			FRUSTUM_L=$(bc <<< "scale=6; ${HORIZ_LEFT}+${HORIZ_SCREEN_SIZE}*1")
			FRUSTUM_R=$(bc <<< "scale=6; ${HORIZ_LEFT}+${HORIZ_SCREEN_SIZE}*2")
			FRUSTUM_B=$(bc <<< "scale=6; ${VERT_BOT}+${VERT_SCREEN_SIZE}*3")
			FRUSTUM_T=$(bc <<< "scale=6; ${VERT_BOT}+${VERT_SCREEN_SIZE}*4")
			;;
	esac
	echo "${FRUSTUM_L} ${FRUSTUM_R} ${FRUSTUM_B} ${FRUSTUM_T} ${NEAR} ${FAR}"
}

for i in node1 node2 node3 node4 node5 node6 node7 node8; do
	INI="$i.ini"
	echo $INI
	rm -f $INI
	touch $INI
	
	echo "include = config/ivs/common.ini" >> $INI
	echo "log.filename = log-$i.txt" >> $INI
	
	echo -n "frustum= " >> $INI
	getFrustum $i >> $INI
	
	#echo "dgr.mode = slave" >> $INI
	#echo "dgr.slave.listenport = 5701" >> $INI
done


	
