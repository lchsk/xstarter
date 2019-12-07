---
title: xstarter - GNU/Linux terminal application launcher
description: xstarter.org - terminal application launcher for GNU/Linux
keywords: xstarter, c, gnu gpl, linux, gnu, terminal, urxvt, gnome, xterm, konsole, emacs, vim
---

xstarter is an application launcher for Linux

It lives in terminal and works well with tiling window managers (e.g. xmonad, i3 etc).

![xstarter screenshot](./data/xstarter_1.png)

## Features
- a clean, simple interface that works on various terminals
- able to search for applications using environment variables (e.g. $PATH) and user-provided list of directories
- can be configured to launch via a key-binding
- can optionally open an application in terminal (see key shortcuts)
- allows to provide arguments to applications (see key shortcuts)
- remembers previously launched applications allowing to find them more quickly
- allows fuzzy search (parts of the query can be separated with a space)
- applications can be launched with 1, 2, ..., 0 keys, depending on their position in the search results
- fast, uses cache by default
- easy to configure via a single text file

## Running xstarter
xstarter needs to be launched from a terminal ($ xstarter). It is useful to run it using a key binding, preferably by binding your preferred key to a command starting xstarter from a terminal of your choice, e.g. xterm -e xstarter. Alternatively, you can simply bind it to xstarter in which case xstarter will open itself in a terminal.

You can also create an alias in your shell (e.g. bash, zsh): `alias xs=xstarter`.

It should run on any modern terminal. It is tested on: `xterm, rxvt, gnome-terminal, xfce4-terminal, konsole`. In case of any problems please report it.

## Configuration
Configuration file is available and includes comments that explain configuration variables. It is located in the `~/.xstarter.d/` directory.

## Providing arguments to applications
You can provide arguments to applications you wish to open.

If the number of search results is equal to 1, you can provide arguments the selected application should be started with, e.g. `surf xstarter.org`
You can provide multiple arguments, e.g. `emacs -nw -Q`
You can select the application you want to provide arguments for by pressing Tab key
This feature works both with GUI (press Enter to start) and terminal applications (press C-o to start)


## Key shortcuts

Use numbers 1..9 and 0 to open an application from the list.

By default, the following shortcuts are available:

- Return (enter) Open selected application
- C-o Open selected application in terminal (defined by "terminal" variable in the config file, xterm by default)
- C-n Move down the list
- C-p Move up the list
- C-g Quit
- C-d Delete entered character
- C-w Delete entire query
- Tab Auto-complete using current selection; allow to provide arguments

[source code](https://github.com/lchsk/xstarter)

See also: [personal website](https://lchsk.com) | [twitter](https://twitter.com/lchsk) | [postgres linux desktop client](https://lchsk.com/sanchosql)
