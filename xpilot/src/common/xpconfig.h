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

#ifndef LIBDIR
#  ifndef _WINDOWS
#    error "LIBDIR NOT DEFINED. GIVING UP."
#  else
#    define LIBDIR			"lib/"
#  endif
#endif

#define LOCALGURU			"kps@users.sourceforge.net"
#define DEFAULT_MAP			"polybloods.xp2"
#define MAPDIR				LIBDIR "maps/"
#define TEXTUREDIR			LIBDIR "textures/"
#define SOUNDDIR			LIBDIR "sound/"
#define ZCAT_EXT			".gz"
#define ZCAT_FORMAT 			"gzip -d -c < %s"

#ifndef _WINDOWS

#  define DEFAULTS_FILE_NAME		LIBDIR "defaults"
#  define PLAYER_PASSWORDS_FILE_NAME	LIBDIR "player_passwords"
#  define PASSWORD_FILE_NAME		LIBDIR "password"
#  define ROBOTFILE			LIBDIR "robots"
#  define SERVERMOTDFILE		LIBDIR "servermotd"
#  define LOCALMOTDFILE			LIBDIR "localmotd"
#  define LOGFILE			LIBDIR "log"
#  define SHIP_FILE       		""
#  define SOUNDFILE	  		SOUNDDIR "sounds"
#  ifdef DEBUG
#    define D(x)	x ;  fflush(stdout);
#  else
#    define D(x)
#  endif

#else /* _WINDOWS */

#  define DEFAULTS_FILE_NAME		LIBDIR "defaults.txt"
#  define PLAYER_PASSWORDS_FILE_NAME	LIBDIR "player_passwords.txt"
#  define PASSWORD_FILE_NAME		LIBDIR "password.txt"
#  define ROBOTFILE			LIBDIR "robots.txt"
#  define SERVERMOTDFILE		LIBDIR "servermotd.txt"
#  define LOCALMOTDFILE			LIBDIR "localmotd.txt"
#  define LOGFILE			LIBDIR "log.txt"
#  define SHIP_FILE			"XPilot.shp"
#  define SOUNDFILE			SOUNDDIR "sounds.txt"
#  ifdef _DEBUG
#    define DEBUG	1
#    define D(x)	x
#  else
#    define D(x)
#  endif

#endif /* _WINDOWS */

/* Please don't change this one. */
#define CONTACTADDRESS			"xpilot@xpilot.org"

/*
 * Uncomment this if your machine doesn't use
 * two's complement negative numbers.
 */
/* #define MOD2(x, m) mod(x, m) */

/*
 * The following macros decide the speed of the game and
 * how often the server should draw a frame.  (Hmm...)
 */

#define UPDATES_PR_FRAME	1

/*
 * If COMPRESSED_MAPS is defined, the server will attempt to uncompress
 * maps on the fly (but only if neccessary). ZCAT_FORMAT should produce
 * a command that will unpack the given .Z file to stdout (for use in popen).
 * ZCAT_EXT should define the proper compressed file extension.
 */

#ifndef _WINDOWS
#  define COMPRESSED_MAPS
#else
/* Couldn't find a popen(), also compress and gzip don't exist. */
#  undef COMPRESSED_MAPS
#endif

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
