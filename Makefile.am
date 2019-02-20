### process this file with automake to produce Makefile.in
#
# $Id: Makefile.am,v 1.3 2000/12/12 11:18:14 kazuhiko Exp $
#       

bin_PROGRAMS = bdfresize
man_MANS = bdfresize.1
bdfresize_SOURCES = bdfresize.c charresize.c getopt.c
bdfresize_LDADD = @LIBOBJS@

noinst_HEADERS = getopt.h

EXTRA_DIST = $(man_MANS) def.h memset.c
