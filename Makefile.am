## Process this file with automake to produce Makefile.in

bin_PROGRAMS = freevamp

INCLUDES = @GTK_CFLAGS@
freevamp_LDADD = @GTK_LIBS@
freevamp_SOURCES = freevamp.c fvicon.h copying.c getopt.c getopt.h getopt1.c

EXTRA_DIST = copying.awk

copying.c: COPYING copying.awk
	$(AWK) -f $(srcdir)/copying.awk < $(srcdir)/COPYING > $@
