#ifndef _MENU_H_
#define _MENU_H_
#include <gctypes.h>

#define THEMEMANAGER_VERSION		1

#define MAX_TEXTURES	            11
#define TEX_ARROWS		            0
#define TEX_BACKGROUND	            1
#define TEX_CONTAINER	            2
#define TEX_EMPTY		            3
#define TEX_LOADING		            4
#define TEX_NUMBERS		            5
#define TEX_MESSAGE_BUBBLE		    6
#define TEX_DISCLAIMER_BACKGROUND   7
#define TEX_NO_IMG                  8
#define TEX_NET_CONNECT             9
#define TEX_NO_NET_CONNECT          10

#define MENU_HOME			  -1
#define MENU_SHOW_THEME		  4
#define MENU_INSTALL_THEME    6
#define MENU_MANAGE_DEVICE	  2
#define MENU_SELECT_THEME     3
#define MENU_DOWNLOAD_IMAGE   5
#define MENU_ORIG_THEME       7
#define MENU_EXIT		      -10

#define MAXTHEMES		      400
#define KNOWN_SYSTEMMENU_VERSIONS   18


#define HOTSPOT_LEFT		MAXHOTSPOTS-2
#define HOTSPOT_RIGHT		MAXHOTSPOTS-1

// Constants
#define ARROWS_X		        25
#define ARROWS_Y		        250
#define ARROWS_WIDTH	        20 

#define EMPTY			-1

#define BLACK	0x000000FF
#define YELLOW	0xFFFF00FF
#define WHITE	0xFFFFFFFF
#define WHITE_SMOKE	0xF5F5F5FF
#define ORANGE	0xFF8000FF
#define RED     0xFF0000FF
#define GRAY    0x525252FF
#define GREEN   0x008000ff
#define TRANS_WHITE  0xFFFFFF50

#define ZCHUNK 16384


#include "video.h"


// themelist buffer & variables
typedef struct{
	char* title;
	char* downloads;
    MRCtex* banner;
	bool has_banner;
	u8 *region;
	u32 version;
	int type;
	int size;
	char *id;
	char *png;
	char *mym;
	
} ModTheme; 

typedef struct{
	u8 *region;
	u32 version;
}CurthemeStats;

void Menu_Loop();
const char *getdevicename(int);
const char *getappname(u32);
const char *getregion(u32);
const char *getsysvernum(u32);
int __downloadApp();
bool checkforpriiloader();
void __Draw_Loading(int, int);
void __Draw_Message(const char *, int, u32);
void logfile(const char *, ...);

#endif
