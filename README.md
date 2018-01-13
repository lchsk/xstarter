# xstarter

**xstarter is an application launcher for Linux**

It lives in terminal and works well with tiling window manages (e.g. xmonad, i3 etc).

Current version: 0.5.2

| Branch | Build |
| --- | --- |
|`master`|[![Build Status](https://travis-ci.org/lchsk/xstarter.svg?branch=master)](https://travis-ci.org/lchsk/xstarter)|
|`dev`|[![Build Status](https://travis-ci.org/lchsk/xstarter.svg?branch=dev)](https://travis-ci.org/lchsk/xstarter)|

## Installation

See [installation guide](INSTALL.md)

## Features

* a clean, simple interface that works on various terminals
* able to search for applications using environment variables (e.g. `$PATH`) and user-provided list of directories
* can be configured to launch via a key-binding
* can optionally open an application in terminal (see key shortcuts)
* remembers previously launched applications allowing to find them more quickly
* allows fuzzy search (parts of the query can be separated with a space)
* applications can be launched with 1, 2, ..., 0 keys, depending on their position in the search results
* fast, uses cache by default
* easy to configure via a single text file

## Running xstarter

**xstarter** needs to be launched from a terminal (`$ xstarter`). It is useful to run it using a key binding, preferably by binding your preferred key to a command starting **xstarter** from a terminal of your choice, e.g. `xterm -e xstarter`. Alternatively, you can simply bind it to `xstarter` in which case xstarter will open itself in a terminal.

You can also create an alias in your shell (e.g. bash, zsh): `alias xs=xstarter`.

It should run on any modern terminal. It is tested on: `xterm`, `rxvt`, `gnome-terminal`, `xfce4-terminal`, `konsole`. In case of any problems please [report it](https://github.com/lchsk/xstarter/issues/new).

## Configuration

[Configuration file](./xstarter.conf) is available and includes comments that explain configuration variables. It is located in the `~/.xstarter.d/` directory.

## Key shortcuts

Use numbers 1..9 and 0 to open an application from the list.

By default, the following shortcuts are available:

| Shortcut | Action |
| --- | --- |
|Return (enter)|Open selected application|
|C-o|Open selected application in terminal (defined by "terminal" variable in the config file, xterm by default)|
|C-n|Move down the list|
|C-p|Move up the list|
|C-g|Quit|
|C-d|Delete entered character|
|C-w|Delete entire query|


## License

GPL