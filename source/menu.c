/* menu.c
 *
 * wiithememanager - Wii theme installer/downloader based on the gui of mighty channels(Marc) by Scooby74029 Copyright (c) 2024 Scooby74029
 *
 * Triiforce(Mighty Channels) mod by Marc
 *
 * Copyright (c) 2009 The Lemon Man, Nicksasa & WiiPower
 *
 * Distributed under the terms of the GNU General Public License (v2)
 * See http://www.gnu.org/licenses/gpl-2.0.txt for more info.
 *
 *********************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gccore.h>
#include <ogc/lwp_watchdog.h>
#include <wiiuse/wpad.h>
#include <asndlib.h>
#include <network.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <malloc.h>
#include <ogcsys.h>
#include <dirent.h>
#include <unistd.h>
#include <gctypes.h>
#include <stdarg.h>
#include <assert.h>
#include <zip/unzip.h>
#include <zip/ioapi.h>

#include "menu.h"
#include "tools.h"
#include "config.h"
#include "fat_mine.h"
#include "video.h"
#include "http.h"
#include "wpad.h"

#include "rijndael.h"
#include "sys.h"
#include "themedatabase.h"
#include "http.h"
#include "zlib.h"
#include "wiithememanager_arrows_png.h"
#include "wiithememanager_background_png.h"
#include "wiithememanager_container_png.h"
#include "wiithememanager_container_wide_png.h"
#include "wiithememanager_empty_png.h"
#include "wiithememanager_loading_png.h"
#include "wiithememanager_numbers_png.h"
#include "wiithememanager_qmark_png.h"

static bool wideScreen = false;
static MRCtex* textures[MAX_TEXTURES];
int debugcard = 1;
bool priiloadercheck = false;
u32 systemmenuversion = 0;

static s16* orden;
static int spinselected = -1;
int thememode = -1;
static u32 themecnt = 0;
u8 commonkey[16] = { 0xeb, 0xe4, 0x2a, 0x22, 0x5e, 0x85, 0x93, 0xe4, 0x48,
    0xd9, 0xc5, 0x45, 0x73, 0x81, 0xaa, 0xf7
};
static u16 page, maxPages = 1;
static int selectedtheme = 0, movingGame = -1;
static int loadingAnim = 0;

static bool saveconfig = false;

static bool pageLoaded[50];
static char tempString[256];
ModTheme ThemeList[MAXTHEMES];

bool foundneek;
CurthemeStats curthemestats;
dirent_t *ent = NULL;
dirent_t *nandfilelist = NULL;
extern GXRModeObj *vmode;
extern u32* framebuffer;
bool needloading = false;
bool downloadable_theme_List = true;
bool netconnection = false;
bool priiloaderackknowledgement = false;
u32 known_Versions[KNOWN_SYSTEMMENU_VERSIONS] = {416, 417, 418, 448, 449, 450, 454, 480, 481, 482, 486, 512, 513, 514, 518, 608, 609, 610};
char *regions[KNOWN_SYSTEMMENU_VERSIONS] = {"J", "U", "E", "J", "U", "E", "K", "J", "U", "E", "K", "J", "U", "E", "K", "J", "U", "E"};
char *knownappfilenames[KNOWN_SYSTEMMENU_VERSIONS] = {"0000006f.app", "00000072.app", "00000075.app", "00000078.app", "0000007b.app", "0000007e.app", "00000081.app", "00000084.app", "00000087.app", "0000008a.app", "0000008d.app", "00000094.app", "00000097.app", "0000009a.app", "0000009d.app", "0000001c.app", "0000001f.app", "00000022.app"};
char *appfilename[2] = { "Cetk", "Tmd" };
char *updatedownloadcount = NULL;
const u8 COLS[]={3, 4};
#define ROWS 3
const u8 FIRSTCOL[]={136, 110}; // 136 112
#define FIRSTROW 110
const u8 SEPARACIONX[]={180, 136};
#define SEPARACIONY 125
const u8 ANCHOIMAGEN[]={154, 116}; //116
#define ALTOIMAGEN 90
const char *themedir = "themes";

static bool Sd_Mounted = false;
static bool Usb_Mounted = false;

#define ALIGN32(x) (((x) + 31) & ~31)

int retrieve_themefilesize();
int retrieve_downloadcount();
const char *getregion(u32 num);
const char *getsysvernum(u32 num);
const char *getdevicename(int index);
int __Spin_Question(void);
void __Select_Device(void);
void __Load_Config(void);

bool __Check_HBC(void) {
	u32 *stub = (u32 *)0x80001800;

	// Check HBC stub
	if (*stub)
		return true;
	return false;
}
bool checkNinit_netconnection() {
	int ret = net_init();
	__Draw_Loading();
	if(ret == 0) return 1;
	return 0;
}
bool is_vWii(u32 version) {
	bool version_is_vWii = false;
	
	switch(version) {
		case 608:
		case 609:
		case 610:
			version_is_vWii = true;
			break;
	}
	return version_is_vWii;
}
bool checkforpriiloader(bool is_vWii) {
	dirent_t *priiloaderfiles = NULL;
	u32 nandfilecnt;
	int filecntr;
	char *searchstr;
	//logfile("is_vWii [%i]\n", is_vWii);
	searchstr = "title_or.tmd";
	getdir("/title/00000001/00000002/content",&priiloaderfiles,&nandfilecnt);
	for(filecntr = 0; filecntr < nandfilecnt; filecntr++) {
		if(strcmp(priiloaderfiles[filecntr].name, searchstr) == 0)
		return true;
	}
	return false; 
}
void logfile(const char *format, ...) {
	
	char buffer[256];
	char path[256];
	va_list args;
	va_start (args, format);
	vsprintf (buffer,format, args);
	FILE *f = NULL;
	if(!Sd_Mounted)
		Fat_Mount(SD);
	
	sprintf(path, "%s:/wiithememanager.log", getdevicename(SD));
	f = fopen(path, "a");
	if (!f) {
		printf("Error writing log\n");
		return;
	}
	fputs(buffer, f);
	fclose(f);
	va_end (args);
	if(Sd_Mounted)
		Fat_Unmount(SD);
	
	
	return;
}
void __Draw_Loading(void) {
 	MRC_Draw_Tile(300, 425, textures[TEX_LOADING], 24, loadingAnim);
	MRC_Render_Box(300, 425);

	loadingAnim += 1;
	if(loadingAnim == 16)
		loadingAnim = 0;
}

void __Draw_Page(int selected) {
	int i, j, x, y, containerWidth, theme;

	containerWidth=textures[TEX_CONTAINER]->width/2;

	// Background
	MRC_Draw_Texture(0, 0, textures[TEX_BACKGROUND]);
	sprintf(tempString, "IOS %d v%d", IOS_GetVersion(), IOS_GetRevision());
	MRC_Draw_Box(29, 429, strlen(tempString)*8 + 1, 17, WHITE);
	MRC_Draw_String(30, 430, BLACK, tempString);
	// add fix for custom theme version dewtection here ex 65535
	sprintf(tempString, "System Menu v%s%s", getsysvernum(systemmenuversion), getregion(systemmenuversion));
	MRC_Draw_Box(459, 429, strlen(tempString)*8 + 2, 17, WHITE);
	MRC_Draw_String(460, 430, BLACK, tempString);
	
	if(themecnt == 0 || !pageLoaded[page]){
		return;
	}

	// themes
	theme = COLS[wideScreen]*ROWS*page;
	y = FIRSTROW;
	for(i = 0; i < ROWS; i++){
		x = FIRSTCOL[wideScreen];
		for(j = 0; j < COLS[wideScreen]; j++){
			if(movingGame != theme){
				if(orden[theme] == EMPTY){
					MRC_Draw_Texture(x, y, textures[TEX_EMPTY]);
					MRC_Draw_Tile(x, y, textures[TEX_CONTAINER], containerWidth, 0);
				}else if(selected == i*COLS[wideScreen]+j){
					MRC_Draw_Texture(x, y, ThemeList[orden[theme]].banner);
					MRC_Draw_Tile(x, y, textures[TEX_CONTAINER], containerWidth, 1);
					sprintf(tempString, "%s", ThemeList[orden[theme]].title);
					MRC_Draw_Box(x - (containerWidth/2 + 1), y + 49, strlen(tempString)*8 + 1, 17, WHITE);
					MRC_Draw_String(x - containerWidth/2, y + 50, BLACK, tempString);
					
				}else{
					MRC_Draw_Texture(x, y, ThemeList[orden[theme]].banner);
					MRC_Draw_Tile(x, y, textures[TEX_CONTAINER], containerWidth, 0);
				}
			}
			theme++;
			x  += SEPARACIONX[wideScreen];
		}
		y += SEPARACIONY;
	}

	// Page number
	sprintf(tempString, "%d of %d", page + 1, maxPages);
	MRC_Draw_Box(289, 429, strlen(tempString)*8 + 1, 17, WHITE);
	MRC_Draw_String(290, 430, BLACK, tempString);

	if(movingGame > -1){
		if(orden[movingGame]==EMPTY){
			MRC_Draw_Texture(Wpad_GetWiimoteX(), Wpad_GetWiimoteY(), textures[TEX_EMPTY]);
		}else{
			MRC_Draw_Texture(Wpad_GetWiimoteX(), Wpad_GetWiimoteY(), ThemeList[orden[movingGame]].banner);
		}
	}

	// Arrows
	MRC_Draw_Tile(ARROWS_X, ARROWS_Y, textures[TEX_ARROWS], ARROWS_WIDTH, 0+(page>=0)+(page>=0 && selected==HOTSPOT_LEFT));
	MRC_Draw_Tile(640-ARROWS_X, ARROWS_Y, textures[TEX_ARROWS], ARROWS_WIDTH, 3+(page+1<maxPages)+(page+1<maxPages && selected==HOTSPOT_RIGHT));
}

void __Draw_Button(int hot, const char* text, bool selected) {
	hotSpot button = Wpad_GetHotSpotInfo(hot);
	int textX = button.x+(button.width-strlen(text)*8)/2;
	u32 color;
	if(selected){
		MRC_Draw_Box(button.x, button.y, button.width, button.height/2, 0x3c7291ff);
		MRC_Draw_Box(button.x, button.y+button.height/2, button.width, button.height/2, 0x2d6483ff);
		color = WHITE;
	}else{
		MRC_Draw_Box(button.x, button.y, button.width, button.height/2, 0xe3e3e3ff);
		MRC_Draw_Box(button.x, button.y+button.height/2, button.width, button.height/2, 0xd8d8d8ff);
		MRC_Draw_Box(button.x, button.y, button.width, 1, 0xb6b4c5ff);
		MRC_Draw_Box(button.x, button.y+button.height-1, button.width, 1, 0xb6b4c5ff);
		MRC_Draw_Box(button.x, button.y, 1, button.height, 0xb6b4c5ff);
		MRC_Draw_Box(button.x+button.width-1, button.y, 1, button.height, 0xb6b4c5ff);
		color = 0x404040ff;
	}
	MRC_Draw_String(textX, button.y+button.height/2-8, color, text);
}

void __Draw_Window(int width, int height, const char* title) {
	int x=(640-width)/2;
	int y=(480-height)/2-32;
	__Draw_Page(-1);
	char mode[32];// = ;
	//char *imode = ;
	//MRC_Draw_Box(0, 0, 640, 480, BLACK);
	MRC_Draw_Texture(0, 0, textures[TEX_BACKGROUND]);
	sprintf(tempString, "IOS %d v%d", IOS_GetVersion(), IOS_GetRevision());
	MRC_Draw_Box(29, 429, strlen(tempString)*8 + 1, 17, WHITE);
	MRC_Draw_String(30, 430, BLACK, tempString);
	if(downloadable_theme_List) {
		sprintf(mode, "%s", "Mode Downloader");
		MRC_Draw_Box(259, 429, strlen(tempString)*10 + 2, 17, WHITE);
		MRC_Draw_String(260, 430, BLACK, mode);
	}
	else {
		sprintf(mode, "%s", "Mode Installer");
		MRC_Draw_Box(259, 429, strlen(tempString)*10 - 1, 17, WHITE);
		MRC_Draw_String(260, 430, BLACK, mode);
	}
	sprintf(tempString, "System Menu v%s%s", getsysvernum(systemmenuversion), getregion(systemmenuversion));
	MRC_Draw_Box(459, 429, strlen(tempString)*8 + 2, 17, WHITE);
	MRC_Draw_String(460, 430, BLACK, tempString);
	
	MRC_Draw_Box(x, y, width, 32, YELLOW);
	MRC_Draw_Box(x, y+32, width, height, WHITE);

	MRC_Draw_String(x+(width-strlen(title)*8)/2, y+8, BLACK, title);
}

void __Draw_Message(const char* title, int ret) {
	int i;

	MRC_Draw_Texture(0, 0, textures[TEX_BACKGROUND]);
	sprintf(tempString, "IOS %d v%d", IOS_GetVersion(), IOS_GetRevision());
	MRC_Draw_Box(29, 429, strlen(tempString)*8 + 1, 17, WHITE);
	MRC_Draw_String(30, 430, BLACK, tempString);
	
	sprintf(tempString, "System Menu v%s%s", getsysvernum(systemmenuversion), getregion(systemmenuversion));
	MRC_Draw_Box(459, 429, strlen(tempString)*8 + 2, 17, WHITE);
	MRC_Draw_String(460, 430, BLACK, tempString);
	MRC_Draw_Box(20, 150, 600, 48, WHITE);
	for(i = 0;i < 16; i++){
		MRC_Draw_Box(20, 200-16+i, 600, 1, i*2);
		MRC_Draw_Box(20, 200+48+16-i, 600, 1, i*2);
	}
	MRC_Draw_String((640-strlen(title)*8)/2, 165, BLACK, title);

	if(ret < 0){
		sprintf(tempString, "ret=%d", ret);
		MRC_Draw_String(540, 216, 0xff0000ff, tempString);
	}

	MRC_Render_Screen();
	
	if(ret != 0)
	Wpad_WaitButtons();
}
#define QUESTION_BUTTON_X			90
#define QUESTION_BUTTON_Y			240
#define QUESTION_BUTTON_SEPARATION	30
#define QUESTION_BUTTON_WIDTH		175
#define QUESTION_BUTTON_HEIGHT		30
int __Question_Window(const char* title, const char* text, const char* a1, const char* a2) {
	int i, hotSpot, hotSpotPrev;
	int ret=0, repaint=true;

	// Create/restore hotspots
	Wpad_CleanHotSpots();
	for(i=0; i<2; i++)
		Wpad_AddHotSpot(i,
			QUESTION_BUTTON_X+i*(QUESTION_BUTTON_WIDTH+QUESTION_BUTTON_SEPARATION),
			QUESTION_BUTTON_Y,
			QUESTION_BUTTON_WIDTH,
			QUESTION_BUTTON_HEIGHT,
			(i == 0? 1 : i - 1),
			(i == 2? 0 : i + 1),
			i, i
		);


	__Draw_Window(552, 128, title);
	MRC_Draw_String(100, 200, BLACK, text);

	// Loop
	hotSpot=hotSpotPrev=-1;

	
	for(;;){
		hotSpot = Wpad_Scan();

		// If hot spot changed
		if((hotSpot!=hotSpotPrev && hotSpot<2) || repaint){
			hotSpotPrev = hotSpot;

			__Draw_Button(0, a1, hotSpot==0);
			__Draw_Button(1, a2, hotSpot==1);
			repaint=false;
		}
		MRC_Draw_Cursor(Wpad_GetWiimoteX(), Wpad_GetWiimoteY(), 0);

		if(((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_A) || (PAD_ButtonsDown(0) & PAD_BUTTON_A)) && hotSpot!=-1){
			if(hotSpot==0)
				ret=1;
			if(hotSpot==1)
				ret=0;
			break;
		}
	}
	if(debugcard) logfile("ret question [%i]\n", ret);
	return ret;
}


void findnumpages(void) {
	int i;
	maxPages = 0;
	for(i = MAXTHEMES - 1; i > -1; i--){
		if(orden[i] != EMPTY){
			break;
		}
	}

	while(maxPages*COLS[wideScreen]*ROWS<=i){
		maxPages++;
	}

	if(movingGame!=-1)
		maxPages++;

	if(maxPages > 50)
		maxPages = 50;
}
/*void __Save_Changes(void){
	int i, j, configsize;
	unsigned char *outBuffer;

	configsize=1+15+themecnt*(2+4+7+3); //1=cfgversion + 15=reserved   +   (2=position   4=id   7=config   3=reserved)=16
	//#ifdef DEBUG_MODE
	gprintf("Saving changes... (%d bytes)\n", configsize);
	//#endif
	outBuffer=allocate_memory(configsize);

	for(i=0; i<16; i++)
		outBuffer[i]=0;
	outBuffer[0]=wiithememanager_VERSION;

	j=16;
	for(i=0; i<themecnt; i++){
		__Draw_Loading();
		if(orden[i]!=EMPTY){
			outBuffer[j]=i%256;
			outBuffer[j+1]=i/256;
			outBuffer[j+2]=ThemeList[orden[i]].id[0];
			outBuffer[j+3]=ThemeList[orden[i]].id[1];
			outBuffer[j+4]=ThemeList[orden[i]].id[2];
			outBuffer[j+5]=ThemeList[orden[i]].id[3];

			outBuffer[j+6]=ThemeList[orden[i]].videoMode;
			outBuffer[j+7]=ThemeList[orden[i]].videoPatch;
			outBuffer[j+8]=ThemeList[orden[i]].language;
			outBuffer[j+9]=ThemeList[orden[i]].hooktype;
			outBuffer[j+10]=ThemeList[orden[i]].ocarina;
			outBuffer[j+11]=ThemeList[orden[i]].debugger;
			outBuffer[j+12]=ThemeList[orden[i]].bootMethod;
			outBuffer[j+13]=0;
			outBuffer[j+14]=0;
			outBuffer[j+15]=0;
			j+=16;
		}
	}

	// Save changes to FAT
	Fat_SaveFile(wiithememanager_PATH WIITHEMEMANAGER_CONFIG_FILE, (void *)&outBuffer, configsize);

	saveconfig=false;
}*/




