# xstarter

**xstarter is a terminal-based application launcher for *nix systems.**

## Building

```
$ git clone https://github.com/lchsk/xstarter/
$ cd xstarter
$ cmake .
$ make
```

Run `./bin/xstarter` to open the application.

## Run xstarter using a key binding

**xstarter** needs to be started from a terminal. If you want to run it using a key binding, a helpful shell script is available. Bind your preferred shortcut to the `xstarter_run.sh` file in the main directory.
Currently supported terminal emulators include: xterm, *rxvt, gnome-terminal, xfce4-terminal, konsole.

## Configuration

[Configuration file](./xstarter.conf) is available and includes comments that explain configuration variables. It should be placed in the `~/.xstarter.d/` directory.
