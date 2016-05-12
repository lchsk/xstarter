#!/bin/bash

DIR=`dirname $0`
term=$("$DIR/xstarter" "-t")

`$term -e "$DIR/xstarter" "-f"`

path=`cat "/tmp/.xstarter"`
rm -f "/tmp/.xstarter"

"$path" & disown $!