/* Constants */
#define MAX_FILELIST_LEN	1024
#define MAX_FILEPATH_LEN	256
#define NEEK                4
s32 __themeCmp(const void *a, const void *b){
	ModTheme *hdr1 = (ModTheme *)a;
	ModTheme *hdr2 = (ModTheme *)b;
	
	if (hdr1->type == hdr2->type){
		return strcmp(hdr1->title, hdr2->title);
	}else{
		return 0;
	}
}
const char *getappname(u32 systemmenuversion) {
	switch(systemmenuversion){
		case 417: return "00000072.app";// usa
		break;
		case 449: return "0000007b.app";
		break;
		case 481: return "00000087.app";
		break;
		case 513: return "00000097.app";// usa
		break;
		case 418: return "00000075.app";// pal
		break;
		case 450: return "0000007e.app";
		break;
		case 482: return "0000008a.app";
		break;
		case 514: return "0000009a.app";// pal
		break;
		case 416: return "00000070.app";// jpn
		break;
		case 448: return "00000078.app";
		break;
		case 480: return "00000084.app";
		break;
		case 512: return "00000094.app";// jpn
		break;
		case 486: return "0000008d.app";// kor
		break;
		case 454: return "00000081.app";
		break;
		case 518: return "0000009d.app";// kor
		break;
		default: return "UNKNOWN";
		break;
	}
}
u32 checkcustomsystemmenuversion() {
	u32 nandfilecnt = 0, filecounter = 0, knownversioncounter = 0;
	char *knownversionstr = NULL;
	
	getdir("/title/00000001/00000002/content",&nandfilelist,&nandfilecnt);
	for(filecounter = 0; filecounter < nandfilecnt; filecounter++) {
		for(knownversioncounter = 0; knownversioncounter < KNOWN_SYSTEMMENU_VERSIONS; knownversioncounter++) {
			knownversionstr = knownappfilenames[knownversioncounter];
			if(strcmp(nandfilelist[filecounter].name, knownversionstr) == 0) return known_Versions[knownversioncounter];
		}
	}
	return 0;
}	
bool retreivecurrentthemeregion(u32 inputversion) {
	//gprintf("check nandapp():\n");
	switch(inputversion)
	{
		case 416:
		case 448:
		case 480:
		case 512:
			curthemestats.region = (u8*)74;
		break;
		case 417:
		case 449:
		case 481:
		case 513:
			curthemestats.region = (u8*)85;
		break;
		case 418:
		case 450:
		case 482:
		case 514:
			curthemestats.region = (u8*)69;
		break;
		case 454:
		case 486:
		case 518:
			curthemestats.region = (u8*)75;
		break;
		default:
			curthemestats.region = 0;
			return 0;
		break;
	}
	//gprintf("cur theme .region(%c)  .version(%d) \n",curthemestats.region,curthemestats.version);
	
	return 1;
}
const char *getregion(u32 num) {
    switch(num)
    {
    case 417:
    case 449:
    case 481:
    case 513:
        return "U";
        break;
    case 418:
    case 450:
    case 482:
    case 514:
        return "E";
        break;
    case 416:
    case 448:
    case 480:
    case 512:
        return "J";
        break;
    case 486:
    case 454:
    case 518:
        return "K";
        break;
    default:
        return "UNKNOWN";
        break;
    }
}
const char *getsysvernum(u32 num) {
    switch(num)
    {
    case 416:
    case 417:
    case 418:
        return "4.0";
        break;
    case 448:
    case 449:
    case 450:
	case 454:
        return "4.1";
        break;
    case 480:
    case 481:
    case 482:
	case 486:
        return "4.2";
        break;
    case 512:
    case 513:
    case 514:
	case 518:
        return "4.3";
        break;
    default:
        return "UNKNOWN";
        break;
    }
}
u32 GetSysMenuVersion() {
    //Get sysversion from TMD
    u64 TitleID = 0x0000000100000002LL;
    u32 tmd_size;
    s32 r = ES_GetTMDViewSize(TitleID, &tmd_size);
    if(r<0)
    {
        //gprintf("error getting TMD views Size. error %d\n",r);
        return 0;
    }

    tmd_view *rTMD = (tmd_view*)memalign( 32, (tmd_size+31)&(~31) );
    if( rTMD == NULL )
    {
        //gprintf("error making memory for tmd views\n");
        return 0;
    }
    memset(rTMD,0, (tmd_size+31)&(~31) );
    r = ES_GetTMDView(TitleID, (u8*)rTMD, tmd_size);
    if(r<0)
    {
        //gprintf("error getting TMD views. error %d\n",r);
        free( rTMD );
        return 0;
    }
    u32 version = rTMD->title_version;
    if(rTMD)
    {
        free(rTMD);
    }
    return version;
}
u32 filelist_retrieve(bool downloadable_theme_list) {
	//static char filelist[MAX_FILELIST_LEN];
    char dirpath[MAX_FILEPATH_LEN];
    //char dirpath2[ISFS_MAXPATH + 1];
    //char *ptr = filelist;
    //struct stat filestat;
    
    //s32 start = 0;
	u32 fu, ff, cnt = 0;
	struct dirent *entry = NULL;
	needloading = true;
	//dirent_t *neeklist = NULL;
	//u32 neekcount;
	if(downloadable_theme_list) {
		for(fu = 0; fu < MAXTHEMES; fu++){
			if(DBThemelist[fu] == NULL)
				break;
			cnt += 1;
		}
		for(ff = 0; ff < cnt; ff++){
			if(needloading) __Draw_Loading();
			ThemeList[ff].title = DBThemelist[ff];
			ThemeList[ff].titlenospace = DBThemelist_nospaces[ff];
			ThemeList[ff].type = 10;
			ThemeList[ff].size = 0;
			ThemeList[ff].version = 0;
			ThemeList[ff].region = NULL;
			ThemeList[ff].downloadcount = -1;
			//gprintf("theme =%s .type%d %d \n",ThemeList[ff].title, ThemeList[ff].type,ff);
		}
		
		//goto end;
    }
	else {
		//Generate dirpath 

		DIR *dir;
		sprintf(dirpath, "%s:/themes", getdevicename(thememode));
		if(debugcard) logfile("dirpath[%s]\n", dirpath);
		
		
		/* Open directory */

		dir = opendir(dirpath);
		themedir = "themes";
		if (!dir)
		{
			sprintf(dirpath, "%s:/modthemes", getdevicename(thememode));
			dir = opendir(dirpath);
			themedir = "modthemes";
			if (!dir) {
				if(debugcard) logfile("Unable to open %s \n", getdevicename(thememode));
				return -1;
			}
			
		}
		cnt = 0;
		//if(debugcard) logfile("cnt[%u]\n", cnt);
		// Get directory entries 
		while((entry = readdir(dir))) // If we get EOF, the expression is 0 and
										 // the loop stops. 
		{
			if(strncmp(entry->d_name, ".", 1) != 0 && strncmp(entry->d_name, "..", 2) != 0)
			cnt += 1;
		}
		//if(debugcard) logfile("2-cnt2[%u]\n", cnt2);
		rewinddir(dir);
		ent = allocate_memory(sizeof(dirent_t) * cnt);
		cnt = 0;
		//if(debugcard) logfile("3-cnt2[%u]\n", cnt2);
		while((entry = readdir(dir))) // If we get EOF, the expression is 0 and
										 // the loop stops. 
		{
			if(needloading) __Draw_Loading();
			if(strncmp(entry->d_name, ".", 1) != 0 && strncmp(entry->d_name, "..", 2) != 0){
				strcpy(ent[cnt].name, entry->d_name);//, sizeof(entry->d_name));
				ThemeList[cnt].title = ent[cnt].name;
				ThemeList[cnt].type = 20;
				//gprintf("theme =%s .type%d \n",ThemeList[themecnt].title, ThemeList[themecnt].type);
				cnt += 1;
				
			}
		}
		qsort(ThemeList, cnt, sizeof(ModTheme), __themeCmp);
		closedir(dir);
	}
	
	needloading = false;
	if(debugcard) logfile("cnt[%u]\n", cnt);
    return cnt;
}


