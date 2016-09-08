#!/bin/bash

############
# defaults #
############

N=10
K=100
S=1
P1=
P2=
AI1=
AI2=

#########################
# parsing argument list #
#########################

function print_usage {
	echo "Usage: ${BASH_SOURCE[0]} [-n n] [-k k] [-s s] [-p1 x1,y1] [-p2 x2,y2] [-ai1 ai1] [-ai2 ai2]" >&2
	exit 1
}

if (( $# % 2 != 0 )); then
	print_usage
fi

while (( $# != 0 )); do
	ARG_NAME="$1"
	ARG_VALUE="$2"
	shift 2

	case "$ARG_NAME" in
		-n) N="$ARG_VALUE";;
		-k) K="$ARG_VALUE";; 
		-s) S="$ARG_VALUE";;
		-p1) P1="$ARG_VALUE";;
		-p2) P2="$ARG_VALUE";;
		-ai1) AI1="$ARG_VALUE";;
		-ai2) AI2="$ARG_VALUE";;
		*) print_usage;;
	esac
done

#################################
# checking argument correctness #
#################################

if ! [[ "$N" =~ ^[0-9]+$ ]] || (( N < 9 || N > 2147483647 )); then
	echo "Parameter \"n\" out of range or not a number." >&2
	exit 1
elif ! [[ "$K" =~ ^[0-9]+$ ]] || (( K < 1 || K > 2147483647 )); then
	echo "Parameter \"k\" out of range or not a number." >&2
	exit 1
elif ! [[ "$S" =~ ^(0|[1-9][0-9]*)$ ]]; then
	echo "Parameter \"s\" is not a non-negative integer." >&2
	exit 1
elif ! [[ "$P1" == "" || "$P1" =~ ^[1-9][0-9]*,[1-9][0-9]*$ ]]; then
	echo "Parameter \"p1\" is not a corrent point coordinate." >&2
	exit 1
elif ! [[ "$P2" == "" || "$P2" =~ ^[1-9][0-9]*,[1-9][0-9]*$ ]]; then
	echo "Parameter \"p2\" is not a corrent point coordinate." >&2
	exit 1
elif ! [[ "$AI1" == "" || -x "$AI1" ]]; then
	echo "Parameter \"ai1\" is not a correct executable file path." >&2
	exit 1
elif ! [[ "$AI2" == "" || -x "$AI2" ]]; then
	echo "Parameter \"ai2\" is not a correct executable file path." >&2
	exit 1
fi

##########################################
# parsing or computing initial positions #
##########################################

MIN_DIST=8
function dist_too_small {
	A=($1 $2)
	B=($3 $4)

	if (( A[0] - B[0] >= MIN_DIST || B[0] - A[0] >= MIN_DIST || A[1] - B[1] >= MIN_DIST || B[1] - A[1] >= MIN_DIST )); then
		return 1 # bash false
	else
		return 0 # bash true
	fi
}

function no_place_for_other_player {
	if dist_too_small $1 $2 1 1 && dist_too_small $1 $2 $((N-3)) $N; then
		return 0
	else
		return 1
	fi
}

# 45-bit random unsigned integer
RANDOM_RANGE=32768
function large_random {
	echo $((RANDOM+RANDOM*(RANDOM_RANGE+RANDOM*RANDOM_RANGE)))
}

function random_point {
	echo $((1 + `large_random` % (N - 3))) $((1 + `large_random` % N))
}

function random_matching_point {
	while true; do
		CANDIDATE=(`random_point`)
		if ! dist_too_small $1 $2 ${CANDIDATE[@]}; then
			echo ${CANDIDATE[@]}
			return
		fi
	done
}

if [[ "$P1" != "" ]]; then
	P1_ARR=(${P1/,/ })

	if (( P1_ARR[0] > N - 3 || P1_ARR[1] > N )); then
		echo "Parameter \"p1\" out of board bounds." >&2
		exit 1
	fi
fi

if [[ "$P2" != "" ]]; then
	P2_ARR=(${P2/,/ })

	if (( P2_ARR[0] > N - 3 || P2_ARR[1] > N )); then
		echo "Parameter \"p2\" out of board bounds." >&2
		exit 1
	fi
fi

# when picking at random, we pick the second point uniformly until it's okay
# the worst case where it's still possible is for n=15, where the probability can be as low as 1/15, but this is plenty high

while [[ "$P1_ARR" == "" && "$P2_ARR" == "" ]]; do
	P1_CANDIDATE=(`random_point`)
	if ! no_place_for_other_player ${P1_CANDIDATE[@]}; then
		P1_ARR=(${P1_CANDIDATE[@]})
	fi
done

if [[ "$P1_ARR" == "" ]]; then
	if no_place_for_other_player ${P2_ARR[@]}; then
		echo "Specified starting point \"p2\" leaves no legal starting points to choose for the other player." >&2
		exit 1
	fi

	P1_ARR=(`random_matching_point ${P2_ARR[@]}`)
