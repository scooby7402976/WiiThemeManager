#ifndef _MENU_H_
#define _MENU_H_
#include <gctypes.h>

#define WIITHEMEMANAGER_VERSION		1.0.0



#define MAX_TEXTURES	            7
#define TEX_ARROWS		            0
#define TEX_BACKGROUND	            1
#define TEX_CONTAINER	            2
#define TEX_EMPTY		            3
#define TEX_LOADING		            4
#define TEX_NUMBERS		            5
#define TEX_INFO		            6

#define MENU_HOME			  0
#define MENU_CONFIG        	  1
#define MENU_SORT_GAMES		  2
#define MENU_SHOW_THEME		  3
#define MENU_INSTALL_THEME    4
#define MENU_MANAGE_DEVICE	  5
#define MENU_SELECT_THEME     6
#define MENU_DOWNLOAD_IMAGE   7
#define MENU_ORIG_THEME       8
#define MENU_EXIT_HBC		  9
#define MENU_EXIT_SYSTEMMENU 10
#define MENU_EXIT_PRIILOADER 11

#define MAXTHEMES		250
#define KNOWN_SYSTEMMENU_VERSIONS   18


#define HOTSPOT_LEFT		MAXHOTSPOTS-2
#define HOTSPOT_RIGHT		MAXHOTSPOTS-1

// Constants
#define ARROWS_X		        16
#define ARROWS_Y		        210
#define ARROWS_WIDTH	        20 
#define WIITHEMEMANAGER_PATH		    "apps/wiithememanager"
#define IMAGES_PREFIX		    "imgs"
#define WIITHEMEMANAGER_CONFIG_FILE	"wiithememanager.cfg"

#define EMPTY			-1

#define BLACK	0x00000000
#define YELLOW	0xFFFF00FF
#define WHITE	0xFFFFFFFF
#define ORANGE	0xFF800000
#define RED     0xFF0000FF

#define TRANS_WHITE  0x00FFFFFF

#define ZCHUNK 16384


#include "video.h"


// themelist buffer & variables
typedef struct{
	const char* title;
	const char* titlenospace;
    MRCtex* banner;
	u8 *region;
	u32 version;
	int type;
	int downloadcount;
	int size;
} ModTheme; //32 bytes!

//extern ModTheme ThemeList[MAXTHEMES];

typedef struct{
	u8 *region;
	u32 version;
}CurthemeStats;

//extern CurthemeStats curthemestats;
static bool Sd_Mounted;
static bool Usb_Mounted;

void Menu_Loop();


const char *getdevicename(int);
const char *getappname(u32);
const char *getregion(u32);
const char *getsysvernum(u32);

int __downloadApp(int);

bool checkforpriiloader(bool);

void __Draw_Loading(void);
void __Draw_Message(const char *, int);
void logfile(const char *, ...);

#endif
