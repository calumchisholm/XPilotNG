# If you don't have freetype 2 installed comment out the next two lines out
FREETYPE2_LIBS=`freetype-config --libs`
FREETYPE2_CFLAGS=-DHAVE_FREETYPE2 `freetype-config --cflags`

OBJS=../netclient.o ../client.o ../event.o ../usleep.o ../query.o ../textinterface.o ../datagram.o ../mapdata.o ../gfx2d.o ../messages.o ../clientrank.o ../talkmacros.o ../clientcommand.o ../paint.o ../paintmap.o ../paintobjects.o sdlevent.o sdlkeys.o todo.o options.o main.o gameloop_x.o sdlinit.o sdlpaint.o sdlgui.o images.o radar.o SDL_gfxPrimitives.o text.o sdlwindow.o console.o SDL_console.o DT_drawtext.o
CFLAGS=-DHAVE_CONFIG_H -DHAVE_LIBZ -g -Wall -Wmissing-prototypes -Wstrict-prototypes -Wmissing-declarations -Wredundant-decls  -Wshadow -I ../../.. -I ../../common -I ../ -I . -I ./console `sdl-config --cflags`
LDFLAGS=-lm -lz `sdl-config --libs` -lGL -lGLU

sdlclient: $(OBJS)
	gcc $(LDFLAGS) $(FREETYPE2_LIBS) -o sdlclient $(OBJS) ../../common/libxpcommon.a
%.o: %.c
	gcc $(CFLAGS) $(FREETYPE2_CFLAGS) -c -o $@ $<
clean:
	rm -f *.o sdlclient
