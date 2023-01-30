#ifndef _MENU_H_
#define _MENU_H_
#include <gctypes.h>


#define MENU_MANAGE_DEVICE  0
#define MENU_SELECT_THEME   1
#define MENU_SHOW_THEME	   2
#define MENU_Install_Theme  3
#define Menu_Start_Themeing 4
#define MENU_MAKE_THEME     5
#define MENU_ORIG_THEME     6
#define MENU_HOME		   7
#define MENU_EXIT		   8
#define MENU_EXIT_TO_MENU   9


#define MAXTHEMES		200

#define THEMEWII_VERSION		2.4
#define HOTSPOT_LEFT		MAXHOTSPOTS - 2
#define HOTSPOT_RIGHT		MAXHOTSPOTS - 1

#include "video.h"



typedef struct _dirent {
	char WorkingName[65];
	char DisplayName[65];
	int type;
	MRCtex* preview_banner1;
	MRCtex* preview_banner2;
	MRCtex* preview_banner3;
	MRCtex* preview_banner4;
	u8 *region;
	u32 version;
} dirent_t;

dirent_t *themelist;
typedef struct{
	u8 *region;
	u32 version;
}CurthemeStats;

CurthemeStats curthemestats;

u32 Current_System_Menu_Version;
int thememode;
u32 themecnt;

// Prototypes
int Menu_Loop(int);
#ifdef __cplusplus
extern "C" {
#endif
const char *getdevicename(int index);
char *getappname(u32 Versionsys);
char *getregion(u32 num);
void spinner();
char *getsysvernum(u32 num);
void __Draw_Message(const char* title, int ret);
int __downloadApp(int downloadonly);
int Undo_U8(char *Filepath, char* Filesavepath);
char *getdownloadregion(u32 num);
s32 getdir(char *, dirent_t **, u32 *);
#ifdef __cplusplus
}
#endif
#endif
