#!/bin/bash
CURR_DIR="`dirname $0`"
export QT_QPA_PLATFORM_PLUGIN_PATH="$CURR_DIR/platforms"
export LD_LIBRARY_PATH="$CURR_DIR/qt5libs/:$LD_LIBRARY_PATH"
$CURR_DIR/sredniowiecze_gui $@
