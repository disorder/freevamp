## Process this file with automake to produce Makefile.in

bin_PROGRAMS = freevamp

datadir = @datadir@
localedir = $(datadir)/locale
INCLUDES = -DLOCALEDIR=\"$(localedir)\" -I$(srcdir)/intl @GTK_CFLAGS@
LIBS = @INTLLIBS@ @LIBS@
freevamp_LDADD = @GTK_LIBS@
freevamp_SOURCES = freevamp.c fvicon.h copying.c getopt.c getopt.h getopt1.c \
	gettext.h

EXTRA_DIST = config.rpath  copying.awk

copying.c: COPYING copying.awk
	$(AWK) -f $(srcdir)/copying.awk < $(srcdir)/COPYING > $@

SUBDIRS = intl m4 po

ACLOCAL_AMFLAGS = -I m4