void __Load_Config(void) {
	int ret, i, j, k;
	s16* posiciones=allocate_memory(sizeof(u16)*themecnt);

	orden=allocate_memory(sizeof(u16)*MAXTHEMES);

	for(i=0; i<themecnt; i++)
		posiciones[i]=-1;

	
	// Ordenar los juegos segun archivo de configuracion
	char *archivoLeido=NULL;

	ret = Fat_ReadFile(WIITHEMEMANAGER_PATH WIITHEMEMANAGER_CONFIG_FILE, (void *)&archivoLeido, 0);

	if(ret>0){
		// Parse config file
		i=16;
		while(i<ret){
			j=archivoLeido[i]+archivoLeido[i+1]*256;

			for(k=0; k<4; k++)
				tempString[k]=archivoLeido[i+2+k];
			tempString[4]='\0';

			for(k=0; k<themecnt; k++){
				/*if(strcmp(tempString, ThemeList[k].id)==0){
					//printf("%s found at %d\n", tempString, k);
					ThemeList[k].videoMode=archivoLeido[i+6];
					ThemeList[k].videoPatch=archivoLeido[i+7];
					ThemeList[k].language=archivoLeido[i+8];
					ThemeList[k].hooktype=archivoLeido[i+9];
					ThemeList[k].ocarina=archivoLeido[i+10];
					ThemeList[k].debugger=archivoLeido[i+11];
					ThemeList[k].bootMethod=archivoLeido[i+12];
					//ThemeList[k].ios=archivoLeido[i+13];

					posiciones[k]=j;
					break;
				}*/
			}
			i+=16;
		}
		free(archivoLeido);
	}

	
	//Initialize empty
	for(i=0; i<MAXTHEMES; i++)
		orden[i]=EMPTY;

	//Place games
	for(i=0; i<themecnt; i++)
		if(posiciones[i]!=-1)
			orden[posiciones[i]]=i;

	//Check if some game was not placed
	for(i=0; i<themecnt; i++){
		if(posiciones[i]==-1){
			for(j=0; j<MAXTHEMES; j++){
				if(orden[j]==EMPTY){
					posiciones[i]=j;
					break;
				}
			}
		}
		orden[posiciones[i]]=i;
	}
	free(posiciones);

	selectedtheme=0;
	page=0;
	findnumpages();
}

/*
void __Free_Channel_Images(void) {
	int i;
	int imagesPerScreen = COLS[wideScreen]*ROWS;
	
	for(i=0; i<MAXTHEMES; i++){
		//printf("%d: %d\n", i, orden[i]);

		if(i==pages*COLS[wideScreen]*ROWS)
			break;
		if(orden[i]!=EMPTY){
			MRC_Free_Texture(ThemeList[orden[i]].banner);
		}
	}

	pages=0;
	page=0;
}*/
void __Free_Channel_Images(void){
	int i;
	int imagesPerScreen = COLS[wideScreen]*ROWS;

	for(i=0; i<maxPages*imagesPerScreen; i++){
		//printf("%d: %d\n", i, orden[i]);

		if(!pageLoaded[i/imagesPerScreen]){
			i += imagesPerScreen;
			continue;
		}
		if(orden[i]!=EMPTY){
			MRC_Free_Texture(ThemeList[orden[i]].banner);
		}
	}

	for (i=0; i<maxPages; i++)
		pageLoaded[i] = FALSE;
	page=0;
}
void __Finish_ALL_GFX(void) {
	int i;
	if(thememode >= 0)
	__Free_Channel_Images();

	for(i=0; i<MAX_TEXTURES; i++){
		MRC_Free_Texture(textures[i]);
	}
	MRC_Finish();

	saveconfig=false;
}



void __Load_Images_From_Page(void) {
	void *imgBuffer=NULL;
	int i, max, pos, ret, theme;

	//#ifdef DEBUG_MODE
	//gprintf("Loading images...\n");
	//#endif

	max = COLS[wideScreen]*ROWS;
	pos = max*page;
	for(i = 0; i < max; i++){
		theme = orden[pos+i];
		if(theme != EMPTY){
			__Draw_Loading();

			// Load image from FAT
			if(ThemeList[theme].type == 20)
				sprintf(tempString,"%s:/config/wiithememanager/imgs/%s.png",getdevicename(thememode), ThemeList[theme].title);
			if(ThemeList[theme].type == 10)
				sprintf(tempString,"%s:/config/wiithememanager/imgs/%s", getdevicename(thememode), Theme_Png[theme]);
			ret = Fat_ReadFile(tempString, &imgBuffer, 1);
			//gprintf("ret from fat read images %d\n",ret);
			
			// Decode image
			if(ret > 0){
				ThemeList[theme].banner = MRC_Load_Texture(imgBuffer);
				free(imgBuffer);
			}
			else{
				ThemeList[theme].banner = __Create_No_Banner(ThemeList[theme].title, ANCHOIMAGEN[wideScreen], ALTOIMAGEN);
			}

			MRC_Resize_Texture(ThemeList[theme].banner, ANCHOIMAGEN[wideScreen], ALTOIMAGEN);
			__MaskBanner(ThemeList[theme].banner);
			MRC_Center_Texture(ThemeList[theme].banner, 1);
		}
		//MRC_Draw_Texture(64, 440, configuracionJuegos[theme].banner);
	}
	pageLoaded[page] = TRUE;
}




void __load_textures(void) {
	const char* fileNames[MAX_TEXTURES]={
		"_arrows", "_background", (wideScreen? "_container_wide" : "_container"), "_empty",
		"_loading", "_numbers"}; //, "_qmark"};
	const u8* defaultTextures[MAX_TEXTURES]={
		wiithememanager_arrows_png, wiithememanager_background_png, (wideScreen? wiithememanager_container_wide_png : wiithememanager_container_png), wiithememanager_empty_png,
		wiithememanager_loading_png, wiithememanager_numbers_png}; //, wiithememanager_qmark_png};

	int i, ret;
	char *imgData = NULL;

	for(i = 0; i < MAX_TEXTURES; i++){
		if(thememode == -1) {
			textures[i] = MRC_Load_Texture((void *)defaultTextures[i]);
		}
		else {
			sprintf(tempString, "%s:/config/wiithememanager/wiithememanager%s.png", getdevicename(thememode), fileNames[i]);
			ret = Fat_ReadFile(tempString, (void*)&imgData, 0);
			if(ret>0) {
				textures[i] = MRC_Load_Texture(imgData);
				free(imgData);
			}
		}
	}


	//Resize containers if widescreen
	if(wideScreen){
		MRC_Resize_Texture(textures[TEX_EMPTY], ANCHOIMAGEN[1], ALTOIMAGEN);
	}
	__MaskBanner(textures[TEX_EMPTY]);
	MRC_Center_Texture(textures[TEX_ARROWS], 6);
	MRC_Center_Texture(textures[TEX_CONTAINER], 2);
	MRC_Center_Texture(textures[TEX_EMPTY], 1);
}
/* Constant */
#define BLOCK_SIZE	0x1000
#define CHUNKS 1000000

/* Macros */
#define round_up(x,n)	(-(-(x) & -(n)))

u32 filesize(FILE * file) {
	u32 curpos, endpos;
	
	if(file == NULL)
		return 0;
	
	curpos = ftell(file);
	fseek(file, 0, 2);
	endpos = ftell(file);
	fseek(file, curpos, 0);
	
	return endpos;
}

s32 InstallFile(FILE * fp) {

	char * data;
	s32 ret, nandfile, ios = 2;
	u32 length = 0,numchunks, cursize, i;
	char filename[ISFS_MAXPATH] ATTRIBUTE_ALIGN(32);
	u32 newtmdsize ATTRIBUTE_ALIGN(32);
	u64 newtitleid ATTRIBUTE_ALIGN(32);
	signed_blob * newtmd;
	tmd_content * newtmdc, * newtmdcontent = NULL;
	
	char tite[256];
	sprintf(tite,"[+] Installing %s .", ThemeList[orden[selectedtheme]].title);
	__Draw_Message(tite,0);
	sleep(2);
		
	newtitleid = 0x0000000100000000LL + ios;
		
	ES_GetStoredTMDSize(newtitleid, &newtmdsize);
	newtmd = (signed_blob *) memalign(32, newtmdsize);
	memset(newtmd, 0, newtmdsize);
	ES_GetStoredTMD(newtitleid, newtmd, newtmdsize);
	newtmdc = TMD_CONTENTS((tmd *) SIGNATURE_PAYLOAD(newtmd));
		
	for(i = 0; i < ((tmd *) SIGNATURE_PAYLOAD(newtmd))->num_contents; i++)
	{
		if(newtmdc[i].index == 1)
		{
			newtmdcontent = &newtmdc[i];
				
			if(newtmdc[i].type & 0x8000) //Shared content! This is the hard part :P.
				return -1;
			else {//Not shared content, easy
				sprintf(filename, "/title/00000001/%08x/content/%08x.app", ios, newtmdcontent->cid);
			}
			break;
		}
		else if(i == (((tmd *) SIGNATURE_PAYLOAD(newtmd))->num_contents) - 1)
			return -1;
	}

	free(newtmd);
	
	nandfile = ISFS_Open(filename, ISFS_OPEN_RW);
	ISFS_Seek(nandfile, 0, SEEK_SET);
	
	length = filesize(fp);
	if(debugcard) logfile("length of file %d \n",length);
	numchunks = length/CHUNKS + ((length % CHUNKS != 0) ? 1 : 0);
	
	if(debugcard) logfile("[+] Total parts: %d\n", numchunks);
	
	for(i = 0; i < numchunks; i++)
	{
		
		data = memalign(32, CHUNKS);
		if(data == NULL)
		{
			return -1;
		}
		char *ms = memalign(32,256);
		
		sprintf(ms,"Installing part %d of %d parts ,",i + 1, numchunks);

		if(debugcard) logfile("	Installing part %d\n",(i + 1) );

		__Draw_Message(ms,0);
		
		ret = fread(data, 1, CHUNKS, fp);
		if (ret < 0) 
		{
			if(debugcard) logfile("[-] Error reading from SD! (ret = %d)\n\n", ret);
			
			//gprintf("	Press any button to continue...\n");
			return -1;
		}
		else
		{
			cursize = ret;
		}
		wiilight(1);
		ret = ISFS_Write(nandfile, data, cursize);
		if(ret < 0)
		{
			if(debugcard) logfile("[-] Error writing to NAND! (ret = %d)\n\n", ret);
			//gprintf("	Press any button to continue...\n");
			return ret;
		}
		free(data);
		free(ms);
		ms = NULL;
		wiilight(0);
	}
	
	ISFS_Close(nandfile);

	return 0;
}

