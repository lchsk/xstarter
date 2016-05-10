#!/bin/bash

DIR=`dirname $0`
term=$("$DIR/xstarter" "-t")

`$term -e "$DIR/xstarter" "-p"`

path=`cat "/tmp/.xstarter"`
rm -f "/tmp/.xstarter"

"$path" & disown $!
