
INCLUDES = -I$(top_srcdir)/src/common
bin_PROGRAMS = xpilots
xpilots_SOURCES = \
	alliance.c asteroid.c auth.c cannon.c cell.c cmdline.c \
	collision.c command.c contact.c \
	event.c fileparser.c frame.c \
	id.c item.c laser.c \
	map.c metaserver.c netserver.c \
	object.c objpos.c option.c \
	parser.c play.c player.c polygon.c \
	rank.c recwrap.c robot.c robotdef.c rules.c \
	saudio.c sched.c score.c server.c \
	ship.c shipshape_s.c shot.c showtime.c srecord.c \
	tuner.c update.c walls.c walls_old.c wildmap.c xpmap.c xp2map.c
xpilots_LDADD = $(top_builddir)/src/common/libxpcommon.a
