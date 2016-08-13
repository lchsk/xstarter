# xstarter

**xstarter is a terminal-based application launcher for Linux.**

| Branch | Build |
| --- | --- |
|`master`|[![Build Status](https://travis-ci.org/lchsk/xstarter.svg?branch=master)](https://travis-ci.org/lchsk/xstarter)|
|`dev`|[![Build Status](https://travis-ci.org/lchsk/xstarter.svg?branch=dev)](https://travis-ci.org/lchsk/xstarter)|

![xstarter](xstarter.gif)

## Features

* a clean, simple interface that works on various terminals
* can search for applications using environment variables (e.g. `$PATH`) and user-provided list of directories
* can be configured to launch via a key-binding
* allows fuzzy search (parts of the query can be separated with a space)
* fast, uses cache by default
* easy to configure via a single text file

## Installation

```
git clone https://github.com/lchsk/xstarter/
cd xstarter
cmake .
make
```

Run `./bin/xstarter` to open the application in the terminal.

To install:

```
make install
```

## Run xstarter using a key binding

**xstarter** needs to be started from a terminal. If you want to run it using a key binding, bind your preferred shortcut to the `./bin/xstarter_run` file.

It should run on any modern terminal. It is tested on: xterm, rxvt, gnome-terminal, xfce4-terminal, konsole. In case of any problems please [report it](https://github.com/lchsk/xstarter/issues/new).

## Configuration

[Configuration file](./xstarter.conf) is available and includes comments that explain configuration variables. It should be placed in the `~/.xstarter.d/` directory.