/*
bool checkofficialthemesig(const char * name) {
	char filepath[256];
    FILE *fp = NULL;
    u32 length, i;
    u8 *themedata = NULL;
	//int readsiglen = strlen(name);
	//char readsig[readsiglen];
	
	sprintf(filepath, "%s:/%s/%s", getdevicename(thememode), themedir, name);
    fp = fopen(filepath, "rb");
    if (!fp) {
        printf("unable to open path\n");
		return 0;
	}
    length = filesize(fp);
    themedata = allocate_memory(length);
    memset(themedata,0,length);
    fread(themedata,1,length,fp);
	fclose(fp);
	//themesize = length;
    if(length <= 0) {
        printf("[-] Unable to read file !! \n");
		//logfile("[-] Unable to read file !! \n");
        return 0;
    }
	else {
		if(((themedata[2256] == 0) && (themedata[2334] == 85)) || ((themedata[2256] == 0) && (themedata[2334] == 69)) || ((themedata[2256] == 0) && (themedata[2334] == 75))) {
			official_theme = 1;
			sigchecked = true;
			free(themedata);
			return true;
		}
		if((themedata[17648] == 0) && (themedata[17726] == 74)) {
			official_theme = 1;
			sigchecked = true;
			free(themedata);
			return true;
		}
		for(i = 0; i < length; i++) {
			if((themedata[i] == 119) && (themedata[i+15] == 169) && (themedata[i+26] == 57)) {
				//logfile("foundsig\n");
				if((themedata[i] == 119) && (themedata[i + 8] == 114)) { // w_______r
					official_theme = 2;
				}
				if((themedata[i] == 119) && (themedata[i + 8] == 109)) { // w_____________r
					official_theme = 3;
				}
				
				
				//logfile("official_theme[%d]\n", official_theme);
				sigchecked = true;
				free(themedata);
				return true;
			}
			if((themedata[i] == 77) && (themedata[i + 1] == 111) && (themedata[i + 2] == 100) && (themedata[i + 3] == 77) && (themedata[i + 4] == 105) && (themedata[i + 5] == 105)) { //M_________________________k
				official_theme = 4;
				sigchecked = true;
				free(themedata);
				return true;
			}
		}
	}
	free(themedata);
	return false;
}
*/
bool installregion(u32 inputversion) {
	switch(inputversion) {
		case 416:
		case 448:
		case 480:
		case 512:
		case 608:
			ThemeList[orden[selectedtheme]].region = (u8*)74;
			break;
		case 417:
		case 449:
		case 481:
		case 513:
		case 609:
			ThemeList[orden[selectedtheme]].region = (u8*)85;
			break;
		case 418:
		case 450:
		case 482:
		case 514:
		case 610:
			ThemeList[orden[selectedtheme]].region = (u8*)69;
			break;
		case 454:
		case 486:
		case 518:
			ThemeList[orden[selectedtheme]].region = (u8*)75;
			break;
		default:
			ThemeList[orden[selectedtheme]].region = 0;
			return false;
		break;
	}
	return true;
}
u32 findinstallthemeversion(const char * name) { 
	char filepath[256];
    FILE *fp = NULL;
    u32 length, i, rtn = 0;
    u8 *data;
	
	sprintf(filepath, "%s:/themes/%s", getdevicename(thememode), name);
    fp = fopen(filepath, "rb");
    if (!fp) {
        if(debugcard) logfile("unable to open path\n");
		return 0;
	}
    length = filesize(fp);
    data = allocate_memory(length);
    memset(data,0,length);
    fread(data,1,length,fp);
	fclose(fp);
	
    if(length <= 0) {
        if(debugcard) logfile("[-] Unable to read file !! \n");
		//if(debugcard) logfile("[-] Unable to read file !! \n");
        return 0;
    }
    else {
        for(i = 0; i < length; i++)
        {
            if(data[i] == 83)
            {
                if(debugcard) logfile("data at [%d]\n", i);
				if(data[i+6] == 52)  // 4
                {
                    if(debugcard) logfile("data at [%d]\n", i);
					if(data[i+8] == 48)  // 0
                    {
                        if(debugcard) logfile("data at [%d]\n", i);
						if(data[i+28] == 85)  // usa
                        {
                            if(debugcard) logfile("data at [%d]\n", i);
							rtn = 417;
                           // rtn.region = 85;
                            break;
                        }
                        else if(data[i+28] == 74)  //jap
                        {
                            rtn = 416;
                            //rtn.region = 74;
                            break;
                        }
                        else if(data[i+28] == 69)  // pal
                        {
                            rtn = 418;
                            //rtn.region = 69;
                            break;
                        }
                    }
                    else if(data[i+8] == 49)  // 4.1
                    {
                        if(data[i+31] == 85)  // usa
                        {
                            rtn = 449;
                           // rtn.region = 85;
                            break;
                        }
                        else if(data[i+31] == 74)  //jap
                        {
                            rtn = 448;
                            //rtn.region = 74;
                            break;
                        }
                        else if(data[i+31] == 69)  // pal
                        {
                            rtn = 450;
                            //rtn.region = 69;
                            break;
                        }
                        else if(data[i+31] == 75)  // kor
                        {
                            rtn = 454;
                            //rtn.region = 75;
                            break;
                        }
                    }
					else if(data[i+8] == 50)  // 4.2
                    {
                        if(data[i+28] == 85)  // usa
                        {
                            if(debugcard) logfile("data at [%d]\n", i);
							rtn = 481;
                            //rtn.region = 85;
                            break;
                        }
                        else if(data[i+28] == 74)  // jap
                        {
                            rtn = 480;
                            //rtn.region = 74;
                            break;
                        }
                        else if(data[i+28] == 69)  // pal
                        {
                            rtn = 482;
                            //rtn.region = 69;
                            break;
                        }
                        else if(data[i+28] == 75)  // kor
                        {
                            rtn = 486;
                            //rtn.region = 75;
                            break;
                        }
                    }
                    else if(data[i+8] == 51) // 4.3
                    {
                        if(data[i+28] == 85)  // usa
                        {
                            rtn = 513;
                            //rtn.region = 85;
                            break;
                        }
                        else if(data[i+28] == 74)  //jap
                        {
                            rtn = 512;
                            //rtn.region = 74;
                            break;
                        }
                        else if(data[i+28] == 69)  // pal
                        {
                            rtn = 514;
                            //rtn.region = 69;
                            break;
                        }
                        else if(data[i+28] == 75)  // kor
                        {
                            rtn = 518;
                            //rtn.region = 75;
                            break;
                        }
                    }
                }
            }
			// add vwii here
        }
    }
	free(data);
	
	return rtn;
}
int currentthemeregion(){
	int rtn = 0;
	
	switch(systemmenuversion)
	{
		case 416: rtn = 74;
		break;
		case 417: rtn = 85;
		break;
		case 418: rtn = 69;
		break;
		case 448: rtn = 74;
		break;
		case 449: rtn = 85;
		break;
		case 450: rtn = 69;
		break;
		case 454: rtn = 75;
		break;
		case 480: rtn = 74;
		break;
		case 481: rtn = 85;
		break;
		case 482: rtn = 69;
		break;
		case 486: rtn = 75;
		break;
		case 512: rtn = 74;
		break;
		case 513: rtn = 85;
		break;
		case 514: rtn = 69;
		break;
		case 518: rtn = 75;
		break;
		case 608: rtn = 74;
		break;
		case 609: rtn = 85;
		break;
		case 610: rtn = 69;
		break;
	}
	return rtn;
}
int __install_Theme() {  // install.app .csm file
	//gprintf("install theme start! \n");
	char filepath[1024];
	FILE *fp = NULL;
	if(!priiloadercheck) {
		const char *nopriiloader = "Priiloader not detected . Installs Disabled .";
		__Draw_Message(nopriiloader, 0);
		sleep(3);
		return MENU_SELECT_THEME;
	}
	char *start = memalign(32,256);
	sprintf(start,"Starting Custom Theme Installation !");
	__Draw_Message(start,0);
	
	sprintf(filepath, "%s:/themes/%s", getdevicename(thememode), ThemeList[orden[selectedtheme]].title);
	if(debugcard) logfile("filepath (%s) \n",filepath);
	curthemestats.version = GetSysMenuVersion();
	if(curthemestats.version > 610) curthemestats.version = checkcustomsystemmenuversion();
	retreivecurrentthemeregion(curthemestats.version);
	if(debugcard) logfile("cur theme .version(%d) .region(%c) \n",curthemestats.version, curthemestats.region);
	ThemeList[orden[selectedtheme]].version = findinstallthemeversion(ThemeList[orden[selectedtheme]].title);
    installregion(ThemeList[orden[selectedtheme]].version);
	if(debugcard) logfile("install theme .version(%d) .region(%c) \n",ThemeList[orden[selectedtheme]].version,ThemeList[orden[selectedtheme]].region);
	
	if(curthemestats.version != ThemeList[orden[selectedtheme]].version) {
        const char *badversion = "Install can not continue system versions differ ! Press any button to exit.";
		__Draw_Message(badversion, 1);  
        sysHBC();
    }
    else if(curthemestats.region != ThemeList[orden[selectedtheme]].region) {
        const char *badregion = "Install can not continue system regions differ ! Press any button to exit.";
		__Draw_Message(badregion, 1);
        sysHBC();
    }
   
    fp = fopen(filepath, "rb");
    if (!fp){
        if(debugcard) logfile("[+] File Open Error not on SD!\n");
    }
	// Install 
    InstallFile(fp);

    // Close file 
    if(fp) fclose(fp);
		
	char *done = (char*)memalign(32,256);
	sprintf(done,"Your Custom Theme has been installed !");
	__Draw_Message(done,0);
	sleep(2);
	free(done);
	done = NULL;
	return MENU_EXIT_SYSTEMMENU;
}


int __Select_Theme(void){
	int i, j, hotSpot, hotSpotPrev, ret;
	ret = MENU_EXIT_HBC;

	if(themecnt < 0) {
		__Draw_Message("No Files Found .", 0);
		sleep(3);
		return MENU_HOME;
	}
	// Create/restore hotspots
	for(i = 0; i < ROWS; i++) {
		for(j = 0;j < COLS[wideScreen]; j++){
			int pos = i * COLS[wideScreen] + j;
			Wpad_AddHotSpot(pos,
				FIRSTCOL[wideScreen] - ANCHOIMAGEN[wideScreen] / 2 + j * SEPARACIONX[wideScreen],
				FIRSTROW - ALTOIMAGEN / 2 + i * SEPARACIONY,
				ANCHOIMAGEN[wideScreen],
				ALTOIMAGEN,
				(i == 0 ? COLS[wideScreen] * (ROWS-1) + j : pos-COLS[wideScreen]),
				(i == ROWS - 1 ? j : pos + COLS[wideScreen]),
				(j == 0 ? pos+COLS[wideScreen] - 1 : pos - 1),
				(j == COLS[wideScreen] - 1 ? pos - COLS[wideScreen]+1 : pos + 1)
			);
		}
	}
	Wpad_AddHotSpot(HOTSPOT_LEFT, 0,160,32,88, HOTSPOT_LEFT, HOTSPOT_LEFT, HOTSPOT_RIGHT, 0);
	Wpad_AddHotSpot(HOTSPOT_RIGHT, 640-32,160,32,88, HOTSPOT_RIGHT, HOTSPOT_RIGHT,(COLS[wideScreen]*2)-1, HOTSPOT_LEFT);
	hotSpot = hotSpotPrev = -1;

	// Load images from actual page
	if(!pageLoaded[page])
		__Load_Images_From_Page();
	__Draw_Page(-1);


	// Select game loop
	for(;;){
		hotSpot=Wpad_Scan();

		// If hot spot changed
		if(movingGame>-1){
			__Draw_Page(hotSpot);
		}
		else if(hotSpot!=hotSpotPrev){
			hotSpotPrev=hotSpot;
			__Draw_Page(hotSpot);
		}

		if(hotSpot >= 0 && hotSpot <= COLS[wideScreen]*ROWS){
			selectedtheme = page*COLS[wideScreen]*ROWS+hotSpot;
		}
		else{
			selectedtheme = -1;
		}

		MRC_Draw_Cursor(Wpad_GetWiimoteX(), Wpad_GetWiimoteY(), (movingGame > -1));

		if(movingGame == -1){
			if(((WPAD_ButtonsDown(WPAD_CHAN_0) & (WPAD_BUTTON_A | WPAD_BUTTON_B | WPAD_BUTTON_1)) || (PAD_ButtonsDown(0) & (PAD_BUTTON_A | PAD_TRIGGER_Z))) && hotSpot>-1 && hotSpot<COLS[wideScreen]*ROWS && orden[selectedtheme] != EMPTY){
				if(WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_B){
					movingGame = selectedtheme;
					findnumpages();
				}else if((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_A) || (PAD_ButtonsDown(0) & PAD_BUTTON_A)){
					ret = MENU_SHOW_THEME;
					break;
				}
			}
		}
		else{
			//moving game
			if(!(WPAD_ButtonsHeld(WPAD_CHAN_0) & WPAD_BUTTON_B)){
				if(selectedtheme != -1 && selectedtheme != movingGame){
					u16 copia0 = orden[movingGame];
					u16 copia1 = orden[selectedtheme];

					orden[movingGame] = copia1;
					orden[selectedtheme] = copia0;
					saveconfig = true;
				}
				movingGame = -1;
				findnumpages();
				__Draw_Page(hotSpot);
			}
		}

		if(WPAD_ButtonsDown(WPAD_CHAN_0) || PAD_ButtonsDown(0)){
			if(((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_MINUS) || (PAD_ButtonsDown(0) & PAD_TRIGGER_L)) || (hotSpot==HOTSPOT_LEFT && ((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_A) || (PAD_ButtonsDown(0) & PAD_BUTTON_A)))){
				//gprintf("page = %d maxpages = %d pages = %d\n",page,maxPages,pages);
				
				if(page == 0)
					page = maxPages;
				page--;
				ret = MENU_SELECT_THEME;
				break;
			}else if(((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_PLUS) || (PAD_ButtonsDown(0) & PAD_TRIGGER_R)) || (hotSpot==HOTSPOT_RIGHT && ((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_A) || (PAD_ButtonsDown(0) & PAD_BUTTON_A)))){
				page++;
				if (page >= maxPages)
					page = 0;
				ret = MENU_SELECT_THEME;
				break;
			}else if(((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_HOME) || (PAD_ButtonsDown(0) & PAD_BUTTON_START))){
				ret = MENU_HOME;
				break;
			}else if((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_B) || (PAD_ButtonsDown(0) & PAD_BUTTON_B)){
				ret = MENU_SELECT_THEME;
				break;
			}

			//if(WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_2)
			//	MRC_Capture();

		}

	}

	return ret;
}


