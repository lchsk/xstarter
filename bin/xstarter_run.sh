#!/bin/bash

DIR=`dirname "$0"`
term=$("$DIR/xstarter" "-t")

if [ "$term" == "gnome-terminal" ]; then
	gnome-terminal -e "$DIR/xstarter -f"
fi

if [ "$term" == "xfce4-terminal" ]; then
	xfce4-terminal -e "$DIR/xstarter -f"
fi

if [ "$term" == "xterm" ]; then
	xterm -e "$DIR/xstarter -f"
fi

if [ "$term" == "konsole" ]; then
	$(konsole -e "$DIR/xstarter" "-f")
fi

if [[ "$term" == *"rxvt"* ]]; then
	$($term -e "$DIR/xstarter" "-f")
fi

path=`cat "/tmp/.xstarter"`
rm -f "/tmp/.xstarter"

"$path" & disown $!
