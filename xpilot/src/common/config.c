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
    static char conf[] = CONF_LIBDIR;

    return conf;
}

char *Conf_defaults_file_name(void)
{
    static char conf[] = CONF_DEFAULTS_FILE_NAME;

    return conf;
}

char *Conf_password_file_name(void)
{
    static char conf[] = CONF_PASSWORD_FILE_NAME;

    return conf;
}

#if 0
char *Conf_player_passwords_file_name(void)
{
    static char conf[] = CONF_PLAYER_PASSWORDS_FILE_NAME;

    return conf;
}
#endif

char *Conf_mapdir(void)
{
    static char conf[] = CONF_MAPDIR;

    return conf;
}

char *Conf_fontdir(void)
{
    static char conf[] = CONF_FONTDIR;

    return conf;
}

char *Conf_default_map(void)
{
    static char conf[] = CONF_DEFAULT_MAP;

    return conf;
}

char *Conf_servermotdfile(void)
{
    static char conf[] = CONF_SERVERMOTDFILE;
    static char env[] = "XPILOTSERVERMOTD";
    char *filename;

    filename = getenv(env);
    if (filename == NULL)
	filename = conf;

    return filename;
}

char *Conf_localmotdfile(void)
{
    static char conf[] = CONF_LOCALMOTDFILE;

    return conf;
}

char conf_logfile_string[] = CONF_LOGFILE;

char *Conf_logfile(void)
{
    return conf_logfile_string;
}

char *Conf_ship_file(void)
{
    static char conf[] = CONF_SHIP_FILE;

    return conf;
}

char *Conf_texturedir(void)
{
    static char conf[] = CONF_TEXTUREDIR;

    return conf;
}

char *Conf_localguru(void)
{
    static char conf[] = CONF_LOCALGURU;

    return conf;
}

char *Conf_contactaddress(void)
{
    static char conf[] = CONF_CONTACTADDRESS;

    return conf;
}

char *Conf_robotfile(void)
{
    static char conf[] = CONF_ROBOTFILE;

    return conf;
}

char *Conf_zcat_ext(void)
{
    static char conf[] = CONF_ZCAT_EXT;

    return conf;
}

char *Conf_zcat_format(void)
{
    static char conf[] = CONF_ZCAT_FORMAT;

    return conf;
}

char *Conf_sounddir(void)
{
    static char conf[] = CONF_SOUNDDIR;

    return conf;
}

char *Conf_soundfile(void)
{
    static char conf[] = CONF_SOUNDFILE;

    return conf;
}


void Conf_print(void)
{
    xpprintf("============================================================\n");
    xpprintf("VERSION                   = %s\n", VERSION);
    xpprintf("PACKAGE                   = %s\n", PACKAGE);

#ifdef DBE
    xpprintf("DBE\n");
#endif
#ifdef MBX
    xpprintf("MBX\n");
#endif
#ifdef SERVER_SOUND
    xpprintf("SERVER_SOUND\n");
#endif
#ifdef PLOCKSERVER
    xpprintf("PLOCKSERVER\n");
#endif
#ifdef DEVELOPMENT
    xpprintf("DEVELOPMENT\n");
#endif

    xpprintf("Conf_localguru()          = %s\n", Conf_localguru());
    xpprintf("Conf_contactaddress()     = %s\n", Conf_contactaddress());
    xpprintf("Conf_libdir()             = %s\n", Conf_libdir());
    xpprintf("Conf_defaults_file_name() = %s\n", Conf_defaults_file_name());
    xpprintf("Conf_password_file_name() = %s\n", Conf_password_file_name());
    xpprintf("Conf_mapdir()             = %s\n", Conf_mapdir());
    xpprintf("Conf_default_map()        = %s\n", Conf_default_map());
    xpprintf("Conf_servermotdfile()     = %s\n", Conf_servermotdfile());
    xpprintf("Conf_robotfile()          = %s\n", Conf_robotfile());
    xpprintf("Conf_logfile()            = %s\n", Conf_logfile());
    xpprintf("Conf_localmotdfile()      = %s\n", Conf_localmotdfile());
    xpprintf("Conf_ship_file()          = %s\n", Conf_ship_file());
    xpprintf("Conf_texturedir()         = %s\n", Conf_texturedir());
    xpprintf("Conf_fontdir()            = %s\n", Conf_fontdir());
    xpprintf("Conf_sounddir()           = %s\n", Conf_sounddir());
    xpprintf("Conf_soundfile()          = %s\n", Conf_soundfile());
    xpprintf("Conf_zcat_ext()           = %s\n", Conf_zcat_ext());
    xpprintf("Conf_zcat_format()        = %s\n", Conf_zcat_format());
    xpprintf("============================================================\n");
}
