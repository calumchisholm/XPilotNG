/*
 * XPilot, a multiplayer gravity war game.  Copyright (C) 1991-2001 by
 *
 *      Bjørn Stabell        <bjoern@xpilot.org>
 *      Ken Ronny Schouten   <ken@xpilot.org>
 *      Bert Gijsbers        <bert@xpilot.org>
 *      Dick Balaska         <dick@xpilot.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef XPCONFIG_H
#define XPCONFIG_H

#ifndef CONF_LIBDIR
#  ifndef _WINDOWS
#    error "CONF_LIBDIR NOT DEFINED. GIVING UP."
#  else
#    define CONF_LIBDIR			"lib/"
#  endif
#endif

#ifndef CONF_LOCALGURU
#  define CONF_LOCALGURU		"kps@users.sourceforge.net"
#endif

#ifndef CONF_DEFAULT_MAP
#  define CONF_DEFAULT_MAP		"polybloods.xp2"
#endif

#ifndef CONF_MAPDIR
#  define CONF_MAPDIR			CONF_LIBDIR "maps/"
#endif

#ifndef CONF_TEXTUREDIR
#  define CONF_TEXTUREDIR		CONF_LIBDIR "textures/"
#endif

#ifndef CONF_SOUNDDIR
#  define CONF_SOUNDDIR			CONF_LIBDIR "sound/"
#endif

#ifndef _WINDOWS
#  ifndef CONF_DEFAULTS_FILE_NAME
#    define CONF_DEFAULTS_FILE_NAME	CONF_LIBDIR "defaults"
#  endif
#  ifndef CONF_PASSWORD_FILE_NAME
#    define CONF_PASSWORD_FILE_NAME	CONF_LIBDIR "password"
#  endif
#  ifndef CONF_ROBOTFILE
#    define CONF_ROBOTFILE		CONF_LIBDIR "robots"
#  endif
#  ifndef CONF_SERVERMOTDFILE
#    define CONF_SERVERMOTDFILE		CONF_LIBDIR "servermotd"
#  endif
#  ifndef CONF_LOCALMOTDFILE
#    define CONF_LOCALMOTDFILE		CONF_LIBDIR "localmotd"
#  endif
#  ifndef CONF_LOGFILE
#    define CONF_LOGFILE		CONF_LIBDIR "log"
#  endif
#  ifndef CONF_SHIP_FILE
#    define CONF_SHIP_FILE       	""
#  endif
#  ifndef CONF_SOUNDFILE
#    define CONF_SOUNDFILE	  	CONF_SOUNDDIR "sounds"
#  endif
#  ifdef DEBUG
#    define D(x)	x ;  fflush(stdout);
#  else
#    define D(x)
#  endif
#else /* _WINDOWS */
#  ifndef CONF_DEFAULTS_FILE_NAME
#    define CONF_DEFAULTS_FILE_NAME	CONF_LIBDIR "defaults.txt"
#  endif
#  ifndef CONF_PASSWORD_FILE_NAME
#    define CONF_PASSWORD_FILE_NAME	CONF_LIBDIR "password.txt"
#  endif
#  ifndef CONF_ROBOTFILE
#    define CONF_ROBOTFILE		CONF_LIBDIR "robots.txt"
#  endif
#  ifndef CONF_SERVERMOTDFILE
#    define CONF_SERVERMOTDFILE		CONF_LIBDIR "servermotd.txt"
#  endif
#  ifndef CONF_LOCALMOTDFILE
#    define CONF_LOCALMOTDFILE		CONF_LIBDIR "localmotd.txt"
#  endif
#  ifndef CONF_LOGFILE
#    define CONF_LOGFILE		CONF_LIBDIR "log.txt"
#  endif
#  ifndef CONF_SHIP_FILE
#    define CONF_SHIP_FILE		"XPilot.shp"
#  endif
#  ifndef CONF_SOUNDFILE
#    define CONF_SOUNDFILE		CONF_SOUNDDIR "sounds.txt"
#  endif
#  ifdef _DEBUG
#    define DEBUG	1
#    define D(x)	x
#  else
#    define D(x)
#  endif
#endif /* _WINDOWS */

#if 0 /* currently unused */
#ifndef _WINDOWS
#  define CONF_PLAYER_PASSWORDS_FILE_NAME CONF_LIBDIR "player_passwords"
#else
#  define CONF_PLAYER_PASSWORDS_FILE_NAME CONF_LIBDIR "player_passwords.txt"
#endif
#endif

/* Please don't change this one. */
#define CONF_CONTACTADDRESS		"xpilot@xpilot.org"

/*
 * Uncomment this if your machine doesn't use
 * two's complement negative numbers.
 */
/* #define MOD2(x, m) mod(x, m) */

/*
 * The following macros decide the speed of the game and
 * how often the server should draw a frame.  (Hmm...)
 */

#define CONF_UPDATES_PR_FRAME	1

/*
 * If COMPRESSED_MAPS is defined, the server will attempt to uncompress
 * maps on the fly (but only if neccessary). CONF_ZCAT_FORMAT should produce
 * a command that will unpack the given .gz file to stdout (for use in popen).
 * CONF_ZCAT_EXT should define the proper compressed file extension.
 */
#ifndef _WINDOWS
#  define CONF_COMPRESSED_MAPS
#else
/* Couldn't find a popen(), also compress and gzip don't exist. */
#  undef CONF_COMPRESSED_MAPS
#endif
#define CONF_ZCAT_EXT			".gz"
#define CONF_ZCAT_FORMAT 		"gzip -d -c < %s"

/*
 * Windows doesn't play with stdin/out well at all... 
 * So for the client i route the "debug" printfs to the debug stream 
 * The server gets 'real' messages routed to the messages window 
 */
#ifndef _WINDOWS
#  define xpprintf	printf
#else
#  ifdef _XPILOTNTSERVER_
#    define xpprintf	xpprintfW
/* # define xpprintf _Trace  */
#  else
#    define xpprintf	_Trace
#  endif
#endif

/*
 * XPilot on Windows does lots of double to int conversions. So we have:
 * warning C4244: 'initializing' : conversion from 'double ' to 'int ',
 * possible loss of data a million times.  I used to fix each warning
 * added by the Unix people, but this makes for harder to read code (and
 * was tiring with each patch) 
 */
#ifdef	_WINDOWS
#  pragma warning (disable : 4244 4761)
#endif

void Conf_print(void);
char *Conf_libdir(void);
char *Conf_defaults_file_name(void);
char *Conf_password_file_name(void);
char *Conf_player_passwords_file_name(void);
char *Conf_mapdir(void);
char *Conf_default_map(void);
char *Conf_servermotdfile(void);
char *Conf_localmotdfile(void);
char *Conf_logfile(void);
char *Conf_ship_file(void);
char *Conf_texturedir(void);
char *Conf_sounddir(void);
char *Conf_soundfile(void);
char *Conf_localguru(void);
char *Conf_contactaddress(void);
char *Conf_robotfile(void);
char *Conf_zcat_ext(void);
char *Conf_zcat_format(void);

#endif /* XPCONFIG_H */
