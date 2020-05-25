# Installation

## Arch Linux (AUR)

Use AUR to install it:

`git clone https://aur.archlinux.org/xstarter.git`

`cd xstarter && makepkg -is`

See xstarter in Arch Linux AUR: https://aur.archlinux.org/packages/xstarter/

## DEB package

Download `.deb` package from https://github.com/lchsk/xstarter/releases

`apt install -f xstarter-*.deb`

## RPM package

Download `.rpm` package from https://github.com/lchsk/xstarter/releases

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

Download `.tar.gz` or `.zip` archive from https://github.com/lchsk/xstarter/releases

Extract it

Run `./bin/xstarter` to open the application in the terminal. You can manually copy it to a directory included in your `$PATH`.
