
bin_PROGRAMS = xpilot-ng-sdl
AM_CPPFLAGS = -DCONF_LIBDIR=\"$(pkglibdir)/\"
INCLUDES = -I$(top_srcdir)/src/common -I$(top_srcdir)/src/client
xpilot_ng_sdl_SOURCES = \
	DT_drawtext.c DT_drawtext.h \
	SDL_console.c SDL_console.h SDL_gfxPrimitives.c SDL_gfxPrimitives.h \
	SDL_gfxPrimitives_font.h console.c console.h gameloop_x.c \
	glwidgets.c glwidgets.h images.c images.h main.c radar.c radar.h \
	sdlevent.c sdlgui.c sdlinit.c sdlkeys.c sdlkeys.h sdlpaint.c \
	sdlpaint.h sdlwindow.c sdlwindow.h text.c text.h todo.c \
	sdlmeta.h sdlmeta.c sdlinit.h

xpilot_ng_sdl_LDADD = $(top_builddir)/src/client/libxpclient.a  $(top_builddir)/src/common/libxpcommon.a @GL_LIBS@ @SDL_EXTRA_LIBS@ @X_LIBS@ @X_PRE_LIBS@ @X_EXTRA_LIBS@ -lX11
