
bin_PROGRAMS = xp-replay
INCLUDES = -I$(top_srcdir)/src/common -I$(top_srcdir)/src/client
xp_replay_SOURCES = buttons.c buttons.h xp-replay.c xp-replay.h
xp_replay_LDADD = $(top_builddir)/src/common/libxpcommon.a @X_LIBS@ @X_PRE_LIBS@ @X_EXTRA_LIBS@ -lX11
SUBDIRS = tools