const char *getsavename(u32 idx){
    switch(idx)
    {
    case 417:
        return "72";
        break;
    case 449:
        return "7b";
        break;
    case 481:
        return "87";
        break;
    case 513:
        return "97";// usa
        break;
    case 418:
        return "75";
        break;
    case 450:
        return "7e";
        break;
    case 482:
        return "8a";
        break;
    case 514:
        return "9a";// pal
        break;
    case 416:
        return "6f";
        break;
    case 448:
        return "78";
        break;
    case 480:
        return "84";
        break;
    case 512:
        return "94";// jpn
        break;
    case 486:
        return "8d";// kor
        break;
    case 454:
        return "81";
        break;
    case 518:
        return "9d";// kor
        break;
    default:
        return "UNK";
        break;
    }
}
void get_title_key(signed_blob *s_tik, u8 *key){
    static u8 iv[16] ATTRIBUTE_ALIGN(0x20);
    static u8 keyin[16] ATTRIBUTE_ALIGN(0x20);
    static u8 keyout[16] ATTRIBUTE_ALIGN(0x20);

    const tik *p_tik;
    p_tik = (tik*) SIGNATURE_PAYLOAD(s_tik);
    u8 *enc_key = (u8 *) &p_tik->cipher_title_key;
    memcpy(keyin, enc_key, sizeof keyin);
    memset(keyout, 0, sizeof keyout);
    memset(iv, 0, sizeof iv);
    memcpy(iv, &p_tik->titleid, sizeof p_tik->titleid);

    aes_set_key(commonkey);
    aes_decrypt(iv, keyin, keyout, sizeof keyin);

    memcpy(key, keyout, sizeof keyout);
}
void decrypt_buffer(u16 index, u8 *source, u8 *dest, u32 len){
    static u8 iv[16];
    memset(iv, 0, 16);
    memcpy(iv, &index, 2);
    aes_decrypt(iv, source, dest, len);
}
const char *getPath(int ind){
    switch(ind){
    case 0:
        return "http://nus.cdn.shop.wii.com/ccs/download/0000000100000002/cetk";
        break;
    case 1:
        return "http://nus.cdn.shop.wii.com/ccs/download/0000000100000002/tmd.";
        break;
        return "http://nus.cdn.shop.wii.com/ccs/download/0000000100000002/0000006f";
        break;
    case 6:
        return "http://nus.cdn.shop.wii.com/ccs/download/0000000100000002/00000072";
        break;
    case 7:
        return "http://nus.cdn.shop.wii.com/ccs/download/0000000100000002/00000075";
        break;
    case 8:
        return "http://nus.cdn.shop.wii.com/ccs/download/0000000100000002/00000078";
        break;
    case 9:
        return "http://nus.cdn.shop.wii.com/ccs/download/0000000100000002/0000007b";
        break;
    case 10:
        return "http://nus.cdn.shop.wii.com/ccs/download/0000000100000002/0000007e";
        break;
    case 11:
        return "http://nus.cdn.shop.wii.com/ccs/download/0000000100000002/00000081";
        break;
    case 12:
        return "http://nus.cdn.shop.wii.com/ccs/download/0000000100000002/00000084";
        break;
    case 13:
        return "http://nus.cdn.shop.wii.com/ccs/download/0000000100000002/00000087";
        break;
    case 14:
        return "http://nus.cdn.shop.wii.com/ccs/download/0000000100000002/0000008a";
        break;
    case 15:
        return "http://nus.cdn.shop.wii.com/ccs/download/0000000100000002/0000008d";
        break;
    case 16:
        return "http://nus.cdn.shop.wii.com/ccs/download/0000000100000002/00000094";
        break;
    case 17:
        return "http://nus.cdn.shop.wii.com/ccs/download/0000000100000002/00000097";
        break;
    case 18:
        return "http://nus.cdn.shop.wii.com/ccs/download/0000000100000002/0000009a";
        break;
    case 19:
        return "http://nus.cdn.shop.wii.com/ccs/download/0000000100000002/0000009d";
        break;
    default:
        return "ERROR";
        break;
    }
}
int getslot(int num){
    switch(num)
    {
    case 416:
        return 5;
        break;
    case 417:
        return 6;
        break;
    case 418:
        return 7;
        break;
    case 448:
        return 8;
        break;
    case 449:
        return 9;
        break;
    case 450:
        return 10;
        break;
    case 454:
        return 11;
        break;
    case 480:
        return 12;
        break;
    case 481:
        return 13;
        break;
    case 482:
        return 14;
        break;
    case 486:
        return 15;
        break;
    case 512:
        return 16;
        break;
    case 513:
        return 17;
        break;
    case 514:
        return 18;
        break;
    case 518:
        return 19;
        break;
    default:
        return -1;
        break;
    }
}
int __downloadApp(int downloadonly)  {
	__Draw_Loading();
	if(thememode < 0) {
		if(debugcard) {
			logfile("__downloadApp - thememode[%d] downloadable_theme_List[%d] netconnection[%d]\n", thememode, downloadable_theme_List, netconnection);
		}
		return MENU_HOME;
	}
	char *tmpstr = (char*)malloc(256);
	if(!netconnection) {
		if(debugcard) {
			logfile("Internet connection not detected .\n");
			sprintf(tmpstr,"Unable to connect to internet . Please check Wii Internet settings . ");
			__Draw_Message(tmpstr, 0);
			sleep(5);
			free(tmpstr);
		}
		return MENU_HOME;
	}
	u32 tmpversion;
    int ret; //, retries;
    int counter;
    char *savepath = (char*)memalign(32,256);
	
	char *tmpstr2 = (char*)malloc(256);
	signed_blob * s_tik = NULL;
    signed_blob * s_tmd = NULL;
	
    tmpversion = GetSysMenuVersion();
    if(tmpversion > 610) tmpversion = checkcustomsystemmenuversion();
	sprintf(tmpstr2,"%s:/themes/%s", getdevicename(thememode), getappname(tmpversion));
	if(!Fat_CheckFile(tmpstr2)){
		sprintf(tmpstr,"Downloading %s for System Menu v%u ", getappname(tmpversion), tmpversion);
		__Draw_Message(tmpstr, 0);
		sleep(1);
		__Draw_Loading();
		sprintf(tmpstr,"%s:/themes", getdevicename(thememode));
		//if(!Fat_CheckFile(tmpstr))
		Fat_MakeDir(tmpstr);
		/*sprintf(tmpstr,"Initializing  Network ....");
		__Draw_Message(tmpstr, 0);
		sleep(1);
		for(retries = 0; retries < 5; ++retries) {
			__Draw_Loading();
			ret=net_init();
			if(ret < 0 && ret != -EAGAIN){
			   sprintf(tmpstr,"Connection Failed ! Please check internet connection .");
				__Draw_Message(tmpstr, 0);
				net_deinit();
				sleep(1);
				if(retries >= 5)
				goto end;
			}
			if(ret == 0){
				sprintf(tmpstr,"Connection Complete !!");
				__Draw_Message(tmpstr, 0);
				sleep(1);
				break;
			}
		}*/
		for(counter = 0; counter < 3; counter++) {	
			__Draw_Loading();
			if(counter == 0 || counter == 1) sprintf(tmpstr,"DownLoading %s File .", appfilename[counter]);
			else sprintf(tmpstr,"DownLoading %s File .", getappname(tmpversion));
			__Draw_Message(tmpstr,0);
			sleep(1);
			char *path = (char*)memalign(32, 256);
			int aa = getslot(tmpversion);
			if(counter == 0) sprintf(path,"%s",getPath(counter));
			if(counter == 1) sprintf(path,"%s%d",getPath(counter), tmpversion);
			if(counter == 2) sprintf(path,"%s",getPath(aa));
			u32 outlen = 0;
			u32 http_status = 0;
			u32 Maxsize = 4294967295;
			ret = http_request(path, Maxsize);
			sprintf(tmpstr,"DownLoad Complete !");
			__Draw_Message(tmpstr,0);
			sleep(1);
			free(path);
			u8* outbuf = (u8*)malloc(outlen);
			if(counter == 0) {
				ret = http_get_result(&http_status, (u8 **)&s_tik, &outlen);
			}
			if(counter == 1) {
				ret = http_get_result(&http_status, (u8 **)&s_tmd, &outlen);
			}
			if(counter == 2) {
				ret = http_get_result(&http_status, &outbuf, &outlen);
			}
			sprintf(tmpstr,"Decrypting files ....");
			__Draw_Message(tmpstr,0);
			sleep(1);
			//set aes key
			u8 key[16];
			u16 index;
			get_title_key(s_tik, key);
			aes_set_key(key);
			u8* outbuf2 = (u8*)malloc(outlen);
			if(counter == 2) {
				if(outlen > 0) {//suficientes bytes
					index = 01;
					//then decrypt buffer
					decrypt_buffer(index,outbuf,outbuf2,outlen);
					sprintf(savepath,"%s:/themes/%s", getdevicename(thememode), getappname(tmpversion));
					sprintf(tmpstr,"Saving %s .", getappname(tmpversion));
					__Draw_Message(tmpstr,0);
					sleep(1);
					ret = Fat_SaveFile(savepath, (void *)&outbuf2, outlen);
					if(ret <= 0) {
						sprintf(tmpstr,"Saving %s Failed .", getappname(tmpversion));
						__Draw_Message(tmpstr,0);
						sleep(2);
					}
				}
			}
			sprintf(tmpstr,"Decryption  Complete !");
			__Draw_Message(tmpstr,0);
			sleep(1);
			if(outbuf!=NULL)
				free(outbuf);
		}
		net_deinit();
		
		sprintf(tmpstr,"[+] Download %s Complete .", getappname(tmpversion));
		__Draw_Message(tmpstr,0);
		sleep(2);
	}
	
//end:

	//free(oldapp);
	free(savepath);
	free(tmpstr);
	free(tmpstr2);
	
	return MENU_HOME;
}


