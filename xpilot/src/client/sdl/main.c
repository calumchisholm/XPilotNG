#include <signal.h>
#include "xpclient.h"

extern void Game_loop(void);
extern void Options_cleanup(void);
extern void Store_sdlinit_options(void);
extern void Store_sdlgui_options(void);

#ifdef _WINDOWS
/* from win32hacks.c */
extern int Winsock_init();
extern void Winsock_cleanup();
#endif

static void Main_shutdown(void)
{
    Net_cleanup();
    Client_cleanup();
#ifdef _WINDOWS
	Winsock_cleanup();
#endif
}

static void sigcatch(int signum)
{
    signal(SIGINT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
    Main_shutdown();
    error("got signal %d\n", signum);
    exit(1);
}

int main(int argc, char *argv[])
{
    bool auto_shutdown = false;

    init_error(argv[0]);

    seedMT((unsigned)time(NULL) ^ Get_process_id());

    memset(&connectParam, 0, sizeof(Connect_param_t));
    connectParam.contact_port = SERVER_PORT;
    connectParam.team = TEAM_NOT_SET;

    Store_default_options();
    Store_talk_macro_options();
    Store_key_options();
    Store_sdlinit_options();
    Store_sdlgui_options();

    memset(&xpArgs, 0, sizeof(xp_args_t));
    Parse_options(&argc, argv);

    /* CLIENTRANK */
    Init_saved_scores();

#ifdef _WINDOWS
	if (Winsock_init()) {
		error("failed to initialize windows networking");
		exit(1);
	}
#endif

    if (!Contact_servers(argc - 1, &argv[1],
			 xpArgs.auto_connect, xpArgs.list_servers,
			 auto_shutdown, xpArgs.shutdown_reason,
			 0, NULL, NULL, NULL, NULL,
			 &connectParam)) return 0;

    /* If something goes wrong before Client_setup I'll leave the
     * cleanup to the OS because afaik Client_cleanup will clean
     * stuff initialized in Client_setup. */

    if (Client_init(connectParam.server_name, connectParam.server_version)) {
	error("failed to initialize client"); 
	exit(1);
    }

    if (Net_init(connectParam.server_addr, connectParam.login_port)) {
	error("failed to initialize networking"); 
	exit(1);
    }
    if (Net_verify(connectParam.user_name, 
		   connectParam.nick_name, 
		   connectParam.disp_name)) {
	error("failed to verify networking"); 
	exit(1);
    }
    if (Net_setup()) {
	error("failed to setup networking"); 
	exit(1);
    }
    if (Client_setup()) {
	error("failed to setup client"); 
	exit(1);
    }

    signal(SIGINT, sigcatch);
    signal(SIGTERM, sigcatch);

    if (Net_start()) {
	Main_shutdown();
	error("failed to start networking"); 
	exit(1);
    }
    if (Client_start()) {
	Main_shutdown();
	error("failed to start client"); 
	exit(1);
    }
    Game_loop();
    Main_shutdown();
    return 0;
}
