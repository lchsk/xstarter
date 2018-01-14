# Installation

## Arch Linux (AUR)

You can use yaourt or similar:

`yaourt -S xstarter`

Alternatively, you can install it manually:

`git clone https://aur.archlinux.org/xstarter.git`

`cd xstarter && makepkg -is`

For more information, see https://aur.archlinux.org/packages/xstarter/

## DEB package

Download `.deb` package

`apt install -f xstarter-*.deb`

## RPM package

Download `.rpm` package

`dnf install xstarter-*.rpm`

## From source

```
git clone https://github.com/lchsk/xstarter/
cd xstarter && cmake . && make && sudo make install
```

In order to compile it, you need development versions of the following libraries installed:

* ncurses e.g. 5.9, 6.0
* glib2

## Using archives

Download `.tar.gz` or `.zip` archive

Extract it

Run `./bin/xstarter` to open the application in the terminal. You can manually copy it to a directory included in your $PATH.
