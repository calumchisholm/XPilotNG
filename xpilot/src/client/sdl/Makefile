OBJS=../netclient.o ../client.o ../usleep.o ../query.o ../textinterface.o ../datagram.o ../mapdata.o ../gfx2d.o ../messages.o ../clientrank.o ../talkmacros.o ../ignore.o ../paintmap.o ../paintobjects.o sdlevent.o sdlkeys.o todo.o options.o main.o gameloop_x.o sdlinit.o sdlpaint.o sdlgui.o
CFLAGS=-D HAVE_LIBZ -c -I ../../common -I ../ -I . `sdl-config --cflags`
LDFLAGS=-lm -lz `sdl-config --libs` -lGL -lGLU

sdlclient: $(OBJS)
	gcc $(LDFLAGS) -o sdlclient $(OBJS) ../../common/libxpcommon.a
%.o: %.c
	gcc $(CFLAGS) -c -o $@ $<