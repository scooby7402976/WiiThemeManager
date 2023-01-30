/* menu.c
 *
 * wiithememanager Wii theme installer based on the gui of mighty channels by scooby74029
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
#include <network.h> //for network
#include <sys/errno.h>
#include <sys/stat.h> //for mkdir
#include <sys/types.h>
#include <sys/dir.h>
#include <malloc.h>
#include <ogcsys.h>
#include <dirent.h>
#include <unistd.h>
#include <gctypes.h>
#include <stdarg.h>
//#include <include/c++/4.6.3/powerpc-eabi/bits/stdc++.h>

#include "tools.h"
#include "lz77.h"
#include "config.h"
#include "fat_mine.h"
#include "video.h"
#include "http.h"
#include "wpad.h"
#include "menu.h"
#include "rijndael.h"
#include "sys.h"
#include "themedatabase.h"
#include "http.h"
#include "fileops.h"

//Menu images
#include "wiithememanager_arrows_png.h"
#include "wiithememanager_background_png.h"
#include "wiithememanager_container_png.h"
#include "wiithememanager_container_wide_png.h"
#include "wiithememanager_empty_png.h"
#include "wiithememanager_loading_png.h"
#include "wiithememanager_numbers_png.h"

#define MAX_TEXTURES	6
#define TEX_ARROWS		0
#define TEX_BACKGROUND	1
#define TEX_CONTAINER	2
#define TEX_EMPTY		3
#define TEX_LOADING		4
#define TEX_NUMBERS		5
static MRCtex* textures[MAX_TEXTURES];

// Constants
#define ARROWS_X		        24
#define ARROWS_Y		        210
#define ARROWS_WIDTH	        20 
#define WIITHEMEMANAGER_PATH		    "config/wiithememanager/"
#define IMAGES_PREFIX		    "imgs"
#define WIITHEMEMANAGER_CONFIG_FILE	"wiithememanager.cfg"

#define EMPTY			-1

#define BLACK	0x000000FF
#define YELLOW	0xFFFF00FF
#define WHITE	0xFFFFFFFF
#define ORANGE	0xeab000ff
#define RED     0xFF0000FF

static s16* orden;
//static u32 Dbase = 0;
static int spinselected = -1;
static int thememode = 0;
static u32 themecnt = 0;
u8 commonkey[16] = { 0xeb, 0xe4, 0x2a, 0x22, 0x5e, 0x85, 0x93, 0xe4, 0x48,
     0xd9, 0xc5, 0x45, 0x73, 0x81, 0xaa, 0xf7
  };

// Variables for wiithememanager
static u16 pages, page, maxPages;
static int selectedtheme=0, movingGame=-1;
//static const char** lang;
static int loadingAnim=0;
static bool wideScreen=false;
static bool saveconfig=false;
static char tempString[256];
ModTheme ThemeList[MAXTHEMES];
u32 systemmenuversion;
bool foundneek;
CurthemeStats curthemestats;
//bool dbase;
//static FILE *fp;
//static char *outdir;
dirent_t *ent = NULL;
//Fatfile *themefile = NULL;
dirent_t *nandfilelist = NULL;
extern GXRModeObj *vmode;
extern u32* framebuffer;
bool availList = true;
u32 known_Versions[KNOWN_SYSTEMMENU_VERSIONS] = {416, 417, 418, 448, 449, 450, 454, 480, 481, 482, 486, 512, 513, 514, 518};
char *regions[KNOWN_SYSTEMMENU_VERSIONS] = {"J", "U", "E", "J", "U", "E", "K", "J", "U", "E", "K", "J", "U", "E", "K"};
char *knownappfilenames[KNOWN_SYSTEMMENU_VERSIONS] = {"00000070.app", "00000072.app", "00000075.app", "00000078.app", "0000007b.app", "0000007e.app", "00000081.app", "00000084.app", "00000087.app", "0000008a.app", "0000008d.app", "00000094.app", "00000097.app", "0000009a.app", "0000009d.app"};
char *appfilename[2] = { "Cetk", "Tmd" };

const u8 COLS[]={3, 4};
#define ROWS 3
const u8 FIRSTCOL[]={136, 110}; // 136 112
#define FIRSTROW 110
const u8 SEPARACIONX[]={180, 136};
#define SEPARACIONY 125
const u8 ANCHOIMAGEN[]={154, 116}; //116
#define ALTOIMAGEN 90

#define ALIGN32(x) (((x) + 31) & ~31)

const char *getregion(u32 num);
const char *getsysvernum(u32 num);
const char *getdevicename(int index);
int __Spin_Question(void);
void __Load_Config(void);
//int __DownloadDBfile(char *ThemeFile);
//int __DownloadDBpng();

void logfile(const char *format, ...) {
	
	char buffer[256];
	char path[256];
	va_list args;
	va_start (args, format);
	vsprintf (buffer,format, args);
	FILE *f = NULL;
	Fat_Mount(1);
	sprintf(path, "%s:/wiithememanager.log", getdevicename(1));
	f = fopen(path, "a");
	if (!f) {
		printf("Error writing log\n");
		return;
	}
	fputs(buffer, f);
	fclose(f);
	va_end (args);
	
	return;
}
void __Draw_Loading(void) {
 	MRC_Draw_Tile(300, 430, textures[TEX_LOADING], 24, loadingAnim);
	MRC_Render_Box(300, 430);

	loadingAnim += 1;
	if(loadingAnim == 16)
		loadingAnim = 0;
}

void __Draw_Page(int selected) {
	int i, j, x, y, containerWidth, theme;

	containerWidth=textures[TEX_CONTAINER]->width/2;

	// Background
	MRC_Draw_Texture(0, 0, textures[TEX_BACKGROUND]);
	
	sprintf(tempString, "IOS_%d v_%d", IOS_GetVersion(), IOS_GetRevision());
	MRC_Draw_Box(25, 425, 120, 25, WHITE - 0x20);
	MRC_Draw_String(30, 430, BLACK, tempString);
	
	sprintf(tempString, "System_Menu v%s_%s", getsysvernum(systemmenuversion), getregion(systemmenuversion));
	MRC_Draw_Box(450, 425, 160, 25, WHITE - 0x20);
	MRC_Draw_String(455, 430, BLACK, tempString);
	
	if(themecnt == 0 || pages == 0){
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
	MRC_Draw_Box(285, 425, 60, 25, WHITE - 0x20);
	MRC_Draw_String(290, 430, BLACK, tempString);
	/*x = (page + 1 < 10 ? 300 : 292);
	sprintf(tempString, "%d", page + 1);
	for(i = 0; i < strlen(tempString); i++){
		MRC_Draw_Tile(x, 420, textures[TEX_NUMBERS], 8, tempString[i]-48);
		x += 8;
	}
	MRC_Draw_Tile(, 420, textures[TEX_NUMBERS], 8, 10);
	x += 20;
	sprintf(tempString, "%d", maxPages);
	for(i = 0; i < strlen(tempString); i++){
		MRC_Draw_Tile(x, 420, textures[TEX_NUMBERS], 8, tempString[i]-48);
		x += 8;
	}*/
	//sprintf(tempString, "%s", "of");
	//MRC_Draw_String(x - 20, 4020, BLACK, tempString);
	//MRC_Draw_String(300, 404, 0x808080FF, tempString);
	//MRC_Draw_String(310, 404, 0xd0d0d0FF, tempString);
	//sprintf(tempString, "hotspot=%d", selected);
	//MRC_Draw_String(40, 444, 0xFFFF00FF, tempString);

	if(movingGame > -1){
		if(orden[movingGame]==EMPTY){
			MRC_Draw_Texture(Wpad_GetWiimoteX(), Wpad_GetWiimoteY(), textures[TEX_EMPTY]);
		}else{
			MRC_Draw_Texture(Wpad_GetWiimoteX(), Wpad_GetWiimoteY(), ThemeList[orden[movingGame]].banner);
		}
	}

	// Arrows
	MRC_Draw_Tile(ARROWS_X, ARROWS_Y, textures[TEX_ARROWS], ARROWS_WIDTH, 0+(page>0)+(page>0 && selected==HOTSPOT_LEFT));
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
	//MRC_Draw_Box(0, 0, 640, 480, BLACK);
	MRC_Draw_Texture(0, 0, textures[TEX_BACKGROUND]);
	MRC_Draw_Box(x, y, width, 32, YELLOW);
	MRC_Draw_Box(x, y+32, width, height, WHITE-0x20);

	MRC_Draw_String(x+(width-strlen(title)*8)/2, y+8, BLACK, title);
}

