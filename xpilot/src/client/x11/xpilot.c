/* 
 * XPilotNG, an XPilot-like multiplayer space war game.
 *
 * Copyright (C) 1991-2001 by
 *
 *      Bj�rn Stabell        <bjoern@xpilot.org>
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "xpclient_x11.h"

char xpilot_version[] = VERSION;

char			**Argv;
int			Argc;


static void Check_client_versions(void);


static void printfile(const char *filename)
{
    FILE		*fp;
    int			c;

    if ((fp = fopen(filename, "r")) == NULL)
	return;

    while ((c = fgetc(fp)) != EOF)
	putchar(c);

    fclose(fp);
}

const char *Program_name(void)
{
    return "xpilot-ng-x11";
}

/*
 * Oh glorious main(), without thee we cannot exist.
 */
int main(int argc, char *argv[])
{
    int result, retval = 1;
    bool auto_shutdown = false;
    Connect_param_t *conpar = &connectParam;

    /*
     * --- Output copyright notice ---
     */
    printf("  " COPYRIGHT ".\n"
	   "  " TITLE " comes with ABSOLUTELY NO WARRANTY; "
	      "for details see the\n"
	   "  provided COPYING file.\n\n");
    if (strcmp(Conf_localguru(), PACKAGE_BUGREPORT))
	printf("  %s is responsible for the local installation.\n\n",
	       Conf_localguru());

    Conf_print();

    Argc = argc;
    Argv = argv;

    /*
     * --- Miscellaneous initialization ---
     */
    init_error(argv[0]);

    seedMT( (unsigned)time(NULL) ^ Get_process_id());

    Check_client_versions();

    memset(conpar, 0, sizeof(Connect_param_t));

    /*
     * --- Create global option array ---
     */
    Store_default_options();
    Store_X_options();
    Store_hud_options();
    Store_paintradar_options();
    Store_xpaint_options();
    Store_guimap_options();
    Store_guiobject_options();
    Store_talk_macro_options();
    Store_key_options();
    Store_record_options();
    Store_color_options();

    /*
     * --- Check commandline arguments and resource files ---
     */
    memset(&xpArgs, 0, sizeof(xp_args_t));
    Parse_options(&argc, argv);
    /*strcpy(clientname,connectParam.nick_name); */

    Config_init();
    IFNWINDOWS(Handle_X_options();)
    
    /* CLIENTRANK */
    Init_saved_scores();

    if (xpArgs.list_servers)
	xpArgs.auto_connect = true;

    if (xpArgs.shutdown_reason[0] != '\0') {
	auto_shutdown = true;
	xpArgs.auto_connect = true;
    }

    /*
     * --- Message of the Day ---
     */
    printfile(Conf_localmotdfile());

    if (xpArgs.text || xpArgs.auto_connect || argv[1] || is_this_windows()) {
	if (xpArgs.list_servers)
	    printf("LISTING AVAILABLE SERVERS:\n");

	result = Contact_servers(argc - 1, &argv[1],
				 xpArgs.auto_connect, xpArgs.list_servers,
				 auto_shutdown, xpArgs.shutdown_reason,
				 0, NULL, NULL, NULL, NULL,
				 conpar);
    }
    else {
	IFNWINDOWS(result = Welcome_screen(conpar));
    }

    if (result == 1)
	retval = Join(conpar);
    
    if (instruments.clientRanker)
	Print_saved_scores();

    return retval;
}


/*
 * Verify that all source files making up this program have been
 * compiled for the same version.  Too often bugs have been reported
 * for incorrectly compiled programs.
 */
extern char about_version[];
#ifdef SOUND
extern char audio_version[];
extern char caudio_version[];
#endif
extern char bitmaps_version[];
extern char checknames_version[];
extern char client_version[];
extern char clientcommand_version[];
extern char clientrank_version[];
extern char colors_version[];
extern char config_version[];
extern char configure_version[];
extern char datagram_version[];
extern char dbuff_version[];
extern char default_version[];
extern char error_version[];
extern char event_version[];
extern char gfx2d_version[];
extern char guimap_version[];
extern char guiobjects_version[];
extern char join_version[];
extern char math_version[];
extern char messages_version[];
extern char meta_version[];
extern char net_version[];
extern char netclient_version[];
extern char option_version[];
extern char paint_version[];
extern char paintdata_version[];
extern char painthud_version[];
extern char paintmap_version[];
extern char paintobjects_version[];
extern char paintradar_version[];
extern char portability_version[];
extern char query_version[];
extern char record_version[];
extern char shipshape_version[];
extern char socklib_version[];
extern char talk_version[];
extern char talkmacros_version[];
extern char textinterface_version[];
extern char welcome_version[];
extern char widget_version[];
extern char xdefault_version[];
extern char xevent_version[];
extern char xeventhandlers_version[];
extern char xinit_version[];
extern char xpaint_version[];


static void Check_client_versions(void)
{
#ifndef _WINDOWS	/* gotta put this back in before source released */
    static struct file_version {
	char		filename[16];
	char		*versionstr;
    } file_versions[] = {
	{ "about", about_version },
#ifdef SOUND
	{ "audio", audio_version },
	{ "caudio", caudio_version },
#endif
	{ "bitmaps", bitmaps_version },
	{ "checknames", checknames_version },
	{ "client", client_version },
	{ "clientcommand", clientcommand_version },
	{ "clientrank", clientrank_version },
	{ "colors", colors_version },
	{ "config", config_version },
	{ "configure", configure_version },
	{ "datagram", datagram_version },
	{ "dbuff", dbuff_version },
	{ "default", default_version },
	{ "error", error_version },
	{ "event", event_version },
	{ "gfx2d", gfx2d_version },
	{ "guimap", guimap_version },
	{ "guiobjects", guiobjects_version },
	{ "join", join_version },
	{ "math", math_version },
	{ "messages", messages_version },
	{ "meta", meta_version },
	{ "net", net_version },
	{ "netclient", netclient_version },
	{ "option", option_version },
	{ "paint", paint_version },
	{ "paintdata", paintdata_version },
	{ "painthud", painthud_version },
	{ "paintmap", paintmap_version },
	{ "paintobjects", paintobjects_version },
	{ "paintradar", paintradar_version },
	{ "portability", portability_version },
	{ "query", query_version },
	{ "record", record_version },
	{ "shipshape", shipshape_version },
	{ "socklib", socklib_version },
	{ "talk", talk_version },
	{ "talkmacros", talkmacros_version },
	{ "textinterface", textinterface_version },
	{ "welcome", welcome_version },
	{ "widget", widget_version },
	{ "xdefault", xdefault_version },
	{ "xevent", xevent_version },
	{ "xeventhandlers", xeventhandlers_version },
	{ "xinit", xinit_version },
	{ "xpaint", xpaint_version },
	{ "xpilot", xpilot_version },
    };
    int			i;
    int			oops = 0;

    for (i = 0; i < NELEM(file_versions); i++) {
	if (strcmp(VERSION, file_versions[i].versionstr)) {
	    oops++;
	    error("Source file %s.c (\"%s\") is not compiled "
		  "for the current version (\"%s\")!",
		  file_versions[i].filename,
		  file_versions[i].versionstr,
		  VERSION);
	}
    }
    if (oops) {
	error("%d version inconsistency errors, cannot continue.", oops);
	error("Please recompile this program properly.");
	exit(1);
    }
#endif
}
