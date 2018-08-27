---
title: Version 0.8.0 released
created: 2018-08-27T00:00:00Z
description: New version of xstarter is available (v0.7.0). It adds one useful feature: you can now provide arguments to the application. It works for GUI and terminal programs alike.
keywords: new release, xstarter, c, application launcher, linux, open source, external programs, blog

---

New version of **xstarter** (v0.8.0) was released today.

Main intention of this release was to enable xstarter to work with external programs. I want to be able to use xstarter via Emacs and I'm working on a small package that will handle that. With this release xstarter is able to print a list of programs that it stored in cache (it can be tested by running `xstarter -P`). External applications can use it load the list of available programs and show it to users. In order to start a program, e.g. Firefox, use command `xstarter -e firefox`. It can be used on its own to start and detach applications from terminal as well by other software.

In this release I've also fixed an interesting bug that broke xstarter if it was built without compiler optimisation. I wrote about that bug [on my blog](https://lchsk.com/when-the-compiler-fixes-your-mistakes.html).

## How to get it?

- [See releases](https://github.com/lchsk/xstarter/releases)

- [Source code](https://github.com/lchsk/xstarter)