void __Draw_Message(const char* title, int ret) {
	int i;

	MRC_Draw_Texture(0, 0, textures[TEX_BACKGROUND]);
	sprintf(tempString, "IOS_%d v_%d", IOS_GetVersion(), IOS_GetRevision());
	MRC_Draw_String(100, 430, WHITE, tempString);
	
	sprintf(tempString, "System_Menu v%s_%s", getsysvernum(systemmenuversion), getregion(systemmenuversion));
	MRC_Draw_String(420, 430, WHITE, tempString);
	MRC_Draw_Box(0, 150, 640, 48, WHITE-0x20);
	for(i = 0;i < 16; i++){
		MRC_Draw_Box(0, 200-16+i, 640, 1, i*2);
		MRC_Draw_Box(0, 200+48+16-i, 640, 1, i*2);
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
#define QUESTION_BUTTON_SEPARATION	20
#define QUESTION_BUTTON_WIDTH		150
#define QUESTION_BUTTON_HEIGHT		40
int __Question_Window(const char* title, const char* text, const char* a1, const char* a2, const char* a3) {
	int i, hotSpot, hotSpotPrev;
	int ret=0, repaint=true;

	// Create/restore hotspots
	Wpad_CleanHotSpots();
	for(i=0; i<3; i++)
		Wpad_AddHotSpot(i,
			QUESTION_BUTTON_X+i*(QUESTION_BUTTON_WIDTH+QUESTION_BUTTON_SEPARATION),
			QUESTION_BUTTON_Y,
			QUESTION_BUTTON_WIDTH,
			QUESTION_BUTTON_HEIGHT,
			(i == 0? 2 : i - 1),
			(i == 3? 0 : i + 1),
			i, i
		);


	__Draw_Window(552, 128, title);
	MRC_Draw_String(100, 200, BLACK, text);

	// Loop
	hotSpot=hotSpotPrev=-1;

	
	for(;;){
		hotSpot = Wpad_Scan();

		// If hot spot changed
		if((hotSpot!=hotSpotPrev && hotSpot<4) || repaint){
			hotSpotPrev = hotSpot;

			__Draw_Button(0, a1, hotSpot==0);
			__Draw_Button(1, a2, hotSpot==1);
			__Draw_Button(2, a3, hotSpot==2);
			repaint=false;
		}
		MRC_Draw_Cursor(Wpad_GetWiimoteX(), Wpad_GetWiimoteY(), 0);

		if(((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_A) || (PAD_ButtonsDown(0) & PAD_BUTTON_A)) && hotSpot!=-1){
			if(hotSpot==0)
				ret=1;
			if(hotSpot==1)
				ret=2;
			if(hotSpot==2)
				ret=3;
			break;
		}
	}
	logfile("ret spin question [%i]\n", ret);
	return ret;
}


void findnumpages(void) {
	int i;
	maxPages = 0;
	for(i = themecnt-1; i > -1; i--){
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
	for(i=0; i<MAXTHEMES; i++){
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
u32 filelist_retrieve(bool avail_list) {
	//static char filelist[MAX_FILELIST_LEN];
    char dirpath[MAX_FILEPATH_LEN];
    //char dirpath2[ISFS_MAXPATH + 1];
    //char *ptr = filelist;
    //struct stat filestat;
    DIR *dir;
    //s32 start = 0;
	u32 fu, ff, cnt = 0;
	struct dirent *entry = NULL;
	
	//dirent_t *neeklist = NULL;
	//u32 neekcount;
	if(avail_list) {
		for(fu = 0; fu < MAXTHEMES; fu++){
			if(DBThemelist[fu] == NULL)
				break;
			cnt += 1;
		}
		for(ff = 0; ff < cnt; ff++){
			ThemeList[ff].title = DBThemelist[ff];
			ThemeList[ff].type = 10;
			//gprintf("theme =%s .type%d %d \n",ThemeList[ff].title, ThemeList[ff].type,ff);
		}
		logfile("cnt[%u]\n", cnt);
		goto end;
    }
	
	//Generate dirpath 

    
	sprintf(dirpath, "%s:/themes", getdevicename(thememode));
	logfile("dirpath[%s]\n", dirpath);
    /*else if(i == 4)  // usb(uneek nand)
    {
        sprintf(dirpath2, "/themes");
        gprintf("USBPathuneek: %s\n",dirpath2);
        rtn = getdir(dirpath2,&neeklist,&neekcount);
        if(rtn < 0)
        {
            gprintf("rtn getfilesuneek (%d) \n",rtn);
            sysHBC();
        }
        else
        {
            gprintf("going to end \n");
			for(x = 0;x < themecnt;x++){
				gprintf("neeklist[x].name(%s) \n",neeklist[x].name);
				
			}
        }
		goto end;
    }
	
	
     Open directory */

    dir = opendir(dirpath);
    if (!dir)
    {
        //gprintf("could not open dir(%s)\n",dirpath);
        return -1;
    }
	cnt = 0;
	//logfile("cnt[%u]\n", cnt);
    // Get directory entries 
    while((entry = readdir(dir))) // If we get EOF, the expression is 0 and
                                     // the loop stops. 
    {
		if(strncmp(entry->d_name, ".", 1) != 0 && strncmp(entry->d_name, "..", 2) != 0)
		cnt += 1;
    }
	//logfile("2-cnt2[%u]\n", cnt2);
	rewinddir(dir);
	ent = allocate_memory(sizeof(dirent_t) * cnt);
	cnt = 0;
	//logfile("3-cnt2[%u]\n", cnt2);
	while((entry = readdir(dir))) // If we get EOF, the expression is 0 and
                                     // the loop stops. 
    {
		if(strncmp(entry->d_name, ".", 1) != 0 && strncmp(entry->d_name, "..", 2) != 0){
			strcpy(ent[cnt].name, entry->d_name);//, sizeof(entry->d_name));
			ThemeList[cnt].title = ent[cnt].name;
			ThemeList[cnt].type = 20;
			//gprintf("theme =%s .type%d \n",ThemeList[themecnt].title, ThemeList[themecnt].type);
			cnt += 1;
			
		}
    }
	
end:	

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

	ret = Fat_ReadFile(WIITHEMEMANAGER_PATH WIITHEMEMANAGER_CONFIG_FILE, (void *)&archivoLeido);

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
	pages=0;
	findnumpages();
}

void __Free_Channel_Images(void) {
	int i;

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
}

void __Finish_ALL_GFX(void) {
	int i;

	__Free_Channel_Images();

	for(i=0; i<MAX_TEXTURES; i++){
		MRC_Free_Texture(textures[i]);
	}
	MRC_Finish();

	saveconfig=false;
}



void __Load_Images_From_Page(void) {
	void *imgBuffer=NULL;
	int i, max, ret, theme;

	//#ifdef DEBUG_MODE
	//gprintf("Loading images...\n");
	//#endif

	max=COLS[wideScreen]*ROWS;


	for(i=0; i<max; i++){
		theme=orden[max*pages+i];
		if(theme!=EMPTY){
			__Draw_Loading();

			// Load image from FAT
			if(ThemeList[theme].type == 20)
				sprintf(tempString,"%s:/config/wiithememanager/imgs/%s.png",getdevicename(thememode), ThemeList[theme].title);
			if(ThemeList[theme].type == 10)
				sprintf(tempString,"%s:/config/wiithememanager/imgs/%s", getdevicename(thememode), DBlistpng[theme]);
			ret=Fat_ReadFile(tempString, &imgBuffer);
			//gprintf("ret from fat read images %d\n",ret);
			
			// Decode image
			if(ret>0){
				ThemeList[theme].banner=MRC_Load_Texture(imgBuffer);
				free(imgBuffer);
			}
			else{
				ThemeList[theme].banner=__Create_No_Banner(ThemeList[theme].title, ANCHOIMAGEN[wideScreen], ALTOIMAGEN);
			}

			MRC_Resize_Texture(ThemeList[theme].banner, ANCHOIMAGEN[wideScreen], ALTOIMAGEN);
			__MaskBanner(ThemeList[theme].banner);
			MRC_Center_Texture(ThemeList[theme].banner, 1);
		}
		//MRC_Draw_Texture(64, 440, configuracionJuegos[theme].banner);
	}
	pages++;
}




void __Load_Skin_From_FAT(void) {
	const char* fileNames[MAX_TEXTURES]={
		"_arrows", "_background", (wideScreen? "_container_wide" : "_container"), "_empty",
		"_loading", "_numbers"};
	const u8* defaultTextures[MAX_TEXTURES]={
		wiithememanager_arrows_png, wiithememanager_background_png, (wideScreen? wiithememanager_container_wide_png : wiithememanager_container_png), wiithememanager_empty_png,
		wiithememanager_loading_png, wiithememanager_numbers_png};

	int i, ret;
	char *imgData = NULL;

	for(i=0; i<MAX_TEXTURES; i++){
		sprintf(tempString, WIITHEMEMANAGER_PATH "wiithememanager%s.png", fileNames[i]);
		ret = Fat_ReadFile(tempString, (void*)&imgData);
		if(ret>0){
			textures[i]=MRC_Load_Texture(imgData);
			free(imgData);
		}else{
			textures[i]=MRC_Load_Texture((void *)defaultTextures[i]);
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
	
	char * tite = (char *)memalign(32,256);
	sprintf(tite,"[+] Installing Menu Files !");
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
	logfile("length of file %d \n",length);
	numchunks = length/CHUNKS + ((length % CHUNKS != 0) ? 1 : 0);
	
	logfile("[+] Total parts: %d\n", numchunks);
	
	for(i = 0; i < numchunks; i++)
	{
		
		data = memalign(32, CHUNKS);
		if(data == NULL)
		{
			return -1;
		}
		char *ms = memalign(32,256);
		
		sprintf(ms,"Installing part %d",i+1);

		logfile("	Installing part %d\n",(i + 1) );

		__Draw_Message(ms,0);
		
		ret = fread(data, 1, CHUNKS, fp);
		if (ret < 0) 
		{
			logfile("[-] Error reading from SD! (ret = %d)\n\n", ret);
			
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
			logfile("[-] Error writing to NAND! (ret = %d)\n\n", ret);
			//gprintf("	Press any button to continue...\n");
			return ret;
		}
		free(data);
		wiilight(0);
	}
	
	ISFS_Close(nandfile);

	
	
	return 0;
}

bool installregion(u32 inputversion) {
	switch(inputversion) {
		case 416:
		case 448:
		case 480:
		case 512:
			ThemeList[orden[selectedtheme]].region = (u8*)74;
			break;
		case 417:
		case 449:
		case 481:
		case 513:
			ThemeList[orden[selectedtheme]].region = (u8*)85;
			break;
		case 418:
		case 450:
		case 482:
		case 514:
			ThemeList[orden[selectedtheme]].region = (u8*)69;
			break;
		case 454:
		case 486:
		case 518:
			ThemeList[orden[selectedtheme]].region = (u8*)75;
			break;
		default:
			ThemeList[orden[selectedtheme]].region = 0;
			return 0;
		break;
	}
	return 1;
}
u32 findinstallthemeversion(char * name) { 
	char filepath[256];
    FILE *fp = NULL;
    u32 length, i, rtn = 0;
    u8 *data;
	
	sprintf(filepath, "%s:/themes/%s", getdevicename(thememode), name);
    fp = fopen(filepath, "rb");
    if (!fp) {
        logfile("unable to open path\n");
		return 0;
	}
    length = filesize(fp);
    data = allocate_memory(length);
    memset(data,0,length);
    fread(data,1,length,fp);
	fclose(fp);
	
    if(length <= 0) {
        logfile("[-] Unable to read file !! \n");
		//logfile("[-] Unable to read file !! \n");
        return 0;
    }
    else {
        for(i = 0; i < length; i++)
        {
            if(data[i] == 83)
            {
                if(data[i+6] == 52)  // 4
                {
                    if(data[i+8] == 48)  // 0
                    {
                        if(data[i+28] == 85)  // usa
                        {
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
	}
	return rtn;
}
int __install_Theme() {  // install.app .csm file
	//gprintf("install theme start! \n");
	char filepath[1024];
	FILE *fp = NULL;
	//u8 *data;
	//u32 length;
	//int i;
	char *start = memalign(32,256);
	sprintf(start,"Starting Custom Theme Installation !");
	__Draw_Message(start,0);
	sleep(2);
	
	sprintf(filepath, "%s:/themes/%s", getdevicename(thememode), ThemeList[orden[selectedtheme]].title);
	logfile("filepath (%s) \n",filepath);
	curthemestats.version = GetSysMenuVersion();
	retreivecurrentthemeregion(curthemestats.version);
	logfile("cur theme .version(%d) .region(%c) \n",curthemestats.version, curthemestats.region);
	ThemeList[orden[selectedtheme]].version = findinstallthemeversion(ThemeList[orden[selectedtheme]].title);
    installregion(ThemeList[orden[selectedtheme]].version);
	logfile("install theme .version(%d) .region(%c) \n",ThemeList[orden[selectedtheme]].version,ThemeList[orden[selectedtheme]].region);
	
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
        logfile("[+] File Open Error not on SD!\n");
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
	return MENU_EXIT_TO_MENU;
}


int __Select_Theme(void){
	int i, j, hotSpot, hotSpotPrev, ret;
	ret = MENU_EXIT;

	if(themecnt == 0)
		return MENU_HOME;

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
	if(pages < page+1)
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

		if(hotSpot>=0 && hotSpot<=COLS[wideScreen]*ROWS){
			selectedtheme=page*COLS[wideScreen]*ROWS+hotSpot;
		}
		else{
			selectedtheme=-1;
		}

		MRC_Draw_Cursor(Wpad_GetWiimoteX(), Wpad_GetWiimoteY(), (movingGame>-1));

		if(movingGame==-1){
			if(((WPAD_ButtonsDown(WPAD_CHAN_0) & (WPAD_BUTTON_A | WPAD_BUTTON_B | WPAD_BUTTON_1)) || (PAD_ButtonsDown(0) & (PAD_BUTTON_A | PAD_TRIGGER_Z))) && hotSpot>-1 && hotSpot<COLS[wideScreen]*ROWS && orden[selectedtheme]!=EMPTY){
				if(WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_B){
					movingGame=selectedtheme;
					findnumpages();
				}else if((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_A) || (PAD_ButtonsDown(0) & PAD_BUTTON_A)){
					ret=MENU_SHOW_THEME;
					break;
				}
			}
		}
		else{
			//moving game
			if(!(WPAD_ButtonsHeld(WPAD_CHAN_0) & WPAD_BUTTON_B)){
				if(selectedtheme!=-1 && selectedtheme!=movingGame){
					u16 copia0=orden[movingGame];
					u16 copia1=orden[selectedtheme];

					orden[movingGame]=copia1;
					orden[selectedtheme]=copia0;
					saveconfig=true;
				}
				movingGame=-1;
				findnumpages();
				__Draw_Page(hotSpot);
			}
		}

		if(WPAD_ButtonsDown(WPAD_CHAN_0) || PAD_ButtonsDown(0)){
			if(page>=0 && (((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_MINUS) || \
			(PAD_ButtonsDown(0) & PAD_TRIGGER_L)) || (hotSpot==HOTSPOT_LEFT && \
			((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_A) || (PAD_ButtonsDown(0) \
			& PAD_BUTTON_A))))){
				//gprintf("page = %d maxpages = %d pages = %d\n",page,maxPages,pages);
				if (page == 0)
					page = maxPages;
				
				page-=1;
				ret=MENU_SELECT_THEME;
				break;
			}else if(page<maxPages && (((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_PLUS) || (PAD_ButtonsDown(0) & PAD_TRIGGER_R)) || (hotSpot==HOTSPOT_RIGHT && ((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_A) || (PAD_ButtonsDown(0) & PAD_BUTTON_A))))){
				page+=1;
				if (page >= maxPages)
					page = 0;
				ret=MENU_SELECT_THEME;
				break;
			}else if(((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_HOME) || (PAD_ButtonsDown(0) & PAD_BUTTON_START))){
				ret=MENU_HOME;
				break;
			}else if((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_B) || (PAD_ButtonsDown(0) & PAD_BUTTON_B)){
				ret=MENU_SELECT_THEME;
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
    case 289:
        return "42";// usa
        break;
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
    case 290:
        return "45";// pal
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
    case 288:
        return "40";// jpn
        break;
    case 416:
        return "70";
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
    case 2:
        return "http://nus.cdn.shop.wii.com/ccs/download/0000000100000002/0000003f";
        break;
    case 3:
        return "http://nus.cdn.shop.wii.com/ccs/download/0000000100000002/00000042";
        break;
    case 4:
        return "http://nus.cdn.shop.wii.com/ccs/download/0000000100000002/00000045";
        break;
    case 5:
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
    case 288:
        return 2;
        break;
    case 289:
        return 3;
        break;
    case 290:
        return 4;
        break;
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
    u32 tmpversion;
    int ret, retries;
    int counter;
    char *savepath = (char*)memalign(32,256);
	char *tmpstr = (char*)malloc(256);
	char *tmpstr2 = (char*)malloc(256);
	signed_blob * s_tik = NULL;
    signed_blob * s_tmd = NULL;
		
	__Draw_Loading();
    tmpversion = GetSysMenuVersion();
    if(tmpversion > 518) tmpversion = checkcustomsystemmenuversion();
	sprintf(tmpstr2,"%s:/themes/%s", getdevicename(thememode), getappname(tmpversion));
	if(!Fat_CheckFile(tmpstr2)){
		sprintf(tmpstr,"Downloading %s for System Menu v%u ", getappname(tmpversion), tmpversion);
		__Draw_Message(tmpstr, 0);
		sleep(1);
		__Draw_Loading();
		sprintf(tmpstr,"%s:/themes", getdevicename(thememode));
		Fat_MakeDir(tmpstr);
		sprintf(tmpstr,"Initializing  Network ....");
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
		}
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
					ret = Fat_SaveFile(savepath, (void *)&outbuf2, outlen);
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
	
end:

	//free(oldapp);
	free(savepath);
	free(tmpstr);
	free(tmpstr2);
	
	return MENU_SELECT_THEME;
}


#define PROJECTION_HEIGHT 64
int __Show_Theme(){
	void* imageBuffer;
	MRCtex *themeImage, *projection;
	int i, j, ret;
	char *c, *r, a;
	ModTheme *thetheme=&ThemeList[orden[selectedtheme]];
	
	// BLACK SCREEN
	/*a=160;
	for(i=0; i<480; i++){
		if(a<255 && ((i<208 && i%4==0) || i%8==0))
			a++;
		MRC_Draw_Box(0, i, 640, 1, a);
	}*/

	
	// ANOTHER SCREEN FADE TYPE
	a=200;
	for(i=0; i<480; i++){
		if(a<255 && ((i<100 && i%4==0) || (i>200 && i%8==0)))
			a++;
		MRC_Draw_Box(0, i, 640, 1, 0x20202000+a);
	}


	// Load image from FAT
	if(ThemeList[selectedtheme].type == 20)
		sprintf(tempString,"%s:/" WIITHEMEMANAGER_PATH IMAGES_PREFIX "/%s.png",getdevicename(thememode) , thetheme->title);
	if(ThemeList[selectedtheme].type == 10)
		sprintf(tempString,"%s:/config/wiithememanager/imgs/%s" ,getdevicename(thememode),DBlistpng[orden[selectedtheme]]);
		
	//gprintf("tempstring %s \n",tempString);
	ret=Fat_ReadFile(tempString, &imageBuffer);
	// Decode image
	if(ret>0){
		themeImage=MRC_Load_Texture(imageBuffer);
		free(imageBuffer);

		MRC_Resize_Texture(themeImage, (wideScreen? 580 : 640), 340);
		//__MaskBanner(themeImage);
		//MRC_Center_Texture(themeImage, 1);

		projection=allocate_memory(sizeof(MRCtex));
		projection->buffer=allocate_memory(themeImage->width*PROJECTION_HEIGHT*4);
		projection->width=themeImage->width;
		projection->height=PROJECTION_HEIGHT;
		MRC_Center_Texture(projection, 1);
		projection->alpha=true;
	
		a=128;
		r=(projection->buffer);
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
	
		MRC_Draw_Texture(40, 40, themeImage);
		//MRC_Draw_Texture(175, 275, projection);
		MRC_Draw_String((640-strlen(thetheme->title))/2, 400, WHITE, thetheme->title);
		//MRC_Draw_String(30, 330, WHITE, "By ");
		sprintf(tempString, "%s", (availList == 1 ? "[A] - Download Theme" : "[A] - Install Theme"));
		MRC_Draw_String(40, 400, WHITE, tempString);
		MRC_Draw_String(560, 400, WHITE, "[B]-Back");
		MRC_Free_Texture(themeImage);
		MRC_Free_Texture(projection);
	}
	else{
		sprintf(tempString, "%s", (availList == 1 ? "[A] - Download Theme" : "[A] - Install Theme"));
		MRC_Draw_String(30, 360, WHITE, tempString);
		MRC_Draw_String(30, 390, WHITE, "[B]-Back");
		//MRC_Draw_String(30, 360, WHITE, "By ");
		MRC_Draw_String((640-strlen(thetheme->title)*8)/2, 250, WHITE, thetheme->title);
	}
	MRC_Render_Screen();
	int answer = -1;
	
	ret=MENU_SELECT_THEME;
	for(;;){
		WPAD_ScanPads();
		PAD_ScanPads();
		if(WPAD_ButtonsDown(WPAD_CHAN_0) || PAD_ButtonsDown(0)){
			if((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_A) || (PAD_ButtonsDown(0) & PAD_BUTTON_A)){
				if(availList) {
					answer = __Spin_Question();
					logfile("answer[%i]\n", answer);
				}
				ret = MENU_INSTALL_THEME;
				break;
			}
			if((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_B) || (PAD_ButtonsDown(0) & PAD_BUTTON_B)){
				ret = MENU_SELECT_THEME;
				break;
			}
		}
		
	}
	spinselected = answer;
	return ret;
}


#define HOME_BUTTON_X			208
#define HOME_BUTTON_Y			120
#define HOME_BUTTON_WIDTH		224
#define HOME_BUTTON_HEIGHT		40
#define HOME_BUTTON_SEPARATION	8
int __Home(void) {
	int i, hotSpot, hotSpotPrev, ret;
	bool repaint=true;

	// Create/restore hotspots
	Wpad_CleanHotSpots();
	for(i=0; i<5; i++){
		Wpad_AddHotSpot(i,
			HOME_BUTTON_X,
			100+i*(HOME_BUTTON_HEIGHT+HOME_BUTTON_SEPARATION),
			HOME_BUTTON_WIDTH,
			HOME_BUTTON_HEIGHT,
			(i==0? 5 : i-1),
			(i==6? 0 : i+1),
			i, i
		);
	}


	__Draw_Window(HOME_BUTTON_WIDTH+44, 320, "Options");

	sprintf(tempString, "IOS_%d v_%d", IOS_GetVersion(), IOS_GetRevision());
	MRC_Draw_String(250, 380, BLACK, tempString);
	//MRC_Draw_String(10, 440, 0x505050ff, lang[9+thememode]);
	//sprintf(tempString, "wiithememanager v_%d - by Scooby74029", wiithememanager_VERSION);
	//MRC_Draw_String(210, 350, BLACK, tempString);

	// Loop
	hotSpot=hotSpotPrev=-1;

	ret=MENU_SELECT_THEME;
	for(;;){
		hotSpot=Wpad_Scan();

		// If hot spot changed
		if((hotSpot!=hotSpotPrev && hotSpot<6) || repaint){
			hotSpotPrev=hotSpot;

			__Draw_Button(0, "Device Menu", hotSpot == 0);
			__Draw_Button(1, "Download Original Theme", hotSpot == 1);
			if(availList) __Draw_Button(2, "Install Theme", hotSpot == 2);
			else __Draw_Button(2, "Download Theme", hotSpot == 2);
			__Draw_Button(3, "Exit <HBC>", hotSpot == 3);
			__Draw_Button(4, "Exit <Sys-Menu>", hotSpot == 4);
			repaint = false;
		}
		MRC_Draw_Cursor(Wpad_GetWiimoteX(), Wpad_GetWiimoteY(), 0);
		//gprintf("hotSpot = %d \n",hotSpot);
		if(((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_A) || (PAD_ButtonsDown(0) & PAD_BUTTON_A)) && hotSpot>-1 && hotSpot<5){
			if(hotSpot == 0)
				ret = MENU_MANAGE_DEVICE;
			else if(hotSpot == 1)
				ret = MENU_ORIG_THEME;
			else if(hotSpot == 2) {
				availList ^= 1;
				ret = MENU_MANAGE_DEVICE;
			}
			else if(hotSpot == 3)
				ret = MENU_EXIT;
			else if(hotSpot == 4)
				ret = MENU_EXIT_TO_MENU;
			break;
		}else if(((WPAD_ButtonsDown(WPAD_CHAN_0) & (WPAD_BUTTON_HOME | WPAD_BUTTON_B)) || (PAD_ButtonsDown(0) & (PAD_BUTTON_START | PAD_BUTTON_B)))){
			ret = MENU_SELECT_THEME;
			break;
		}else{
			repaint = true;
		}
	}

	return ret;
}
/*int __DownloadDBpng(){
	gprintf("In download db png \n");
	char *fatpath = (char*)malloc(256);
	char *tmpstr = (char*)malloc(256);
	int i,ret, error = 0;
	
	sprintf(tmpstr,"Starting Theme DataBase preview download ....");
	__Draw_Message(tmpstr,0);
	sleep(2);
	
	gprintf("thememode = %d \n",thememode);
	sprintf(tmpstr,"%s:/config/wiithememanager/DBimages",getdevicename(thememode));
	if(!Fat_CheckFile(tmpstr))
		Fat_MakeDir(tmpstr);
	
	if(!CheckFile(tmpstr))
		CreateSubfolder(tmpstr);
		
	int retries = 0;
	
	for(;;){
		__Draw_Loading();
		ret=net_init();
		if(ret<0 && ret!=-EAGAIN){
			

			sprintf(tempString, "ERROR: I can't connect to network . (returned %d)\n",ret);
			__Draw_Message(tempString, 0);
			sleep(2);
			retries+=1;
		}
		if(ret==0) //consigo conexion
			break;
		if(retries == 5){
			sleep(2);
			goto end;
		}
	}
	
	//gprintf("themecnt = %d \n",themecnt);
	__Draw_Loading();
	for(i = 0;i < themecnt;i++){
		__Draw_Loading();
		sprintf(tmpstr,"Downloading %d of %d Theme Previews .",i+1,themecnt);
		__Draw_Message(tmpstr,0);
		sleep(1);
		__Draw_Loading();
		sprintf(tempString, "http://bartlesvillok-am.com/downloads/wiithememanager/%s",DBlistpng[i]);
		//gprintf("tempString(%s) \n",tempString);
		sprintf(fatpath,"%s:/config/wiithememanager/imgs/%s", getdevicename(thememode), DBlistpng[i]);
		//gprintf("fatpath(%s) \n",fatpath);
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
			//	gprintf("download failed !! ret(%d)\n",ret);
				sprintf(tmpstr,"Download %d of %d Theme Previews Failed !",i+1,themecnt);
				__Draw_Message(tmpstr,0);
				sleep(3);
				error+=1;
				goto end2;
				//continue;
			}
			//else
			//	gprintf("Complete !! \n\n");
			__Draw_Loading();
			ret = http_get_result(&http_status, &outbuf, &outlen); 
			
			if(outlen>64){//suficientes bytes
				sprintf(fatpath,"%s:/config/wiithememanager/DBimages/%s", getdevicename(thememode), DBlistpng[i]);
			//	gprintf("fatpath(%s) \n",fatpath);
				ret = Fat_SaveFile(fatpath, (void *)&outbuf,outlen);
				if(ret < 0){
				sprintf(tmpstr,"Saving Preview Falied !");
				__Draw_Message(tmpstr,0);
				sleep(2);
				error+=1;
				//continue;
				}
				//else
				//	gprintf("File Saved \n");
			}
			__Draw_Loading();	
			if(outbuf!=NULL)
				free(outbuf);
			__Draw_Loading();	
		}
	end2:
		__Draw_Loading();
		if(DBlistpng[i] == 0){
			//gprintf("db list[i] null \n");
			break;
		}
		__Draw_Loading();
		if(i == 40){
			//gprintf("i = %d\n",i);
			break;
		}
		__Draw_Loading();
	}
	
	net_deinit();

	gprintf("Exiting dbpng \n");
	sprintf(tmpstr,"Theme DataBase preview download .... Complete .");
	__Draw_Message(tmpstr,0);
	sleep(3);
	if(error == 0)
		return 0;
	
end:
	sprintf(tmpstr,"One or more Theme Previews was not downloaded .");
	__Draw_Message(tmpstr,0);
	sleep(2);
	sprintf(tmpstr,"Please retry download from home button menu .");
	__Draw_Message(tmpstr,0);
	
	return 0;
}*/
const char *getdevicename(int index) {
	switch(index)
	{
		case 1: return "sd";
		break;
		case 2: return "usb"; 
		break;
		default: return "ukn";
	}
}
/*bool Comparefolders(const char *src, const char *dest){
    if(!src || !dest)
        return false;

    char *folder1 = strchr(src, ':');
    char *folder2 = strchr(dest, ':');

	if(!folder1 || !folder2)
        return false;

	int position1 = folder1-src+1;
	int position2 = folder2-dest+1;

	char temp1[50];
	char temp2[50];

	snprintf(temp1, position1, "%s", src);
	snprintf(temp2, position2, "%s", dest);

    if(strcasecmp(temp1, temp2) == 0)
        return true;

    return false;
}


void write_file(void* data, size_t size, char* name){
	FILE *out;
	out = fopen(name, "wb");
	fwrite(data, 1, size, out);
	fclose(out);	
}
int __Select_Database_save(void){
	int i, hotSpot, hotSpotPrev, ret;
	bool repaint=true;
	
	const char *langs[4] = {
		"Sd Card",
		"Usb Device",
		"Neek NAND",
	};
	
	// Create/restore hotspots
	Wpad_CleanHotSpots();
	for(i=0; i<3; i++){
		Wpad_AddHotSpot(i,
			HOME_BUTTON_X,
			180+i*(HOME_BUTTON_HEIGHT+HOME_BUTTON_SEPARATION),
			HOME_BUTTON_WIDTH,
			HOME_BUTTON_HEIGHT,
			(i==0? 3 : i-1),
			(i==3? 0 : i+1),
			i, i
		);
	}

	// Background
	MRC_Draw_Texture(0, 0, textures[TEX_BACKGROUND]);

	MRC_Draw_String(95, 100, BLACK, "Select Theme Database Save Device :");
	//MRC_Draw_String(95, 130, BLACK, "Theme Database file will be saved here also .");
	MRC_Draw_String(95, 360, BLACK, "[A]-Select");//[HOME/start]-Options/Exit");
	
	sprintf(tempString, "IOS_%d v_%d", IOS_GetVersion(), IOS_GetRevision());
	MRC_Draw_String(95, 430, WHITE, tempString);
	
	sprintf(tempString, "System_Menu v%s_%s", getsysvernum(systemmenuversion), getregion(systemmenuversion));
	MRC_Draw_String(420, 430, WHITE, tempString);

	// Loop
	hotSpot=hotSpotPrev=-1;

	ret=0;
	for(;;){
		hotSpot=Wpad_Scan();

		// If hot spot changed
		if((hotSpot!=hotSpotPrev && hotSpot<4) || repaint){
			hotSpotPrev=hotSpot;

			for(i=0; i<3; i++){
				__Draw_Button(i, langs[i], hotSpot==i);
			}
			repaint=false;
		}
		MRC_Draw_Cursor(Wpad_GetWiimoteX(), Wpad_GetWiimoteY(), 0);

		if(((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_A) || (PAD_ButtonsDown(0) & PAD_BUTTON_A))){
			if(hotSpot==0)
				ret=1;
			else if(hotSpot==1)
				ret=2;
			else if(hotSpot==2)
				ret=3;
			Dbase = ret;
			gprintf("Dbase(%d) \n", Dbase);
			break;
		}
		
	}
	return ret;
}*/
#define DEVICE_X			        200
#define DEVICE_Y			        170
#define DEVICE_WIDTH		        224
#define DEVICE_BUTTON_HEIGHT		40
#define DEVICE_BUTTON_SEPARATION	40
int __Select_Device(void) {
	int i, hotSpot, hotSpotPrev, ret;
	bool repaint=true;
	
	const char *langs[2] = { "Sd", "Usb" };
	
	// Create/restore hotspots
	Wpad_CleanHotSpots();
	for(i = 0; i < 2; i++){
		Wpad_AddHotSpot(i,
			DEVICE_X,
			DEVICE_Y + i*(DEVICE_BUTTON_HEIGHT+DEVICE_BUTTON_SEPARATION),
			DEVICE_WIDTH,
			DEVICE_BUTTON_HEIGHT,
			(i == 0 ? 1 : i - 1),
			(i == 2 ? 0 : i + 1),
			i, i
		);
	}

	// Background
	MRC_Draw_Texture(0, 0, textures[TEX_BACKGROUND]);
	sprintf(tempString, "Select %s Device :", (availList == 1 ? "Save" : "Theme"));
	MRC_Draw_Box(90, 95, 170, 25, WHITE - 0x20);
	MRC_Draw_String(95, 100, BLACK, tempString);
	
	sprintf(tempString, "[A] - Select %s Device :", (availList == 1 ? "Save" : "Theme"));
	MRC_Draw_Box(90, 355, 170, 25, WHITE - 0x20);
	MRC_Draw_String(95, 360, BLACK, "[A] - Select Device  ");
	
	sprintf(tempString, "IOS_%d v_%d", IOS_GetVersion(), IOS_GetRevision());
	MRC_Draw_Box(25, 425, 120, 25, WHITE - 0x20);
	MRC_Draw_String(30, 430, BLACK, tempString);
	
	sprintf(tempString, "System_Menu v%s_%s", getsysvernum(systemmenuversion), getregion(systemmenuversion));
	MRC_Draw_Box(450, 425, 160, 25, WHITE - 0x20);
	MRC_Draw_String(455, 430, BLACK, tempString);

	// Loop
	hotSpot = hotSpotPrev = -1;

	ret = 0;
	for(;;){
		hotSpot = Wpad_Scan();

		// If hot spot changed
		if(((hotSpot != hotSpotPrev) && (hotSpot < 3)) || repaint){
			hotSpotPrev = hotSpot;

			for(i = 0; i < 2; i++){
				__Draw_Button(i, langs[i], hotSpot == i);
			}
			repaint=false;
		}
		MRC_Draw_Cursor(Wpad_GetWiimoteX(), Wpad_GetWiimoteY(), 0);

		if(((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_A) || (PAD_ButtonsDown(0) & PAD_BUTTON_A))){
			if(hotSpot==0)
				ret=1;
			else if(hotSpot==1)
				ret=2;	
			break;
		}
	}
	return ret;
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
#define SPIN_BUTTON_Y			170
#define SPIN_BUTTON_WIDTH		220
#define SPIN_BUTTON_HEIGHT		40
#define SPIN_BUTTON_SEPARATION	10
int __Spin_Question(void) {
	int i, hotSpot, hotSpotPrev, ret;
	bool repaint=true;

	// Create/restore hotspots
	Wpad_CleanHotSpots();
	for(i = 0; i < 3; i++){
		Wpad_AddHotSpot(i,
			SPIN_BUTTON_X,
			SPIN_BUTTON_Y+i*(SPIN_BUTTON_HEIGHT+SPIN_BUTTON_SEPARATION),
			SPIN_BUTTON_WIDTH,
			SPIN_BUTTON_HEIGHT,
			(i == 0 ? 2 : i - 1),
			(i == 3 ? 0 : i + 1),
			i, i
		);
	}


	__Draw_Window(SPIN_BUTTON_WIDTH+60, 220, "Channel Spin Option :");

	sprintf(tempString, "IOS_%d v_%d", IOS_GetVersion(), IOS_GetRevision());
	MRC_Draw_Box(25, 425, 120, 25, WHITE - 0x20);
	MRC_Draw_String(30, 430, BLACK, tempString);
	
	sprintf(tempString, "System_Menu v%s_%s", getsysvernum(systemmenuversion), getregion(systemmenuversion));
	MRC_Draw_Box(450, 425, 160, 25, WHITE - 0x20);
	MRC_Draw_String(455, 430, BLACK, tempString);
	//MRC_Draw_String(10, 440, 0x505050ff, lang[9+thememode]);
	//sprintf(tempString, "wiithememanager v_%d - by Scooby74029", wiithememanager_VERSION);
	//MRC_Draw_String(210, 350, BLACK, tempString);

	// Loop
	hotSpot = hotSpotPrev = -1;

	ret = 1;
	for(;;){
		hotSpot=Wpad_Scan();

		// If hot spot changed
		if((hotSpot!=hotSpotPrev && hotSpot<4) || repaint){
			hotSpotPrev=hotSpot;

			__Draw_Button(0, "No Spin", hotSpot == 0);
			__Draw_Button(1, "Spin", hotSpot == 1);
			__Draw_Button(2, "Fast Spin", hotSpot == 2);
			repaint = false;
		}
		MRC_Draw_Cursor(Wpad_GetWiimoteX(), Wpad_GetWiimoteY(), 0);
		if(((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_A) || (PAD_ButtonsDown(0) & PAD_BUTTON_A)) && hotSpot>-1 && hotSpot<3){
			if(hotSpot == 0) ret = 1;
			else if(hotSpot == 1) ret = 2;
			else if(hotSpot == 2) ret = 3;
			break;
		}
		else{
			repaint = true;
		}
	}

	return ret;
}
int __download_Theme() {
	char tmpstr[128];
	char sitepath[256];
	char mymfile[32];
	char version[16];
	char spinoption[32];
	char sessionId[64];
	char themedownloadlink[128];
	const char *siteUrl = "http://bartlesvilleok-am.com/wiithemer/wii/index.php?action=";
	const char *actions[4] = { "prepDir", "copymymfiles", "downloadappfile", "buildtheme" }; // To Do add option to update wii download count and save on server 
	int retries, ret = -2, i = 0;
	
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
	
	if(orden[selectedtheme] > 6 && orden[selectedtheme] < 15) {
		sprintf(mymfile, "&mymfile=%s%s.mym", DBThemelistDL[orden[selectedtheme]], getregion(systemmenuversion));
	}
	else sprintf(mymfile, "&mymfile=%s", DBThemelistDL[orden[selectedtheme]]);
	sprintf(version, "&version=%i", systemmenuversion);
	sprintf(spinoption, "&spinselected=%s", spinoptions(spinselected));
	
	u32 outlen = 0;
	u32 http_status = 0;
	u32 Maxsize = 4294967295;
	u8* outbuf = NULL;
	char *savename = NULL;
	
	for(i = 0; i < 4; i++) {
		__Draw_Loading();
		logfile("i(%d)\n", i);
		switch(i) {
			case 0:
				sprintf(sitepath, "%s%s", siteUrl, actions[i]);
				sprintf(tmpstr, "Requesting Server to Start Build Process .");
				__Draw_Message(tmpstr, 0);
			break;
			case 1:
				sprintf(sitepath, "%s%s%s%s%s%s", siteUrl, actions[i], mymfile, spinoption, "&sessionId=", sessionId);
			break;
			case 2:
				sprintf(sitepath, "%s%s%s%s%s", siteUrl, actions[i], version, "&sessionId=", sessionId);
				sprintf(tmpstr, "Downloading %s from System Menu %s_%s .", getappname(systemmenuversion), getsysvernum(systemmenuversion), getregion(systemmenuversion));
				__Draw_Message(tmpstr, 0);
			break;
			case 3:
				sprintf(sitepath, "%s%s%s%s%s%s%s", siteUrl, actions[i], mymfile, version, spinoption, "&sessionId=", sessionId);
				sprintf(tmpstr, "Server Building Theme .");
				__Draw_Message(tmpstr, 0);
			break;
		}
		logfile("sitepath[%s]\n", sitepath);
		ret = http_request(sitepath, Maxsize);
		if(ret != 0 ) {
			ret = http_get_result(&http_status, &outbuf, &outlen);
			if(ret != 0 ) {
				char output[outlen];
				if(outlen > 0 && http_status == 200) {
					memcpy(output, outbuf, outlen);
					output[outlen] = 0;
					logfile("%s\n", output);
					switch(i) {
						case 0:
							strcpy(sessionId, output);
							logfile("id[%s\n", sessionId);
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
	logfile("link[%s]\n", themedownloadlink);
	sprintf(sitepath, "%s", themedownloadlink);
	sprintf(tmpstr, "Downloading Theme .");
	__Draw_Message(tmpstr, 0);
	logfile("sitepath[%s]\n", sitepath);
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
			logfile("savename = %s", savename);
			sprintf(savepath,"%s:/themes/%s", getdevicename(thememode), savename);
			__Draw_Loading();
			
			ret = Fat_SaveFile(savepath, (void *)&outbuf, outlen);
			sprintf(tmpstr, "Downloading Theme Complete . Choose 'Install Theme' at next screen .");
			__Draw_Message(tmpstr, 0);
			sleep(3);
		}
	}
	if(Fat_CheckFile(savepath)) {
		logfile("\ndelete server session dir here .\n");
		sprintf(sitepath, "%s%s%s", siteUrl, "removesessionDir&sessionId=", sessionId);
		logfile("sitepath[%s]\n", sitepath);
		ret = http_request(sitepath, Maxsize);
	}
	return MENU_HOME;
}
int Menu_Loop(){
	
	int ret = 0;
	char tmpstr[256];

	// Check if widescreen
	if(CONF_GetAspectRatio() == CONF_ASPECT_16_9)
		wideScreen = true;

	// Init MRC graphics
	MRC_Init();
	textures[1] = MRC_Load_Texture((void *)wiithememanager_background_png);
	textures[4] = MRC_Load_Texture((void *)wiithememanager_loading_png);
	
	systemmenuversion = GetSysMenuVersion();
	if(systemmenuversion > 518) systemmenuversion = checkcustomsystemmenuversion();
	
	thememode = __Select_Device();
	if(Fat_Mount(thememode) < 0){
		logfile("fat not mounted %d \n", thememode);
	}
	else
		logfile("fat mounted %d \n", thememode);
	logfile("systemmenuversion(%d) \n", systemmenuversion);
	sprintf(tmpstr,"%s:/config/wiithememanager", getdevicename(thememode));
	logfile("tmpstr = %s \n", tmpstr);
	if(!Fat_CheckFile(tmpstr)) {
		CreateSubfolder(tmpstr);
	}
	ret = ISFS_Initialize();
	logfile("ret isfs init = %d\n", ret);
	
	themecnt = filelist_retrieve(availList);
	// Load skin images
	MRC_Free_Texture(textures[1]);
	MRC_Free_Texture(textures[4]);
	__Load_Skin_From_FAT();

	// Load config file
	__Load_Config();
	
	ret = MENU_SELECT_THEME;
	for(;;){
		if(ret == MENU_SELECT_THEME)
			ret = __Select_Theme();
		else if(ret == MENU_SHOW_THEME)
			ret=__Show_Theme();	
		else if(ret == MENU_HOME)
			ret = __Home();
		else if(ret == MENU_MANAGE_DEVICE){
			thememode =  __Select_Device();
			if(Fat_Mount(thememode) < 0){
				__Draw_Message("Unable to Detect Sd Card . Press any button to Exit .", thememode);
			}
			else
				logfile("fat mounted %d availlist[%d]\n", thememode, availList);
			themecnt = filelist_retrieve(availList);
			__Load_Config();
			ret = MENU_SELECT_THEME;
		}
		else if(ret == MENU_ORIG_THEME)
			ret = __downloadApp(1);
		else if(ret == MENU_INSTALL_THEME) {
			if(availList) ret = __download_Theme();
			else ret = __install_Theme();
		}
		else if((ret == MENU_EXIT) || (ret == MENU_EXIT_TO_MENU))
			break;
	}
	
	if(ret == MENU_EXIT_TO_MENU) __Draw_Message("Exiting to the System Menu ....", 0);
	if(ret == MENU_EXIT) __Draw_Message("Exiting to HBC ....", 0);
	sleep(1);
	__Finish_ALL_GFX();
	Fat_Unmount();
	if(ret == MENU_EXIT_TO_MENU) SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
	return sysHBC();// exit(0);
}

