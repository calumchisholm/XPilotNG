
bin_PROGRAMS = xpilot
AM_CPPFLAGS = -DCONF_LIBDIR=\"$(pkglibdir)/\"
INCLUDES = -I$(top_srcdir)/src/common
xpilot_SOURCES = \
	about.c \
	bitmaps.c bitmaps.h \
	caudio.c caudio.h client.c client.h \
	clientcommand.c clientcommand.h clientrank.c clientrank.h \
	colors.c configure.c configure.h connectparam.h \
	datagram.c datagram.h dbuff.c dbuff.h \
	debugaudio.c default.c \
	event.c \
	gfx2d.c gfx2d.h \
	guimap.c guimap.h guiobjects.c guiobjects.h \
	icon.h \
	join.c \
	meta.c meta.h mapdata.c messages.c \
	netclient.c netclient.h \
	option.c option.h \
	paint.c paint.h paintdata.c paintdata.h painthud.c \
	paintmap.c paintobjects.c paintradar.c \
	query.c \
	record.c record.h recordfmt.h \
	talk.c talk.h talkmacros.c textinterface.c \
	usleep.c \
	welcome.c widget.c widget.h \
	xdefault.c xevent.c xevent.h xeventhandlers.c xeventhandlers.h \
	xinit.c xinit.h xpaint.c xpaint.h xpclient.h xpclient_x11.h xpilot.c

noinst_LIBRARIES = libxpclient.a
libxpclient_a_SOURCES = \
	netclient.c client.c event.c usleep.c query.c textinterface.c \
	datagram.c meta.c mapdata.c gfx2d.c messages.c clientrank.c \
	talkmacros.c clientcommand.c paint.c paintmap.c paintobjects.c \
	option.c default.c

#xpilot_CFLAGS = @X_CFLAGS@
xpilot_LDADD =  $(top_builddir)/src/common/libxpcommon.a @X_LIBS@ @X_PRE_LIBS@ @X_EXTRA_LIBS@ -lX11
#EXTRA_xpilot_SOURCES = rplayaudio.c debugaudio.c

# From section Conditional subdirectories with AM_CONDITIONAL
# in GNU automake manual.
if COND_NO_SDL
  MAYBE_SDL =
else
  MAYBE_SDL = sdl
endif

# kps - . in SUBDIRS is needed to make sure libxpclient.a is compiled before
# SDL client
SUBDIRS = . items $(MAYBE_SDL)
