
bin_PROGRAMS = xpilot
INCLUDES = -I$(top_srcdir)/src/common
xpilot_SOURCES = \
        about.c bitmaps.c \
        caudio.c client.c clientrank.c colors.c configure.c \
        datagram.c dbuff.c default.c \
        gfx2d.c gfx3d.c guimap.c guiobjects.c \
        ignore.c join.c mapdata.c messages.c netclient.c \
        paint.c paintdata.c painthud.c paintmap.c \
        paintobjects.c paintradar.c query.c record.c shipshape_c.c sim.c \
        syslimit.c talk.c talkmacros.c textinterface.c texture.c \
        usleep.c welcome.c widget.c xeventhandlers.c \
        xevent.c xinit.c xpilot.c xpmread.c
#xpilot_CFLAGS = @X_CFLAGS@
xpilot_LDADD =  $(top_builddir)/src/common/libxpcommon.a @X_LIBS@ @X_PRE_LIBS@ @X_EXTRA_LIBS@ -lX11
EXTRA_xpilot_SOURCES = ncdaudio.c rplayaudio.c afaudio.c hpaudio.c debugaudio.c