int retrieve_themefilesize(int pos) {
	char *size = "0";
	int ret;
	char sitepath[512];
	const char *siteUrl = "http://wiithemer.org/wii/index.php?action=";
	u32 outlen = 0;
	u32 http_status = 0;
	u32 Maxsize = 4294967295;
	u8* outbuf = NULL;
	int convert_to_int = 0;
	
	if(debugcard) logfile("in retrieve_themefilesize\n");
	
	__Draw_Loading();
	sprintf(sitepath, "%s%s&themetocheck=%s", siteUrl, "getthemefilesize", DBThemelist_nospaces[pos]);
	if(debugcard) logfile("sitepath[%s]\n", sitepath);
	ret = http_request(sitepath, Maxsize);
	if(ret != 0 ) {
		ret = http_get_result(&http_status, &outbuf, &outlen);
		if(ret != 0 ) {
			if(outlen > 0 && http_status == 200) {
				size = (char*)outbuf;
				
				if(debugcard) logfile("size = %s\n", size);
				convert_to_int = atoi(size);
				if(debugcard) logfile("convert_to_int[%d]\n", convert_to_int);
				sprintf(size, "%d", convert_to_int);
				if(debugcard) logfile("size[%s]\n", size);
			}
		}
	}
	__Draw_Loading();
	return convert_to_int;
}
int retrieve_downloadcount(int pos) {
	char *count = "0";
	int ret;
	char sitepath[512];
	const char *siteUrl = "http://wiithemer.org/wii/index.php?action=";
	u32 outlen = 0;
	u32 http_status = 0;
	u32 Maxsize = 4294967295;
	u8* outbuf = NULL;
	int convert_to_int = 0;
	
	if(debugcard) logfile("in retreive downloadcount\n");
	
	__Draw_Loading();
	sprintf(sitepath, "%s%s&themetocheck=%s", siteUrl, "getdownloadcount", DBThemelist_nospaces[pos]);
	if(debugcard) logfile("sitepath[%s]\n", sitepath);
	ret = http_request(sitepath, Maxsize);
	if(ret != 0 ) {
		ret = http_get_result(&http_status, &outbuf, &outlen);
		if(ret != 0 ) {
			if(outlen > 0 && http_status == 200) {
				count = (char*)outbuf;
				if(debugcard) logfile("count = %s\n", count);
				convert_to_int = atoi(count);
				if(debugcard) logfile("convert_to_int[%d]\n", convert_to_int);
				sprintf(count, "%d", convert_to_int);
				if(debugcard) logfile("count[%s]\n", count);
			}
		}
	}
	__Draw_Loading();
	return convert_to_int;
}
#define PROJECTION_HEIGHT 128
#define MB_SIZE		1048576.0
int __Show_Theme(){
	void* imageBuffer;
	MRCtex *themeImage, *projection;
	int i, j, ret, size = 0;
	char *c, *r, a;
	int count = -1;
	char *countstr = NULL;
	char *sizestr = NULL;
	char file_size [32] = {"0"};
	ModTheme *thetheme = &ThemeList[orden[selectedtheme]];
	char downloadcount[32] = {"0"};
	char fsize[256];
	char filepath[256];
	char checkpath[256];
	char tmpstr[128];
	FILE *fptr = NULL;
	const char *config_dir = "config/wiithememanager/";
	const char *downloadcount_file = "downloads.txt";
	const char *filesize_file = "filesize.txt";
	void* outbuf = NULL;
	//char *officialthemes[5] = { "Unsigned Theme", "Original Theme Unmodified", "Wii Themer Signed", "Wii Theme Manager Signed", "ModMii Signed"};
	
	//if(!sigchecked) checkofficialthemesig(thetheme->title);
	// BLACK SCREEN
	/*a=160;
	for(i=0; i<480; i++){
		if(a<255 && ((i<208 && i%4==0) || i%8==0))
			a++;
		MRC_Draw_Box(0, i, 640, 1, a);
	}*/
	
	if(downloadable_theme_List) {
		__Draw_Loading();
		/*
		if(netconnection) {
		//if(debugcard) logfile("- downloadcount[%d]\n", thetheme->downloadcount);
		sprintf(checkpath, "%s:/%s%s/%s", getdevicename(thememode), config_dir, thetheme->titlenospace, downloadcount_file);
		if(ThemeList[orden[selectedtheme]].downloadcount == -1) {
			//if(debugcard) logfile("checkpath[%s]\n", checkpath);
			if(!Fat_CheckFile(checkpath)) {
				ThemeList[orden[selectedtheme]].downloadcount = retrieve_downloadcount(selectedtheme);
				//if(debugcard) logfile("count[%d]\n", thetheme->downloadcount);
				sprintf(tmpstr, "%s:/%s%s", getdevicename(thememode), config_dir, thetheme->titlenospace);
				Fat_MakeDir(tmpstr);
				fptr = fopen(checkpath, "w+");
				sprintf(downloadcount, "%d", ThemeList[orden[selectedtheme]].downloadcount);
				ret = fputs(downloadcount, fptr);
				fclose(fptr);
				sprintf(downloadcount, "Downloads %d", ThemeList[orden[selectedtheme]].downloadcount);
				//if(debugcard) logfile("downloads.txt not found - downloadcount[%s]\n", downloadcount);
			}
			else {
				//if(debugcard) logfile("file found - %s\n", checkpath);
				Fat_ReadFile(checkpath, &outbuf, 1);
				countstr = (char*)outbuf;
				//if(debugcard) logfile("countstr[%s]\n", countstr);
				count = atoi(countstr);
				sprintf(downloadcount, "Downloads %d", count);
				//if(debugcard) logfile("downloads.txt found - downloadcount[%s]\n", downloadcount);
				ThemeList[orden[selectedtheme]].downloadcount = count;
			}
		}
		else { 
			//if(debugcard) logfile("file found - %s\n", checkpath);
			Fat_ReadFile(checkpath, &outbuf, 1);
			countstr = (char*)outbuf;
			//if(debugcard) logfile("countstr[%s]\n", countstr);
			count = atoi(countstr);
			sprintf(downloadcount, "Downloads %d", count);
			//if(debugcard) logfile("downloads.txt found - downloadcount[%s]\n", downloadcount);
			ThemeList[orden[selectedtheme]].downloadcount = count;
		}
		//if(debugcard) logfile("SIZE: %d\n", ThemeList[orden[selectedtheme]].size);
		sprintf(checkpath, "%s:/%s%s/%s", getdevicename(thememode), config_dir, thetheme->titlenospace, filesize_file);
		if(ThemeList[orden[selectedtheme]].size == 0) {
			if(!Fat_CheckFile(checkpath)) {
				ThemeList[orden[selectedtheme]].size = retrieve_themefilesize(selectedtheme);//atoi(file_size);
				sprintf(tmpstr, "%s:/%s%s", getdevicename(thememode), config_dir, thetheme->titlenospace);
				Fat_MakeDir(tmpstr);
				fptr = fopen(checkpath, "w+");
				sprintf(file_size, "%d", ThemeList[orden[selectedtheme]].size);
				//if(debugcard) logfile("file_size[%s] fputs\n", file_size);
				fputs(file_size, fptr);
				fclose(fptr);
				sprintf(file_size, "%s  Size: %.2f MB", thetheme->title, ThemeList[orden[selectedtheme]].size/MB_SIZE);
			}
			else {
				Fat_ReadFile(checkpath, &outbuf, 1);
				sizestr = (char*)outbuf;
				size = atoi(sizestr);
				ThemeList[orden[selectedtheme]].size = size;
				sprintf(file_size, "%s  Size: %.2f MB", thetheme->title, size/MB_SIZE);
			}
		}
		else {
			Fat_ReadFile(checkpath, &outbuf, 1);
			sizestr = (char*)outbuf;
			size = atoi(sizestr);
			ThemeList[orden[selectedtheme]].size = size;
			sprintf(file_size, "%s  Size: %.2f MB", thetheme->title, size/MB_SIZE);
		}
		}
		*/
	}
	else {
		__Draw_Loading();
		sprintf(filepath, "%s:/%s/%s", getdevicename(thememode), themedir , thetheme->title);
		//if(debugcard) logfile("filepath[%s]\n", filepath);
		fptr = fopen(filepath, "rb");
		if(!fptr) logfile("unable to open path[%s]\n", filepath);
		size = filesize(fptr);
		fclose(fptr);
		sprintf(fsize, "%s  Size: %.2f MB", thetheme->title, size/MB_SIZE);
	}
	__Draw_Loading();
	// ANOTHER SCREEN FADE TYPE
	a=200;
	for(i=0; i<480; i++) {
		if(a<255 && ((i<100 && i%4==0) || (i>200 && i%8==0)))
			a++;
		MRC_Draw_Box(0, i, 640, 1, 0x20202000+a);
	}

	
	// Load image from FAT
	if(ThemeList[selectedtheme].type == 20)
		sprintf(tempString,"%s:/" WIITHEMEMANAGER_PATH IMAGES_PREFIX "/%s.png", getdevicename(thememode) , thetheme->title);
	if(ThemeList[selectedtheme].type == 10)
		sprintf(tempString,"%s:/config/wiithememanager/imgs/%s" , getdevicename(thememode),Theme_Png[orden[selectedtheme]]);
		
	//gprintf("tempstring %s \n",tempString);
	ret = Fat_ReadFile(tempString, &imageBuffer, needloading);
	// Decode image
	if(ret > 0) {
		themeImage = MRC_Load_Texture(imageBuffer);
		free(imageBuffer);

		MRC_Resize_Texture(themeImage, (wideScreen ? 640 : 580), 325);
		__MaskBanner(themeImage);
		//MRC_Center_Texture(themeImage, 1);
		projection = allocate_memory(sizeof(MRCtex));
		projection->buffer=allocate_memory(themeImage->width*PROJECTION_HEIGHT*4);
		projection->width=themeImage->width;
		projection->height=PROJECTION_HEIGHT;
		MRC_Center_Texture(projection, 1);
		projection->alpha = true;
	
		a = 128;
		r = (projection->buffer);
		for(i=0; i<PROJECTION_HEIGHT; i++){
			c=(themeImage->buffer)+(((themeImage->height-1)-i*2)*themeImage->width)*4;
			for(j=0; j<themeImage->width; j++){
				r[0]=c[0];
				r[1]=c[1];
				r[2]=c[2];
				r[3]=a;
				c+=4;
				r+=4;
			}
			if(a>4)
				a-=4;
		}
	
		MRC_Draw_Texture(0, 0, themeImage);
		MRC_Draw_Texture(320, 375, projection);
		
		
		if(downloadable_theme_List) {
			if(netconnection) {
				MRC_Draw_String(((640-strlen(fsize)*8)/2), 400, WHITE, file_size);
				MRC_Draw_String(((640-strlen(fsize)*8)/2), 420, WHITE, downloadcount);
			}
			else MRC_Draw_String(((640-strlen(thetheme->title))/2), 420, WHITE, thetheme->title);
		}
		else {
			MRC_Draw_String(((640-strlen(fsize))/2), 400, WHITE, fsize);
		}
		//MRC_Draw_String(30, 330, WHITE, "By ");
		sprintf(tempString, "%s", (downloadable_theme_List == 1 ? "[A] - Download Theme" : "[A] - Install Theme"));
		MRC_Draw_String(40, 400, WHITE, tempString);
		MRC_Draw_String(40, 420, WHITE, "[B] - Back");
		MRC_Free_Texture(themeImage);
		MRC_Free_Texture(projection);
	}
	else{
		sprintf(tempString, "%s", (downloadable_theme_List == 1 ? "[A] - Download Theme" : "[A] - Install Theme"));
		MRC_Draw_String(30, 400, WHITE, tempString);
		MRC_Draw_String(30, 420, WHITE, "[B] - Back");
		if(downloadable_theme_List) {
			if(netconnection) {
				MRC_Draw_String(((640-strlen(fsize)*8)/2), 400, WHITE, file_size);
				MRC_Draw_String(((640-strlen(fsize)*8)/2), 420, WHITE, downloadcount);
			}
			else MRC_Draw_String(((640-strlen(thetheme->title)*8)/2), 420, WHITE, thetheme->title);
		}
		else {
			MRC_Draw_String(((640-strlen(fsize)*8)/2), 400, WHITE, fsize);
			//sprintf(tempString, "Theme Signature : %s", officialthemes[official_theme]);
			//MRC_Draw_String(((640-strlen(tempString)*8)/2), 380, WHITE, tempString);
		}
	}
	MRC_Render_Screen();
	int answer = -1;
	
	ret = MENU_SELECT_THEME;
	for(;;) {
		WPAD_ScanPads();
		PAD_ScanPads();
		if(WPAD_ButtonsDown(WPAD_CHAN_0) || PAD_ButtonsDown(0)){
			if((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_A) || (PAD_ButtonsDown(0) & PAD_BUTTON_A)){
				if(downloadable_theme_List) {
					answer = __Spin_Question();
					if(debugcard) logfile("answer[%i]\n", answer);
				}
				
				ret = MENU_INSTALL_THEME;
				break;
			}
			if((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_B) || (PAD_ButtonsDown(0) & PAD_BUTTON_B)){
				//ret = MENU_SELECT_THEME;
				//official_theme = 0;
				//sigchecked = false;
				break;
			}
		}
		
	}
	spinselected = answer;
	//free(downloadcount);
	return ret;
}