elif [[ "$P2_ARR" == "" ]]; then
	if no_place_for_other_player ${P1_ARR[@]}; then
		echo "Specified starting point \"p1\" leaves no legal starting points to choose for the other player." >&2
		exit 1
	fi

	P2_ARR=(`random_matching_point ${P1_ARR[@]}`)
else
	if dist_too_small ${P1_ARR[@]} ${P2_ARR[@]}; then
		echo "Points \"p1\" and \"p2\" are closer than 8 from each other." >&2
		exit 1
	fi
fi

###########################
# running the gui and ais #
###########################

function pid_tree {
	echo -n "$1 "

	for child in `pgrep -P $1`; do
		pid_tree $child
	done
}

GUI="./sredniowiecze_gui_with_libs.sh"

function init_msg {
	echo "INIT $N $K $1 ${P1_ARR[@]} ${P2_ARR[@]}"
}

if [[ "$AI1" == "" && "$AI2" == "" ]]; then
	# human vs human

	{
		init_msg 1
		init_msg 2
	} | $GUI -human1 -human2 > /dev/null || exit 1
elif [[ "$AI1" != "" && "$AI2" != "" ]]; then
	# ai vs ai

	PIPE="`mktemp -u`"
	mkfifo "$PIPE"
	exec 3<>"$PIPE"
	rm $PIPE

	PIPE="`mktemp -u`"
	mkfifo "$PIPE"
	exec 4<>"$PIPE"
	rm $PIPE

	PIPE="`mktemp -u`"
	mkfifo "$PIPE"
	exec 5<>"$PIPE"
	rm $PIPE

	PIPE="`mktemp -u`"
	mkfifo "$PIPE"
	exec 6<>"$PIPE"
	rm $PIPE

	PIPE="`mktemp -u`"
	mkfifo "$PIPE"
	exec 7<>"$PIPE"
	rm $PIPE

	{
		$AI1 <&3 >&4
		AI1_ERROR_CODE=$?
		if (( AI1_ERROR_CODE < 0 || AI1_ERROR_CODE > 2 )); then
			exit 1
		fi
	} &
	PID_AI1=$!

	{
		$AI2 <&5 >&6
		AI2_ERROR_CODE=$?
		if (( AI2_ERROR_CODE < 0 || AI2_ERROR_CODE > 2 )); then
			exit 1
		fi
	} &
	PID_AI2=$!

	{
		init_msg 1 >&7
		init_msg 2 >&7

		init_msg 1 >&3
		init_msg 2 >&5

		while true; do
			AI1_MSG=""
			while [[ "$AI1_MSG" != "END_TURN" ]]; do
				read AI1_MSG <&4
				echo $AI1_MSG >&7
				echo $AI1_MSG >&5
			done

			sleep $S

			AI2_MSG=""
			while [[ "$AI2_MSG" != "END_TURN" ]]; do
				read AI2_MSG <&6
				echo $AI2_MSG >&7
				echo $AI2_MSG >&3
			done

			sleep $S
		done
	} &
	PID_MAIN=$!
	
	$GUI <&7
	OK="$?"

	exec 3<&-
	exec 4<&-
	exec 5<&-
	exec 6<&-
	exec 7<&-

	sleep 0.2

	kill -9 `pid_tree $PID_AI1` `pid_tree $PID_AI2` `pid_tree $PID_MAIN` 2> /dev/null

	wait $PID_AI1
	OK="$OK$?"
	wait $PID_AI2
	OK="$OK$?"

	if [[ "$OK" != "000" ]]; then
		exit 1
	fi
else
	# human vs ai

	PIPE="`mktemp -u`"
	mkfifo "$PIPE"
	exec 3<>"$PIPE"
	rm $PIPE

	PIPE="`mktemp -u`"
	mkfifo "$PIPE"
	exec 4<>"$PIPE"
	rm $PIPE

	{
		if [[ "$AI1" != "" ]]; then
			$AI1 <&3 >&4
		else
			$AI2 <&3 >&4
		fi
		AI_ERROR_CODE=$?
		if (( AI_ERROR_CODE < 0 || AI_ERROR_CODE > 2 )); then
			exit 1
		fi
	} &
	AI_PID=$!

	{
		if [[ "$AI1" != "" ]]; then
			GUI_ARGS="-human2"
		else
			GUI_ARGS="-human1"
		fi

		$GUI $GUI_ARGS <&4 >&3 || exit 1
	} &
	GUI_PID=$!

	init_msg 1 >&4
	init_msg 2 >&4

	if [[ "$AI1" != "" ]]; then
		init_msg 1 >&3
	else
		init_msg 2 >&3
	fi

	wait $GUI_PID
	OK="$?"

	exec 3<&-
	exec 4<&-

	sleep 0.2

	kill -9 `pid_tree $AI_PID` 2> /dev/null

	wait $AI_PID
	OK="$OK$?"

	if [[ "$OK" != "00" ]]; then
		exit 1
	fi
fi
