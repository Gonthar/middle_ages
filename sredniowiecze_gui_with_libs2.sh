#!/bin/bash -e

DEFAULT_PATH="../../sredniowiecze_gui/sredniowiecze_gui"

if [ -f $DEFAULT_PATH ]; then
	echo "Running compiled sredniowiecze_gui..." >&2
	$DEFAULT_PATH "$@"
elif [[ "$SREDNIOWIECZE_GUI" != "" ]]; then
	$SREDNIOWIECZE_GUI "$@"
else
	echo "Could not find the application to execute. Either compile it to $DEFAULT_PATH or set the SREDNIOWIECZE_GUI environment variable to application to be ran."
fi
