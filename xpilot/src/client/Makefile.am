bin_PROGRAMS = xpilot-ng-x11
AM_CPPFLAGS = -DCONF_DATADIR=\"$(pkgdatadir)/\" -I$(top_srcdir)/src/common
xpilot_ng_x11_SOURCES = \
	about.c \
	bitmaps.c bitmaps.h \
	colors.c configure.c configure.h connectparam.h \
	dbuff.c dbuff.h \
	guimap.c guimap.h guiobjects.c guiobjects.h \
	icon.h \
	join.c \
	paintdata.c paintdata.h painthud.c paintradar.c \
	record.c record.h recordfmt.h \
	talk.c talk.h \
	welcome.c widget.c widget.h \
	xdefault.c xevent.c xevent.h xeventhandlers.c xeventhandlers.h \
	xinit.c xinit.h xpaint.c xpaint.h xpclient_x11.h xpilot.c

noinst_LIBRARIES = libxpclient.a
libxpclient_a_SOURCES = \
	client.c client.h \
	clientcommand.c clientcommand.h clientrank.c clientrank.h \
	datagram.c datagram.h default.c \
	event.c \
	gfx2d.c gfx2d.h \
	mapdata.c messages.c meta.c meta.h \
	netclient.c netclient.h \
	paint.c paint.h paintmap.c paintobjects.c \
	option.c option.h \
	query.c \
	talkmacros.c textinterface.c \
	usleep.c \
	xpclient.h

if COND_SOUND
libxpclient_a_SOURCES += caudio.c caudio.h oalaudio.c
endif


#xpilot_CFLAGS = @X_CFLAGS@
xpilot_ng_x11_LDADD = $(top_builddir)/src/client/libxpclient.a $(top_builddir)/src/common/libxpcommon.a @X_LIBS@ @X_PRE_LIBS@ @X_EXTENSIONS_LIB@ @X_EXTRA_LIBS@ -lX11 @SOUND_LIBS@
#EXTRA_xpilot_SOURCES = debugaudio.c

# From section Conditional subdirectories with AM_CONDITIONAL
# in GNU automake manual.
if COND_SDL_CLIENT
  MAYBE_SDL = sdl
else
  MAYBE_SDL =
endif

# kps - . in SUBDIRS is needed to make sure libxpclient.a is compiled before
# SDL client
SUBDIRS = . items $(MAYBE_SDL) NT
