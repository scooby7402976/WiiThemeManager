#ifndef _MENU_H_
#define _MENU_H_
#include <gctypes.h>

#define MENU_SELECT_THEME   6
#define MENU_CONFIG        	1
#define MENU_SORT_GAMES		2
#define MENU_SHOW_THEME		3
#define MENU_INSTALL_THEME  4
#define MENU_HOME			5
#define MENU_MANAGE_DEVICE	0
#define MENU_DOWNLOAD		7
#define MENU_ORIG_THEME     8
#define MENU_EXIT			9
#define MENU_EXIT_TO_MENU   10

#define MAXTHEMES		250
#define KNOWN_SYSTEMMENU_VERSIONS   15

#define WIITHEMEMANAGER_VERSION		1
#define HOTSPOT_LEFT		MAXHOTSPOTS-2
#define HOTSPOT_RIGHT		MAXHOTSPOTS-1
#define debug                   1
#include "video.h"


// themelist buffer & variables
typedef struct{
	char* title;
    MRCtex* banner;
	u8 *region;
	u32 version;
	int type;
	char *downloadcount;
} ModTheme; //32 bytes!

//extern ModTheme ThemeList[MAXTHEMES];

typedef struct{
	u8 *region;
	u32 version;
}CurthemeStats;

//extern CurthemeStats curthemestats;

// Prototypes
int Menu_Loop();
#ifdef __cplusplus
extern "C" {
#endif
const char *getdevicename(int);
const char *getappname(u32);
const char *getregion(u32);
const char *getsysvernum(u32);

int __downloadApp(int);

bool checkforpriiloader();

void __Draw_Loading(void);
void __Draw_Message(const char *, int);
void logfile(const char *, ...);
#ifdef __cplusplus
}
#endif
#endif
