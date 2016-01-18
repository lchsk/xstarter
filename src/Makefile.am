# what flags you want to pass to the C compiler & linker
AM_CFLAGS = --pedantic -Wall -std=gnu99 $(GLIB_CFLAGS) $(OPENMP_CFLAGS)
LDADD= $(GLIB_LIBS) -lncurses -lmenu

# this lists the binaries to produce, the (non-PHONY, binary) targets in
# the previous manual Makefile
bin_PROGRAMS = $(top_builddir)/bin/xstarter
__top_builddir__bin_xstarter_SOURCES = \
$(top_builddir)/src/main.c \
$(top_builddir)/src/settings.c \
$(top_builddir)/src/utils_string.c \
$(top_builddir)/src/utils.c \
$(top_builddir)/src/scan.c \
$(top_builddir)/src/term.c
