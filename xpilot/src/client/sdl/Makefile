OBJS=sdlevent.o sdlkeys.o todo.o main.o gameloop_x.o sdlinit.o sdlpaint.o sdlgui.o images.o radar.o SDL_gfxPrimitives.o text.o sdlwindow.o console.o SDL_console.o DT_drawtext.o glwidgets.o widget_wrappers.o
#
# HACK ALERT: common/xpconfig.h requires CONF_LIBDIR to be defined.
# Currently the SDL client files (in this directory) doesn't use any of
# the CONF_ defines explicitly, so we can define CONF_LIBDIR to dummy.
#
CFLAGS=-DCONF_LIBDIR=dummy -DOPTIONHACK -DHAVE_CONFIG_H -DHAVE_LIBZ -g -Wall -Wmissing-prototypes -Wstrict-prototypes -Wmissing-declarations -Wredundant-decls  -Wshadow -I ../../.. -I ../../common -I ../ -I . -I ./console `sdl-config --cflags`

LDFLAGS=-lm -lz `sdl-config --libs` -lGL -lGLU -lSDL_ttf

xpilot_sdl: $(OBJS) ../../client/libxpclient.a  ../../common/libxpcommon.a
	gcc $(LDFLAGS) -o xpilot_sdl $(OBJS) ../../client/libxpclient.a  ../../common/libxpcommon.a
%.o: %.c
	gcc $(CFLAGS) -c -o $@ $<
clean:
	rm -f *.o xpilot_sdl
