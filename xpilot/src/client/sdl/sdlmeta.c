#include "xpclient.h"
#include "sdlmeta.h"
#include "sdlwindow.h"
#include "text.h"
#include "glwidgets.h"


GLWidget *MetaWidget = NULL;

struct Label {
  const char *label;
  int commas;
  int yoff;
  int height;
};
    
struct Label labels[] = {
	{"Server", 0, 0, 0},
	{"IP:Port", 0, 0, 0},
	{"Version", 0, 0, 0},
	{"Users", 0, 0, 0},
	{"Map name", 0, 0, 0},
	{"Map size", 0, 0, 0},
	{"Map author", 0, 0, 0},
	{"Status", 0, 0, 0},
	{"Bases", 0, 0, 0},
	{"Teambases", 0, 0, 0},
	{"Free bases", 0, 0, 0},
	{"Queued players", 0, 0, 0},
	{"FPS", 0, 0, 0},
	{"Sound", 0, 0, 0},
	{"Timing", 0, 0, 0},
	{"Playlist", 1, 0, 0}
    };
/*
 * An array of structures with information to join a local server.
 */
static Connect_param_t *global_conpar;
static Connect_param_t *localnet_conpars;
static server_info_t *global_sip;

static const char player_header[] = "Pl";
static const char queue_header[] = "Q";
static const char bases_header[] = "Ba";
static const char team_header[] = "Tm";
static const char fps_header[] = "FPS";
static const char status_header[] = "Stat";
static const char version_header[] = "Version";
static const char map_header[] = "Map Name";
static const char server_header[] = "Server";
static const char ping_header[] = "Ping";
static const char stat_header[] = "Status";

void Meta_window(char *servername)
{
  static char err[MSG_LEN] = {0};
  int num_serv = 0;
  server_info_t *sip;

  if (!server_list ||
      List_size(server_list) < 10 ||
      server_list_creation_time + 5 < time(NULL)) {
    
    Delete_server_list();
    if ((num_serv = Get_meta_data(err)) <= 0) {
      fprintf(stderr, "Error: couldnt get meta list\n");
    } else {
      printf("xpilot_sdl: Got %d servers\n",num_serv);
    }
  }

  if (Welcome_sort_server_list() == -1) {
    Delete_server_list();
    error("Ran out of memory...");
  }
  server_it = List_begin(server_list);
  for (; server_it != List_end(server_list); LI_FORWARD(server_it)) {
    sip = SI_DATA(server_it);
    printf("%2d",sip->users);
    printf(" %16s:%6d", sip->ip_str, sip->port);
    printf(" %30s\n",sip->hostname);
  }

}
