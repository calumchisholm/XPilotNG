OBJS=../netclient.o ../client.o ../usleep.o ../query.o ../textinterface.o ../datagram.o ../mapdata.o ../gfx2d.o ../messages.o ../clientrank.o ../talkmacros.o ../clientcommand.o ../paintmap.o ../paintobjects.o sdlevent.o sdlkeys.o todo.o options.o main.o gameloop_x.o sdlinit.o sdlpaint.o sdlgui.o images.o radar.o SDL_gfxPrimitives.o
CFLAGS=-DHAVE_CONFIG_H -DHAVE_LIBZ -g -Wall -Wmissing-prototypes -Wstrict-prototypes -Wmissing-declarations -Wredundant-decls  -Wshadow -I ../../.. -I ../../common -I ../ -I . `sdl-config --cflags`
LDFLAGS=-lm -lz `sdl-config --libs` -lGL -lGLU

sdlclient: $(OBJS)
	gcc $(LDFLAGS) -o sdlclient $(OBJS) ../../common/libxpcommon.a
%.o: %.c
	gcc $(CFLAGS) -c -o $@ $<
clean:
	rm -f *.o sdlclient