#define HOME_BUTTON_X			210
#define HOME_BUTTON_Y			125
#define HOME_BUTTON_WIDTH		220
#define HOME_BUTTON_HEIGHT		30
#define HOME_BUTTON_SEPARATION	5
int __Home(void) {
	int i, hotSpot, hotSpotPrev, ret;
	bool repaint = true;
	//char message[256];
	// Create/restore hotspots
	Wpad_CleanHotSpots();
	for(i = 0; i < 7; i++){
		Wpad_AddHotSpot(i,
			HOME_BUTTON_X,
			HOME_BUTTON_Y+i*(HOME_BUTTON_HEIGHT+HOME_BUTTON_SEPARATION),
			HOME_BUTTON_WIDTH,
			HOME_BUTTON_HEIGHT,
			(i == 0 ? 6 : i - 1),
			(i == 6 ? 0 : i + 1),
			i, i
		);
	}
	__Draw_Window(HOME_BUTTON_WIDTH + 45, 300, "Options");
	sprintf(tempString, "Device : %s", getdevicename(thememode));
	MRC_Draw_String(((640-strlen(tempString)*8)/2), 370, BLACK, tempString);
	
	hotSpot = hotSpotPrev = -1;

	ret = MENU_SELECT_THEME;
	for(;;) {
		hotSpot = Wpad_Scan();

		// If hot spot changed
		if((((hotSpot != hotSpotPrev) && (hotSpot < 7)) || repaint)) {
			hotSpotPrev = hotSpot;
			__Draw_Button(0, "Device Menu", hotSpot == 0);
			if(thememode < 0) {
				__Draw_Button(1, "", hotSpot == 1);
				__Draw_Button(2, "", hotSpot == 2);
				__Draw_Button(3, "", hotSpot == 3);
			}
			else {
				//if(downloadable_theme_List) 
				__Draw_Button(1, "Swap Mode", hotSpot == 1);
				//else __Draw_Button(1, "Download Mode", hotSpot == 1);
				if(!downloadable_theme_List) __Draw_Button(2, "", hotSpot == 2);
				else __Draw_Button(2, "Download Theme Images", hotSpot == 2);
				if(thememode < 0) __Draw_Button(3, "", hotSpot == 3);
				else __Draw_Button(3, "Download Original Theme", hotSpot == 3);
			}
			__Draw_Button(4, "Exit Hbc", hotSpot == 4);
			__Draw_Button(5, "Exit System Menu", hotSpot == 5);
			__Draw_Button(6, "Exit PriiLoader", hotSpot == 6);
			repaint = false;
		}
		MRC_Draw_Cursor(Wpad_GetWiimoteX(), Wpad_GetWiimoteY(), 0);
		//gprintf("hotSpot = %d \n",hotSpot);
		if(((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_A) || (PAD_ButtonsDown(0) & PAD_BUTTON_A)) && hotSpot > -1 && hotSpot < 7){
			__Draw_Loading();
			if(hotSpot == 0)
				ret = MENU_MANAGE_DEVICE;
			else if(hotSpot == 1) {
				if(thememode >= 0)
					downloadable_theme_List ^= 1;
				ret = MENU_SELECT_THEME;
				if(themecnt > 0) __Free_Channel_Images();
					themecnt = filelist_retrieve(downloadable_theme_List);
				__Load_Config();
			}
			else if(hotSpot == 2)
				ret = MENU_DOWNLOAD_IMAGE;
			else if(hotSpot == 3)
				ret = MENU_ORIG_THEME;
			else if(hotSpot == 4)
				ret = MENU_EXIT_HBC;
			else if(hotSpot == 5)
				ret = MENU_EXIT_SYSTEMMENU;
			else if(hotSpot == 6)
				ret = MENU_EXIT_PRIILOADER;
			break;
		}
		else if(((WPAD_ButtonsDown(WPAD_CHAN_0) & (WPAD_BUTTON_HOME | WPAD_BUTTON_B)) || (PAD_ButtonsDown(0) & (PAD_BUTTON_START | PAD_BUTTON_B)))) {
			if(thememode < 0) ret = MENU_HOME;
			break;
		}
		else {
			repaint = true;
		}
	}

	return ret;
}
#define dir_delimter '/'
#define MAX_FILENAME 512
#define READ_SIZE 8192
int __Downloadthemepng() {
	if(debugcard) logfile("__Downloadthemepng() \n");
	__Draw_Loading();
	if(thememode < 0) return MENU_HOME;
	if(!netconnection) return MENU_HOME;
	if(!downloadable_theme_List) return MENU_HOME;
	
	char *fatpath = (char*)malloc(1024);
	char *tmpstr = (char*)malloc(1024);
	int ret; //, i, error = 0;
	u32 outlen=0;
	u32 http_status=0;
	u32 Maxsize = 4294967290;
	u8* outbuf = NULL;
	
	
	__Draw_Loading();
	if(debugcard) logfile("thememode = %d \n",thememode);
	sprintf(tmpstr,"%s:/config/wiithememanager/imgs",getdevicename(thememode));
	if(!Fat_CheckFile(tmpstr))
		Fat_CreateSubfolder(tmpstr);
	sprintf(fatpath,"%s:/config/wiithememanager/wiithememanagerimages.zip", getdevicename(thememode));
	if(debugcard) logfile("fatpath(%s) \n",fatpath);
	if(!Fat_CheckFile(fatpath)) {
		sprintf(tmpstr,"Starting Theme Image download ....");
		__Draw_Message(tmpstr,0);
		sleep(2);
		/*for(int retries = 0; retries < 5; ++retries){
			__Draw_Loading();
			ret = net_init();
			if(ret < 0 && ret != -EAGAIN) {
				sprintf(tempString, "ERROR: I can't connect to network . (returned %d)\n",ret);
				__Draw_Message(tempString, 0);
				sleep(2);
			}
			if(ret == 0) //consigo conexion
				break;
			if(retries >= 5){
				sleep(2);
				return MENU_HOME;
			}
		}*/
		__Draw_Loading();
		sprintf(tmpstr,"Downloading Theme images - wiithememanager.zip .");
		__Draw_Message(tmpstr,0);
		sleep(1);
		__Draw_Loading();
		sprintf(tempString, "http://wiithemer.org/wii/wiithememanagerimages.zip");
		if(debugcard) logfile("tempString(%s) \n",tempString);
		sprintf(fatpath,"%s:/config/wiithememanager/wiithememanagerimages.zip", getdevicename(thememode));
			if(debugcard) logfile("fatpath(%s) \n",fatpath);
		ret = http_request(tempString, Maxsize);//Maxsize);
		if(ret) {
			ret = http_get_result(&http_status, &outbuf, &outlen);
			if(ret) {
				if(outlen > 64) {
					ret = Fat_SaveFile(fatpath, (void *)&outbuf,outlen);
					if(ret < 0) {
						sprintf(tmpstr,"Saving Images Falied !");
						__Draw_Message(tmpstr,0);
						sleep(2);
					}
				}
			}
		}
	}
	/* single image downloading
	if(!themecnt) themecnt = 73;
	for(i = 0;i < themecnt; i++){
		__Draw_Loading();
		sprintf(tmpstr,"Downloading %d of %d Theme Previews .",i+1,themecnt);
		__Draw_Message(tmpstr,0);
		sleep(1);
		__Draw_Loading();
		sprintf(tempString, "http://wiithemer.org/wii/wiithememanager/imgs/%s", Theme_Png[i]);
		if(debugcard) logfile("tempString(%s) \n",tempString);
		sprintf(fatpath,"%s:/config/wiithememanager/imgs/%s", getdevicename(thememode), Theme_Png[i]);
		if(debugcard) logfile("fatpath(%s) \n",fatpath);
		__Draw_Loading();
		if(!Fat_CheckFile(fatpath)){
			
			
			u32 outlen=0;
			u32 http_status=0;
			u32 Maxsize = 4294967295;
			u8* outbuf=NULL;
			__Draw_Loading();
			ret = http_request(tempString, Maxsize);//Maxsize);
			if(ret == 0 )
			{
			//	if(debugcard) logfile("download failed !! ret(%d)\n",ret);
				sprintf(tmpstr,"Download %d of %d Theme Previews Failed !",i+1,themecnt);
				__Draw_Message(tmpstr,0);
				sleep(3);
				error+=1;
				return 0;
				//continue;
			}
			//else
			//	if(debugcard) logfile("Complete !! \n\n");
			__Draw_Loading();
			ret = http_get_result(&http_status, &outbuf, &outlen); 
			
			if(outlen > 64){//suficientes bytes
				//sprintf(fatpath,"%s:/config/wiithememanager/DBimages/%s", getdevicename(thememode), Theme_Png[i]);
			//	if(debugcard) logfile("fatpath(%s) \n",fatpath);
				ret = Fat_SaveFile(fatpath, (void *)&outbuf,outlen);
				if(ret < 0){
				sprintf(tmpstr,"Saving Preview Falied !");
				__Draw_Message(tmpstr,0);
				sleep(2);
				error+=1;
				//continue;
				}
				//else
				//	if(debugcard) logfile("File Saved \n");
			}
			__Draw_Loading();	
			if(outbuf!=NULL)
				free(outbuf);
			__Draw_Loading();	
		}	
		__Draw_Loading();
	}
	
	net_deinit();

	if(debugcard) logfile("Extract files here \n");
	sprintf(tmpstr,"Theme Image download Complete .");
	__Draw_Message(tmpstr,0);
	sleep(3);
	*/
	//u8* buf = NULL;
	FILE *fptr = NULL;
	//int size = 0;
	char outpath[1024];
	
	if(Fat_CheckFile(fatpath)) {
		if(debugcard) logfile("file exists\n");
		
		fptr = unzOpen(fatpath);
		if(!fptr)
			logfile("unable to open %s", fatpath);
		else logfile("file opened\n");
		unz_global_info global_info;
		if(unzGetGlobalInfo( fptr, &global_info ) != UNZ_OK ) {
			logfile( "could not read file global info\n" );
			unzClose( fptr );
			return MENU_HOME;
		}
		logfile("global_info.number_entry(%d)\n", global_info.number_entry);
		 char read_buffer[ READ_SIZE ];
		uLong i;
		for ( i = 0; i < global_info.number_entry; ++i )
		{
			__Draw_Loading();
			unz_file_info file_info;
			char filename[ MAX_FILENAME ];
			if ( unzGetCurrentFileInfo(fptr, &file_info, filename, MAX_FILENAME, NULL, 0, NULL, 0 ) != UNZ_OK ) {
				logfile( "could not read file info\n" );
				unzClose( fptr );
				return MENU_HOME;
			}

			// Check if this entry is a directory or file.
			const size_t filename_length = strlen(filename);
			if(filename[filename_length - 1] == dir_delimter ) {
				// Entry is a directory, so create it.
				logfile( "dir:%s\n", filename );
				mkdir(filename, S_IREAD | S_IWRITE);
			}
			else {
				// Entry is a file, so extract it.
				logfile( "file:%s\n", filename );
				sprintf(outpath, "%s:/config/wiithememanager/%s", getdevicename(thememode), filename);
				if(!Fat_CheckFile(outpath)) {
				logfile("outpath(%s)\n", outpath);
				if ( unzOpenCurrentFile( fptr ) != UNZ_OK )
				{
					logfile( "could not open file\n" );
					unzClose( fptr );
					return MENU_HOME;
				}

				// Open a file to write out the data.
				FILE *out = fopen( outpath, "wb" );
				if ( out == NULL )
				{
					logfile( "could not open destination file\n" );
					unzCloseCurrentFile( fptr );
					unzClose( fptr );
					return MENU_HOME;
				}
				wiilight(1);
						sprintf(tmpstr,"Saving %s .", filename);
						__Draw_Message(tmpstr,0);
				int error = UNZ_OK;
				do    
				{
					__Draw_Loading();
					error = unzReadCurrentFile( fptr, read_buffer, READ_SIZE );
					if ( error < 0 )
					{
						logfile( "error %d\n", error );
						unzCloseCurrentFile( fptr );
						unzClose( fptr );
						return MENU_HOME;
					}

					// Write data to file.
					if ( error > 0 )
					{
						
						fwrite( read_buffer, error, 1, out ); // You should check return of fwrite...
						
					}
				} while ( error > 0 );

				fclose( out );
				sprintf(tmpstr,"Saving %s Complete .", filename);
				__Draw_Message(tmpstr,0);
				wiilight(0);
				}
				
			}

			unzCloseCurrentFile( fptr );
			
			 // Go the the next entry listed in the zip file.
			if((i + 1) < global_info.number_entry ) {
				if ( unzGoToNextFile( fptr ) != UNZ_OK )
				{
					logfile( "cound not read next file\n" );
					unzClose( fptr );
					return MENU_HOME;
				}
			}
		}
		unzClose( fptr );
	}
	else {
		sprintf(tmpstr,"Downloading wiithememanagerimages.zip Failed .");
		__Draw_Message(tmpstr,0);
		sleep(3);
		
	}
	
	free(fatpath);
	free(tmpstr);
	return MENU_HOME;
}
const char *getdevicename(int index) {
	const char *return_name = NULL;
	
	switch(index)
	{
		case 0: return_name = "SD";
		break;
		case 1: return_name = "USB"; 
		break;
		default: return_name = "---";
		break;
	}
	return return_name;
}
#define DEVICE_X			        200
#define DEVICE_Y			        180
#define DEVICE_BUTTON_WIDTH		    220
#define DEVICE_BUTTON_HEIGHT		30
#define DEVICE_BUTTON_SEPARATION	50
void __Select_Device() {
	int i, hotSpot, hotSpotPrev, selected_device;
	bool repaint = true;
	//bool showinstructions = false;
	const char *dev[2] = { "SD", "USB" };
	
	// Create/restore hotspots
	Wpad_CleanHotSpots();
	for(i = 0; i < 2; i++){
		Wpad_AddHotSpot(i,
			DEVICE_X,
			DEVICE_Y + i*(DEVICE_BUTTON_HEIGHT+DEVICE_BUTTON_SEPARATION),
			DEVICE_BUTTON_WIDTH,
			DEVICE_BUTTON_HEIGHT,
			(i == 0 ? 1 : 0),
			(i == 1 ? 0 : 1),
			i, i
		);
	}
	
	// Loop
	hotSpot = hotSpotPrev = -1;

	selected_device = MENU_EXIT_HBC;
	for(;;) {
		hotSpot = Wpad_Scan();
		
			MRC_Draw_Texture(0, 0, textures[TEX_BACKGROUND]);
			sprintf(tempString, "Select %s Device :", (downloadable_theme_List == 1 ? "Save" : "Theme"));
			MRC_Draw_Box(94, 99, (strlen(tempString)*8) + 1, 17, WHITE);
			MRC_Draw_String(95, 100, BLACK, tempString);
			
			sprintf(tempString, "Device : %s", getdevicename(thememode));
			MRC_Draw_Box(((640-strlen(tempString)*8)/2 - 1), 429, (strlen(tempString)*8) + 2, 17, WHITE);
			MRC_Draw_String(((640-strlen(tempString)*8)/2), 430, BLACK, tempString);
			
			MRC_Draw_Box(94, 359, (strlen("[A] - Select Device  ")*8) + 1, 17, WHITE);
			MRC_Draw_String(95, 360, BLACK, "[A] - Select Device  ");
			
			sprintf(tempString, "IOS %d v%d", IOS_GetVersion(), IOS_GetRevision());
			MRC_Draw_Box(29, 429, (strlen(tempString)*8) + 1, 17, WHITE);
			MRC_Draw_String(30, 430, BLACK, tempString);
			
			sprintf(tempString, "System Menu v%s%s", getsysvernum(systemmenuversion), getregion(systemmenuversion));
			MRC_Draw_Box(459, 429, (strlen(tempString)*8) + 2, 17, WHITE);
			MRC_Draw_String(460, 430, BLACK, tempString);
		
		// If hot spot changed
		if(((hotSpot != hotSpotPrev) && (hotSpot < 2)) || repaint){
			hotSpotPrev = hotSpot;

			for(i = 0; i < 2; i++){
				__Draw_Button(i, dev[i], hotSpot == i);
			}
			//__Draw_Button(2, "?", hotSpot == 2);
			//MRC_Resize_Texture(textures[TEX_INFO], 50, 50);
			//MRC_Draw_Texture(570, 225, textures[TEX_INFO]);
			
			repaint = false;
		}
		MRC_Draw_Cursor(Wpad_GetWiimoteX(), Wpad_GetWiimoteY(), 0);
		

		if(((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_A) || (PAD_ButtonsDown(0) & PAD_BUTTON_A)) && hotSpot != -1) {
			
			if(hotSpot == 0) {
				selected_device = 0;
				break;
			}
			else if(hotSpot == 1){
				selected_device = 1;	
				break;
			}
		}
		else if(WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_HOME) {
			selected_device = MENU_EXIT_PRIILOADER;
			break;
		}
		else repaint = true;
	}
	if(thememode != -1) {
		if(thememode != selected_device)
			Fat_Unmount(thememode);
		else
			return;
	}
	if(selected_device > 1) {
		thememode = selected_device;
		return;
	}
	thememode = Fat_Mount(selected_device);
	//logfile("Wii Theme Manager Debugging Started .\n");
	//logfile("system menu version[%d] \n", systemmenuversion);
	//logfile("selected_device[%i]\n", selected_device);
	return;
}
const char *spinoptions(int input) {
	char *rtn;
	
	switch(input) {
		case 1:
			rtn = "nospin.mym";
		break;
		case 2:
			rtn = "spin.mym";
		break;
		case 3:
			rtn = "fastspin.mym";
		break;
		default:
			rtn = "UNKNOWN";
		break;
	}
	
	return rtn;
}
#define SPIN_BUTTON_X			210
#define SPIN_BUTTON_Y			180
#define SPIN_BUTTON_WIDTH		220
#define SPIN_BUTTON_HEIGHT		30
#define SPIN_BUTTON_SEPARATION	10
int __Spin_Question() {
	int i, hotSpot, hotSpotPrev, ret;
	bool repaint = true;

	// Create/restore hotspots
	Wpad_CleanHotSpots();
	for(i = 0; i < 3; i++){
		Wpad_AddHotSpot(i,
			SPIN_BUTTON_X,
			SPIN_BUTTON_Y+i*(SPIN_BUTTON_HEIGHT+SPIN_BUTTON_SEPARATION),
			SPIN_BUTTON_WIDTH,
			SPIN_BUTTON_HEIGHT,
			(i == 0 ? 2 : i - 1),
			(i == 2 ? 0 : i + 1),
			i, i
		);
	}

	__Draw_Window(SPIN_BUTTON_WIDTH+60, 170, "Channel Spin Option :");

	sprintf(tempString, "IOS_%d v_%d", IOS_GetVersion(), IOS_GetRevision());
	MRC_Draw_Box(25, 425, 120, 25, WHITE - 0x20);
	MRC_Draw_String(30, 430, BLACK, tempString);
	
	sprintf(tempString, "System_Menu v%s_%s", getsysvernum(systemmenuversion), getregion(systemmenuversion));
	MRC_Draw_Box(455, 425, 160, 25, WHITE - 0x20);
	MRC_Draw_String(460, 430, BLACK, tempString);
	
	// Loop
	hotSpot = hotSpotPrev = -1;

	ret = MENU_SELECT_THEME;
	for(;;){
		hotSpot = Wpad_Scan();

		// If hot spot changed
		if((hotSpot != hotSpotPrev && hotSpot < 3) || repaint){
			hotSpotPrev = hotSpot;

			__Draw_Button(0, "No Spin", hotSpot == 0);
			__Draw_Button(1, "Spin", hotSpot == 1);
			__Draw_Button(2, "Fast Spin", hotSpot == 2);
			repaint = false;
		}
		MRC_Draw_Cursor(Wpad_GetWiimoteX(), Wpad_GetWiimoteY(), 0);
		if(((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_A) || (PAD_ButtonsDown(0) & PAD_BUTTON_A)) && hotSpot > -1 && hotSpot < 3) {
			if(hotSpot == 0) ret = 1;
			else if(hotSpot == 1) ret = 2;
			else if(hotSpot == 2) ret = 3;
			break;
		}
		else if(WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_B || (PAD_ButtonsDown(0) & PAD_BUTTON_B)) break;
		else repaint = true;
	}

	return ret;
}
int __download_Theme() {
	__Draw_Loading();
	if(!netconnection) {
		if(debugcard) {
			logfile("Internet connection not detected .\n");
		}
		return MENU_SELECT_THEME;
	}
	
	char tmpstr[128];
	char sitepath[512];
	char mymfile[32];
	char version[16];
	char spinoption[32];
	char sessionId[64];
	char themedownloadlink[128];
	char themeposition[64];
	char sessionIdstr[128];
	char downloads_update[32];
	//char download_return_count[32];
	const char *siteUrl = "http://wiithemer.org/wii/index.php?action=";
	const char *actions[4] = { "prepDir", "copymymfiles", "downloadappfile", "buildtheme" }; // To Do add option to update wii download count and save on server 
	int ret = -2, i = 0;
	int theme_selected = orden[selectedtheme];
	logfile("selected = %d \n selectedtheme = %d %s\n", theme_selected, selectedtheme, ThemeList[orden[selectedtheme]].title);
	/*
	sprintf(tmpstr, "Connecting to the network .");
	__Draw_Message(tmpstr, 0);
	sleep(1);
	for(retries = 0;retries < 5; ++retries){
		__Draw_Loading();
		ret = net_init();
		if(ret < 0 && ret != -EAGAIN){
			sprintf(tmpstr, "ERROR = Connecting to the network Failed . returned(%d)", ret);
			__Draw_Message(tmpstr, 0);
			sleep(2);
		}
		if(ret == 0) break;
	}
	if(retries >= 5) return MENU_SELECT_THEME;
	
	sprintf(tmpstr, "Connecting to the network complete .");
	__Draw_Message(tmpstr, 0);
	sleep(1);
	*/
	switch(theme_selected) {
		case 20:
		case 21:
		case 22:
		case 23:
		case 24:
		case 25:
		case 26:
		case 27:
		case 47:
		case 117:
			sprintf(mymfile, "&mymfile=%s%s.mym", Theme_Mym_File[orden[selectedtheme]], getregion(systemmenuversion));
			break;
		default:
			sprintf(mymfile, "&mymfile=%s", Theme_Mym_File[orden[selectedtheme]]);
			break;
	}
	 
	logfile("mymfile = %s\n", mymfile);
	sprintf(version, "&version=%i", systemmenuversion);
	sprintf(spinoption, "&spinselected=%s", spinoptions(spinselected));
	sprintf(themeposition, "&selected=%d", theme_selected);
	
	u32 outlen = 0;
	u32 http_status = 0;
	u32 Maxsize = 4294967290;
	u8* outbuf = NULL;
	char *savename = NULL;
	//themedir = "modthemes";
	
	for(i = 0; i < 4; i++) {
		__Draw_Loading();
		if(debugcard) logfile("i(%d)\n", i);
		switch(i) {
			case 0:
				sprintf(sitepath, "%s%s", siteUrl, actions[i]);
				sprintf(tmpstr, "Requesting Server to Start Build Process .");
				__Draw_Message(tmpstr, 0);
			break;
			case 1:
				sprintf(sitepath, "%s%s%s%s%s", siteUrl, actions[i], mymfile, spinoption, sessionIdstr);
			break;
			case 2:
				sprintf(sitepath, "%s%s%s%s", siteUrl, actions[i], version, sessionIdstr);
				sprintf(tmpstr, "Downloading %s from System Menu %s_%s .", getappname(systemmenuversion), getsysvernum(systemmenuversion), getregion(systemmenuversion));
				__Draw_Message(tmpstr, 0);
			break;
			case 3:
				sprintf(sitepath, "%s%s%s%s%s%s%s", siteUrl, actions[i], mymfile, version, spinoption, sessionIdstr, themeposition);
				sprintf(tmpstr, "Server Building Theme .");
				__Draw_Message(tmpstr, 0);
			break;
		}
		if(debugcard) logfile("sitepath[%s]\n", sitepath);
		ret = http_request(sitepath, Maxsize);
		if(ret != 0 ) {
			ret = http_get_result(&http_status, &outbuf, &outlen);
			if(ret != 0 ) {
				char output[outlen];
				if(outlen > 0 && http_status == 200) {
					memcpy(output, outbuf, outlen);
					output[outlen] = 0;
					if(debugcard) logfile("%s\n", output);
					switch(i) {
						case 0:
							strcpy(sessionId, output);
							if(debugcard) logfile("id[%s\n", sessionId);
							sprintf(sessionIdstr, "&sessionId=%s", sessionId);
							sleep(1);
						break;
						case 1:
							sprintf(tmpstr, "Copying needed Files and Directory Complete .");
							__Draw_Message(tmpstr, 0);
							sleep(2);
						break;
						case 2:
							sprintf(tmpstr, "Downloading %s from System Menu %s_%s Complete .", getappname(systemmenuversion), getsysvernum(systemmenuversion), getregion(systemmenuversion));
							__Draw_Message(tmpstr, 0);
							sleep(2);
						break;
						case 3:
							strcpy(themedownloadlink, output);
							sprintf(tmpstr, "Server Building Theme Complete .");
							__Draw_Message(tmpstr, 0);
							sleep(2);
						break;
					}
				}
			}
		}
	}
	if(debugcard) logfile("link[%s]\n", themedownloadlink);
	sprintf(sitepath, "%s", themedownloadlink);
	sprintf(tmpstr, "Downloading Theme .");
	__Draw_Message(tmpstr, 0);
	if(debugcard) logfile("sitepath[%s]\n", sitepath);
	char savepath[128];
	ret = http_request(sitepath, Maxsize);
	if(ret != 0 ) {
		ret = http_get_result(&http_status, &outbuf, &outlen);
		if(outlen > 0 && http_status == 200) {
			char * token = strtok(themedownloadlink, "/");
			while(token != NULL) {
				token = strtok(NULL, "/");
				if(token != NULL)
				savename = token;
			}
			if(debugcard) logfile("savename = %s", savename);
			sprintf(savepath,"%s:/%s", getdevicename(thememode), themedir);
			
			if(!Fat_CheckDir(savepath))
				Fat_CreateSubfolder(savepath);
			//__Draw_Loading();
			sprintf(savepath,"%s:/%s/%s", getdevicename(thememode), themedir, savename);
			ret = Fat_SaveFile(savepath, (void *)&outbuf, outlen);
			sprintf(tmpstr, "Downloading Theme Complete .");
			__Draw_Message(tmpstr, 0);
			sleep(3);
		}
	}
	if(debugcard) logfile("\ndelete server session dir here .\n");
	sprintf(sitepath, "%s%s%s", siteUrl, "removesessionDir", sessionIdstr);
	if(debugcard) logfile("sitepath[%s]\n", sitepath);
	ret = http_request(sitepath, Maxsize);
	
	if(Fat_CheckFile(savepath)) {
		sprintf(downloads_update, "&downloadcount=%d", 1);
		sprintf(sitepath, "%s%s%s&themetoupdate=%s", siteUrl, "updatedownloadcount", downloads_update, DBThemelist_nospaces[orden[selectedtheme]]);
		ret = http_request(sitepath, Maxsize);
		if(debugcard) logfile("sitepath[%s] ret[%d]\n", sitepath, ret);
	}
	net_deinit();
	
	return MENU_HOME;
}
void Menu_Loop(){
	
	int ret = MENU_MANAGE_DEVICE;
	
	// Check if widescreen
	if(CONF_GetAspectRatio() == CONF_ASPECT_16_9)
		wideScreen = true;

	// Init MRC graphics
	MRC_Init();
	textures[1] = MRC_Load_Texture((void *)wiithememanager_background_png);
	textures[4] = MRC_Load_Texture((void *)wiithememanager_loading_png);
	
	MRC_Free_Texture(textures[1]);
	MRC_Free_Texture(textures[4]);
	// Load skin images*/
	__load_textures();
	
	ret = GetSysMenuVersion();
	if(ret > 610) ret = systemmenuversion = checkcustomsystemmenuversion();
	else systemmenuversion = ret;

	priiloadercheck = checkforpriiloader(is_vWii(systemmenuversion));
	if(!priiloadercheck) {
		priiloaderackknowledgement = __Question_Window("Priiloader not Detected", "Theme Installs are disabled . ", "Continue", "Exit");
		if(!priiloaderackknowledgement) ret = MENU_EXIT_HBC;
	}
	
	ret = MENU_MANAGE_DEVICE;
	for(;;) {
		__Draw_Loading();
		if(ret == MENU_SELECT_THEME) {
			
			ret = __Select_Theme();
		}
		else if(ret == MENU_SHOW_THEME) {
			ret=__Show_Theme();	
		}
		else if(ret == MENU_HOME) {
			ret = __Home();
			if(!netconnection) netconnection = checkNinit_netconnection();
			if(debugcard) {
				logfile("netconnection[%d] ret[%d]\n", netconnection, ret);
			}
		}
		else if(ret == MENU_MANAGE_DEVICE){
			__Select_Device();
			//printf("thememode[%i] after select device\n", thememode);
			if((thememode < 0) || (thememode == MENU_EXIT_PRIILOADER)) {
				ret = MENU_HOME;
				continue;
			}
			
			if(themecnt > 0) __Free_Channel_Images();
			themecnt = filelist_retrieve(downloadable_theme_List);
			//if(debugcard) logfile("themedir [%s]\n", themedir);
			__Load_Config();
			ret = MENU_SELECT_THEME;
		}
		else if(ret == MENU_DOWNLOAD_IMAGE) {
			netconnection = checkNinit_netconnection();
			ret = __Downloadthemepng();
		}
		else if(ret == MENU_ORIG_THEME) {
			
			ret = __downloadApp(1);
		}
		else if(ret == MENU_INSTALL_THEME) {
			if(!netconnection) netconnection = checkNinit_netconnection();
			if(downloadable_theme_List) {
				ret = __download_Theme();
			}
			else ret = __install_Theme();
		}
		else if(ret == MENU_EXIT_SYSTEMMENU) {
			__Draw_Message("Exiting to the System Menu ....", 0);
			if(thememode >= 0)
				Fat_Unmount(thememode);
			sleep(1);
			sys_loadmenu();
		}
		else if(ret == MENU_EXIT_HBC) {
			__Draw_Message("Exiting to HBC ....", 0);
			if(thememode >= 0)
				Fat_Unmount(thememode);
			sleep(1);
			sysHBC();
		}
		else if(ret == MENU_EXIT_PRIILOADER) {
			__Draw_Message("Exiting to PriiLoader ....", 0);
			if(thememode >= 0)
				Fat_Unmount(thememode);
			sleep(1);
			system_Exit_Priiloader();
		}
	}
	
	return;
}

