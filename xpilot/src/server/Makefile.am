
INCLUDES = -I$(top_srcdir)/src/common
bin_PROGRAMS = xpilots
xpilots_SOURCES = \
	alliance.c asteroid.c asteroid.h auth.c auth.h \
	cannon.c cannon.h cell.c click.h cmdline.c collision.c \
	command.c connection.h contact.c \
	defaults.h \
	event.c \
	fileparser.c frame.c \
	global.h \
	id.c item.c \
	laser.c \
	map.c map.h metaserver.c metaserver.h \
	netserver.c netserver.h \
	object.c object.h objpos.c objpos.h option.c \
	parser.c play.c player.c polygon.c proto.h \
	rank.c rank.h recwrap.c recwrap.h \
	robot.c robot.h robotdef.c rules.c \
	saudio.c saudio.h sched.c sched.h score.c score.h \
	server.c server.h serverconst.h ship.c shot.c \
	showtime.c srecord.c srecord.h \
	tuner.c tuner.h \
	update.c \
	walls.c walls.h wildmap.c \
	xp2map.c xpmap.c xpserver.h

xpilots_LDADD =  $(top_builddir)/src/common/libxpcommon.a @X_LIBS@ @X_PRE_LIBS@ @X_EXTRA_LIBS@ -lX11