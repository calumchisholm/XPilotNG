#include <signal.h>
#include "xpclient.h"

extern void Game_loop(void);
extern void Options_cleanup(void);

static void Main_shutdown(void)
{
    Net_cleanup();
    Client_cleanup();
    Options_cleanup();
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
    Connect_param_t conpar;
    char *cp;
    bool auto_shutdown = false;

    init_error(argv[0]);

    seedMT((unsigned)time(NULL) ^ Get_process_id());

    conpar.contact_port = SERVER_PORT;
    conpar.team = TEAM_NOT_SET;

    *hostname = 0;
    cp = getenv("XPILOTHOST");
    if (cp) strlcpy(hostname, cp, sizeof(hostname));
    else sock_get_local_hostname(hostname, sizeof hostname, 0);

    cp = getenv("XPILOTUSER");
    if (cp) strlcpy(conpar.real_name, cp, sizeof(conpar.real_name));
    else Get_login_name(conpar.real_name, sizeof(conpar.real_name) - 1);

    memset(&xpArgs, 0, sizeof(xp_args_t));
    Parse_options(&argc, argv, conpar.real_name,
		  &conpar.contact_port, &conpar.team,
		  conpar.nick_name, conpar.disp_name,
		  hostname);

    /* CLIENTRANK */
    Init_saved_scores();

    if (!Contact_servers(argc - 1, &argv[1],
			 xpArgs.auto_connect, xpArgs.list_servers,
			 auto_shutdown, xpArgs.shutdown_reason,
			 0, 0, 0, 0,
			 &conpar)) return 0;

    /* If something goes wrong before Client_setup I'll leave the
     * cleanup to the OS because afaik Client_cleanup will clean
     * stuff initialized in Client_setup. */

    if (Client_init(conpar.server_name, conpar.server_version)) {
	error("failed to initialize client"); 
	exit(1);
    }
    if (Net_init(conpar.server_addr, conpar.login_port)) {
	error("failed to initialize networking"); 
	exit(1);
    }
    if (Net_verify(conpar.real_name, 
		   conpar.nick_name, 
		   conpar.disp_name)) {
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
