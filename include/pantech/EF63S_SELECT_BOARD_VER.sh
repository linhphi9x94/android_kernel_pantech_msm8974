#!/bin/bash
###############################################################################
#
#                            IMAX Board Rev Select Script 
#
###############################################################################
# 2013-08-08 shpark : created
###############################################################################
LOCAL_V_BOARD_VER=
while [ -z $LOCAL_V_BOARD_VER ]
do
	echo "**************************************************************************"
	echo "*               $CMD_V_UPPER_PRJ_NAME Select Board Version                      "
	echo "**************************************************************************"
	echo " 1. PT10"
	echo " 2. WS10"
	echo " 3. WS15"
	echo " 4. WS20"
	echo " 5. TP10"
	echo " 6. TP20"
	echo " 0. exit"
	echo "**************************************************************************"
	echo .
	echo -n "Which Board Version would you select ? [0] "

	read LOCAL_V_BOARD_VER

	case $LOCAL_V_BOARD_VER in
	1)
		export CMD_V_BOARD_VER=pt10
		;;
	2)
		export CMD_V_BOARD_VER=ws10
		;;
	3)
		export CMD_V_BOARD_VER=ws15
		;;
	4)
		export CMD_V_BOARD_VER=ws20
		;;
        5)
		export CMD_V_BOARD_VER=tp10
		;;
        6)
		export CMD_V_BOARD_VER=tp20
		;;
	*)
		echo "Your selected Board Ver is invalid number"
		LOCAL_V_BOARD_VER=
	esac
	echo
done
