# If you don't have freetype 2 and SDL_ttf installed comment out the next two lines out
# or preferably install them =)
FREETYPE2_LIBS=-lSDL_ttf
FREETYPE2_CFLAGS=-DHAVE_SDL_TTF

OBJS=../netclient.o ../client.o ../event.o ../usleep.o ../query.o ../textinterface.o ../datagram.o ../mapdata.o ../gfx2d.o ../messages.o ../clientrank.o ../talkmacros.o ../clientcommand.o ../paint.o ../paintmap.o ../paintobjects.o sdlevent.o sdlkeys.o todo.o options.o main.o gameloop_x.o sdlinit.o sdlpaint.o sdlgui.o images.o radar.o SDL_gfxPrimitives.o text.o sdlwindow.o console.o SDL_console.o DT_drawtext.o
#
# HACK ALERT: common/xpconfig.h requires CONF_LIBDIR to be defined.
# Currently the SDL client files (in this directory) doesn't use any of
# the CONF_ defines explicitly, so we can define CONF_LIBDIR to dummy.
#
CFLAGS=-DCONF_LIBDIR=dummy -DHAVE_CONFIG_H -DHAVE_LIBZ -g -Wall -Wmissing-prototypes -Wstrict-prototypes -Wmissing-declarations -Wredundant-decls  -Wshadow -I ../../.. -I ../../common -I ../ -I . -I ./console `sdl-config --cflags`
LDFLAGS=-lm -lz `sdl-config --libs` -lGL -lGLU

xpilot_sdl: $(OBJS)
	gcc $(LDFLAGS) $(FREETYPE2_LIBS) -o xpilot_sdl $(OBJS) ../../common/libxpcommon.a
%.o: %.c
	gcc $(CFLAGS) $(FREETYPE2_CFLAGS) -c -o $@ $<
clean:
	rm -f *.o xpilot_sdl
