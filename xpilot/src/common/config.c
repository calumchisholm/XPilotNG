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

#include "xpcommon.h"

char config_version[] = VERSION;


char *Conf_libdir(void)
{
    static char conf[] = LIBDIR;

    return conf;
}

char *Conf_defaults_file_name(void)
{
    static char conf[] = DEFAULTS_FILE_NAME;

    return conf;
}

char *Conf_password_file_name(void)
{
    static char conf[] = PASSWORD_FILE_NAME;

    return conf;
}

char *Conf_player_passwords_file_name(void)
{
    static char conf[] = PLAYER_PASSWORDS_FILE_NAME;

    return conf;
}


char *Conf_mapdir(void)
{
    static char conf[] = MAPDIR;

    return conf;
}

static char conf_default_map_string[] = DEFAULT_MAP;

char *Conf_default_map(void)
{
    return conf_default_map_string;
}

char *Conf_servermotdfile(void)
{
    static char conf[] = SERVERMOTDFILE;
    static char env[] = "XPILOTSERVERMOTD";
    char *filename;

    filename = getenv(env);
    if (filename == NULL) {
	filename = conf;
    }

    return filename;
}

char *Conf_localmotdfile(void)
{
    static char conf[] = LOCALMOTDFILE;

    return conf;
}

char conf_logfile_string[] = LOGFILE;

char *Conf_logfile(void)
{
    return conf_logfile_string;
}

/* needed by client/default.c */
char conf_ship_file_string[] = SHIP_FILE;

char *Conf_ship_file(void)
{
    return conf_ship_file_string;
}

/* needed by client/default.c */
char conf_texturedir_string[] = TEXTUREDIR;

char *Conf_texturedir(void)
{
    return conf_texturedir_string;
}

/* needed by client/default.c */
char conf_soundfile_string[] = SOUNDFILE;

char *Conf_soundfile(void)
{
    return conf_soundfile_string;
}

char *Conf_localguru(void)
{
    static char conf[] = LOCALGURU;

    return conf;
}

char *Conf_contactaddress(void)
{
    static char conf[] = CONTACTADDRESS;

    return conf;
}

static char conf_robotfile_string[] = ROBOTFILE;

char *Conf_robotfile(void)
{
    return conf_robotfile_string;
}

char *Conf_zcat_ext(void)
{
    static char conf[] = ZCAT_EXT;

    return conf;
}

char *Conf_zcat_format(void)
{
    static char conf[] = ZCAT_FORMAT;

    return conf;
}

char *Conf_sounddir(void)
{
    static char conf[] = SOUNDDIR;

    return conf;
}

void Conf_print(void)
{
    xpprintf("============================================================\n");
    xpprintf("VERSION                   = %s\n", VERSION);
    xpprintf("PACKAGE                   = %s\n", PACKAGE);
    xpprintf("Conf_libdir()             = %s\n", Conf_libdir());
    xpprintf("Conf_defaults_file_name() = %s\n", Conf_defaults_file_name());
    xpprintf("Conf_password_file_name() = %s\n", Conf_password_file_name());
    xpprintf("Conf_player_passwords_file_name() = %s\n",
	     Conf_player_passwords_file_name());
    xpprintf("Conf_mapdir()             = %s\n", Conf_mapdir());
    xpprintf("Conf_default_map()        = %s\n", Conf_default_map());
    xpprintf("Conf_servermotdfile()     = %s\n", Conf_servermotdfile());
    xpprintf("Conf_localmotdfile()      = %s\n", Conf_localmotdfile());
    xpprintf("Conf_logfile()            = %s\n", Conf_logfile());
    xpprintf("Conf_ship_file()          = %s\n", Conf_ship_file());
    xpprintf("Conf_texturedir()         = %s\n", Conf_texturedir());
    xpprintf("Conf_soundfile()          = %s\n", Conf_soundfile());
    xpprintf("Conf_localguru()          = %s\n", Conf_localguru());
    xpprintf("Conf_contactaddress()     = %s\n", Conf_contactaddress());
    xpprintf("Conf_robotfile()          = %s\n", Conf_robotfile());
    xpprintf("Conf_zcat_ext()           = %s\n", Conf_zcat_ext());
    xpprintf("Conf_zcat_format()        = %s\n", Conf_zcat_format());
    xpprintf("Conf_soundir()            = %s\n", Conf_sounddir());
    xpprintf("============================================================\n");
}
