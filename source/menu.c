/* menu.c
 *
 * ThemeWii Wii theme installer based on the gui of mighty channels by scooby74029
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

#include "tools.h"
#include "lz77.h"
#include "fat_mine.h"
#include "video.h"
#include "http.h"
#include "langs.h"
#include "wpad.h"
#include "menu.h"
#include "gecko.h"
#include "rijndael.h"
#include "sys.h"
#include "http.h"
#include "miniunz.h"
#include "unzip.h"
#include "fileops.h"
#include "tools2.h"
#include "runtimeiospatch.h"
#include "theme_make.h"


//Menu images
#include "themewii_arrows_png.h"
#include "themewii_background_png.h"
#include "themewii_container_preview_png.h"
#include "themewii_container_preview_wide_png.h"
#include "themewii_empty_png.h"
#include "themewii_loading_png.h"
#include "themewii_numbers_png.h"

#define MAX_TEXTURES	6
#define TEX_ARROWS		0
#define TEX_BACKGROUND	1
#define TEX_CONTAINER	2
#define TEX_EMPTY		3
#define TEX_LOADING		4
#define TEX_NUMBERS		5
static MRCtex* textures[MAX_TEXTURES];


// Constants
#define ARROWS_X		         24
#define ARROWS_Y		         210
#define ARROWS_WIDTH	         20 
#define THEMEWII_PATH		    "config/themewii/"
#define IMAGES_PREFIX		    "images"
#define THEMEWII_CONFIG_FILE	    "themewii.cfg"

#define EMPTY			-1

#define BLACK	 0x000000FF
#define YELLOW	 0xFFFF00FF
#define WHITE	 0xFFFFFFFF
#define ORANGE	 0xeab000ff
#define RED     0xFF0000FF

#define KNVERSIONS    18

u8 commonkey[16] = { 0xeb, 0xe4, 0x2a, 0x22, 0x5e, 0x85, 0x93, 0xe4, 0x48,
     0xd9, 0xc5, 0x45, 0x73, 0x81, 0xaa, 0xf7
  };

// Variables for themewii
static int Selected_Theme = 0;
static int Selected_Theme_Preview_Page = 0;
static const char** lang;
static int loadingAnim = 0;
static bool wideScreen = false;
static bool saveconfig = false;
static char tempString[255];
static bool pageLoaded[MAXTHEMES];

static FILE *fp;
static char *outdir;
char spinner_chars[] = "/-\\|";
int spin = 0;
void spinner()
{
	printf("\b%c", spinner_chars[spin++]);
	if(!spinner_chars[spin])
		spin = 0;
}
extern GXRModeObj *vmode;
extern u32* framebuffer;

#define ALIGN32(x) (((x) + 31) & ~31)

const u8 COLS[] = {2, 2};
#define ROWS 2
const u8 FIRSTCOL[] = {190, 190};
#define FIRSTROW 110
const u8 SEPARACIONX[] = {250, 250};
#define SEPARACIONY 200
const u8 ANCHOIMAGEN[] = {172, 172}; // image width
#define ALTOIMAGEN 132 // image heigth

typedef struct {
  u16 type;
  u16 name_offset;
  u32 data_offset; // == absolut offset från U.8- headerns början
  u32 size; // last included file num for directories
} U8_node;
 
typedef struct{
  u32 tag; // 0x55AA382D "U.8-"
  u32 rootnode_offset; // offset to root_node, always 0x20.
  u32 header_size; // size of header from root_node to end of string table.
  u32 data_offset; // offset to data -- this is rootnode_offset + header_size, aligned to 0x40.
  u8 zeroes[16];
}U8_archive_header;

char *getregion(u32 num);
char *getsysvernum(u32 num);
void __Load_Images_From_Page();

const char *getdevicename(int index){
	switch(index)
	{
		case 0: return "sd";
		break;
		case 1: return "usb";
		//case 3: return "neek"; 
		break;
		default: return "ukn";
	}
}
const char *getdevicedisplayname(int index){
	switch(index)
	{
		case 0: return "SD";
		break;
		case 1: return "USB";
		//case 3: return "neek"; 
		break;
		default: return "ukn";
	}
}
s32 __FileCmp(const void *a, const void *b){
	dirent_t *hdr1 = (dirent_t *)a;
	dirent_t *hdr2 = (dirent_t *)b;
	
	if (hdr1->type == hdr2->type){
		return strcmp(hdr1->WorkingName, hdr2->WorkingName);
	}else{
		return 0;
	}
}

s32 getdir(char *path, dirent_t **ent, u32 *cnt){
	s32 res;
	u32 num = 0;

	int i, j, k;
	
	res = ISFS_ReadDir(path, NULL, &num);
	if(res != ISFS_OK){
		printf("Error: could not get dir entry count! (result: %ld)\n", res);
		return -1;
	}

	char ebuf[ISFS_MAXPATH + 1];

	char *nbuf = (char *)allocate_memory((ISFS_MAXPATH + 1) * num);
	if(nbuf == NULL){
		printf("ERROR: could not allocate buffer for name list!\n");
		return -2;
	}

	res = ISFS_ReadDir(path, nbuf, &num);
	DCFlushRange(nbuf,13*num); //quick fix for cache problems?
	if(res != ISFS_OK){
		printf("ERROR: could not get name list! (result: %ld)\n", res);
		free(nbuf);
		return -3;
	}
	
	*cnt = num;
	
	*ent = allocate_memory(sizeof(dirent_t) * num);
	if(*ent==NULL){
		printf("Error: could not allocate buffer\n");
		free(nbuf);
		return -4;
	}

	for(i = 0, k = 0; i < num; i++){	    
		for(j = 0; nbuf[k] != 0; j++, k++)
			ebuf[j] = nbuf[k];
		ebuf[j] = 0;
		k++;

		strcpy((*ent)[i].WorkingName, ebuf);
	}
	
	qsort(*ent, *cnt, sizeof(dirent_t), __FileCmp);
	
	free(nbuf);
	return 0;
}


void __Draw_Loading(void){
 	MRC_Draw_Tile(600,390, textures[TEX_LOADING], 24, loadingAnim);
	MRC_Render_Box(600,390);

	loadingAnim+=1;
	if(loadingAnim==16)
		loadingAnim=0;
}
void __Draw_Page(int selected){
	/*
	int i, j, x, y, containerWidth, theme;

	containerWidth = textures[TEX_CONTAINER]->width/2;

	// Background
	MRC_Draw_Texture(0, 0, textures[TEX_BACKGROUND]);
	
	if(themecnt==0 || !pageLoaded[page]) {
		return;
	}
	
	sprintf(tempString, "IOS_%ld v_%ld", IOS_GetVersion(), IOS_GetRevision());
	MRC_Draw_String(100, 430, WHITE, tempString);
	
	sprintf(tempString, "System_Menu v%s_%s", getsysvernum(Versionsys), getregion(Versionsys));
	MRC_Draw_String(420, 430, WHITE, tempString);
	
	// themes
	theme = COLS[wideScreen]*ROWS*page;
	y = FIRSTROW;
	for(i=0; i < ROWS; i++) {
		x = FIRSTCOL[wideScreen];
		for(j = 0; j < COLS[wideScreen]; j++) {
			if(movingGame != theme) {
				if(orden[theme] == EMPTY){
					MRC_Draw_Texture(x, y, textures[TEX_EMPTY]);
					MRC_Draw_Tile(x, y, textures[TEX_CONTAINER], containerWidth, 0);
				}else if(selected==i*COLS[wideScreen]+j){
					MRC_Draw_Texture(x, y, themelist[orden[theme]].banner);
					MRC_Draw_Tile(x, y, textures[TEX_CONTAINER], containerWidth, 1);
				}else{
					MRC_Draw_Texture(x, y, themelist[orden[theme]].banner);
					MRC_Draw_Tile(x, y, textures[TEX_CONTAINER], containerWidth, 0);
				}
			}
			theme++;
			x += SEPARACIONX[wideScreen];
		}
		y += SEPARACIONY;
	}

	// Page number
	x=(page+1<10? 300 : 292);
	sprintf(tempString, "%d", page+1);
	for(i=0; i<strlen(tempString); i++) {
		MRC_Draw_Tile(x, 404, textures[TEX_NUMBERS], 8, tempString[i]-48);
		x+=8;
	}
	MRC_Draw_Tile(x, 404, textures[TEX_NUMBERS], 8, 10);
	x+=10;
	sprintf(tempString, "%d", maxPages);
	for(i=0; i<strlen(tempString); i++) {
		MRC_Draw_Tile(x, 404, textures[TEX_NUMBERS], 8, tempString[i]-48);
		x+=8;
	}
	//MRC_Draw_String(300, 404, 0x808080FF, tempString);
	//MRC_Draw_String(310, 404, 0xd0d0d0FF, tempString);
	//sprintf(tempString, "hotspot=%d", selected);
	//MRC_Draw_String(40, 444, 0xFFFF00FF, tempString);

	if(movingGame > -1) {
		if(orden[movingGame] == EMPTY){
			MRC_Draw_Texture(Wpad_GetWiimoteX(), Wpad_GetWiimoteY(), textures[TEX_EMPTY]);
		}
		else {
			MRC_Draw_Texture(Wpad_GetWiimoteX(), Wpad_GetWiimoteY(), themelist[orden[movingGame]].banner);
		}
	}

	// Arrows
	MRC_Draw_Tile(ARROWS_X, ARROWS_Y, textures[TEX_ARROWS], ARROWS_WIDTH, 0+(page>0)+(page>0 && selected==HOTSPOT_LEFT));
	MRC_Draw_Tile(640-ARROWS_X, ARROWS_Y, textures[TEX_ARROWS], ARROWS_WIDTH, 3+(page+1<maxPages)+(page+1<maxPages && selected==HOTSPOT_RIGHT));
*/
	int i, j, x, y, containerWidth, theme, page;

	containerWidth = textures[TEX_CONTAINER]->width/2;

	// Background
	MRC_Draw_Texture(0, 0, textures[TEX_BACKGROUND]);
	
	if(!pageLoaded[Selected_Theme]) {
		return;
	}
	//if(selected == -1)
	//	selected = 0;
		
	
	sprintf(tempString, "IOS_%ld v_%ld", IOS_GetVersion(), IOS_GetRevision());
	MRC_Draw_String(50, 430, WHITE, tempString);
	
	sprintf(tempString, "%s","ThemeWii");
	MRC_Draw_String(265, 430, WHITE, tempString);
	
	sprintf(tempString, "System_Menu v%s_%s", getsysvernum(Current_System_Menu_Version), getregion(Current_System_Menu_Version));
	MRC_Draw_String(430, 430, WHITE, tempString);
	
	y = FIRSTROW;
	for(i = 0; i < ROWS; i++) {
		x = FIRSTCOL[wideScreen];
		for(j = 0; j < COLS[wideScreen]; j++) {
			theme = i * COLS[wideScreen] + j;
			switch(theme) {
				case 0:
					gprintf("\n\ncase 0 \n\n");
					MRC_Draw_Texture(x, y, themelist[Selected_Theme].preview_banner1);
					gprintf("\n\ncase 0 \n\n");
				break;
				case 1:
					MRC_Draw_Texture(x, y, themelist[Selected_Theme].preview_banner2);
				break;
				case 2:
					MRC_Draw_Texture(x, y, themelist[Selected_Theme].preview_banner3);
				break;
				case 3:
					MRC_Draw_Texture(x, y, themelist[Selected_Theme].preview_banner4);
				break;
				default:
				
				break;
			}
			if(selected == i * COLS[wideScreen] + j ) {	
			//	gprintf("\n\ndraw tile selected - theme[%i] i[%i]\n\n", theme, i);
				MRC_Draw_Tile(x, y, textures[TEX_CONTAINER], containerWidth, 1);
			}
			else {
			//	gprintf("\n\ndraw tile else - theme[%i] i[%i] \n\n", theme, i);		
				MRC_Draw_Tile(x, y, textures[TEX_CONTAINER], containerWidth, 0);
			}
			x += SEPARACIONX[wideScreen];
		}
		y += SEPARACIONY;
	}
	//themename
	page = Selected_Theme + 1;
	MRC_Draw_String(50, 400, BLACK, themelist[Selected_Theme].DisplayName);
	sprintf(tempString, "%2i/%li", page, themecnt);
	MRC_Draw_String(550, 400, BLACK, tempString);
	
	// Arrows
	MRC_Draw_Tile(ARROWS_X, ARROWS_Y, textures[TEX_ARROWS], ARROWS_WIDTH, 
		0 + (Selected_Theme > 0) + (Selected_Theme > 0 && selected == HOTSPOT_LEFT));
	MRC_Draw_Tile(640-ARROWS_X, ARROWS_Y, textures[TEX_ARROWS], ARROWS_WIDTH,
		3 + (Selected_Theme + 1 < themecnt) + (Selected_Theme + 1 < themecnt && selected == HOTSPOT_RIGHT));
	
	return;
}

void __Draw_Button(int hot, const char* text, bool selected){
	hotSpot button=Wpad_GetHotSpotInfo(hot);
	int textX=button.x+(button.width-strlen(text)*8)/2;
	u32 color;
	if(selected){
		MRC_Draw_Box(button.x, button.y, button.width, button.height/2, 0x3c7291ff);
		MRC_Draw_Box(button.x, button.y+button.height/2, button.width, button.height/2, 0x2d6483ff);
		color=WHITE;
	}else{
		MRC_Draw_Box(button.x, button.y, button.width, button.height/2, 0xe3e3e3ff);
		MRC_Draw_Box(button.x, button.y+button.height/2, button.width, button.height/2, 0xd8d8d8ff);
		MRC_Draw_Box(button.x, button.y, button.width, 1, 0xb6b4c5ff);
		MRC_Draw_Box(button.x, button.y+button.height-1, button.width, 1, 0xb6b4c5ff);
		MRC_Draw_Box(button.x, button.y, 1, button.height, 0xb6b4c5ff);
		MRC_Draw_Box(button.x+button.width-1, button.y, 1, button.height, 0xb6b4c5ff);
		color=0x404040ff;
	}
	MRC_Draw_String(textX, button.y+button.height/2-8, color, text);
}

void __Draw_Window(int width, int height, const char* title){
	int x=(640-width)/2;
	int y=(480-height)/2-32;
	__Draw_Page(-1);
	MRC_Draw_Box(0, 0, 640, 480, BLACK);

	MRC_Draw_Box(x, y, width, 32, YELLOW);
	MRC_Draw_Box(x, y+32, width, height, WHITE-0x20);

	MRC_Draw_String(x+(width-strlen(title)*8)/2, y+8, BLACK, title);
}

void __Draw_Message(const char* title, int ret){
	int i;

	MRC_Draw_Texture(0, 0, textures[TEX_BACKGROUND]);
	sprintf(tempString, "IOS_%ld v_%ld", IOS_GetVersion(), IOS_GetRevision());
	MRC_Draw_String(100, 430, WHITE, tempString);
	
	sprintf(tempString, "System_Menu v%s_%s", getsysvernum(Current_System_Menu_Version), getregion(Current_System_Menu_Version));
	MRC_Draw_String(420, 430, WHITE, tempString);
	MRC_Draw_Box(0, 150, 640, 48, WHITE-0x20);
	for(i=0;i<16;i++){
		MRC_Draw_Box(0, 200-16+i, 640, 1, i*2);
		MRC_Draw_Box(0, 200+48+16-i, 640, 1, i*2);
	}
	MRC_Draw_String((640-strlen(title)*8)/2, 165, BLACK, title);

	if(ret<0){
		sprintf(tempString, "ret=%d", ret);
		MRC_Draw_String(540, 216, 0xff0000ff, tempString);
	}

	MRC_Render_Screen();
	
	if(ret!=0)
		Wpad_WaitButtons();
}
#define QUESTION_BUTTON_X			100
#define QUESTION_BUTTON_Y			240
#define QUESTION_BUTTON_SEPARATION	     32
#define QUESTION_BUTTON_WIDTH		     200
#define QUESTION_BUTTON_HEIGHT		48
bool __Question_Window(const char* title, const char* text, const char* a1, const char* a2){
	int i, hotSpot, hotSpotPrev;
	bool ret=false, repaint=true;

	// Create/restore hotspots
	Wpad_CleanHotSpots();
	for(i=0; i<2; i++)
		Wpad_AddHotSpot(i,
			QUESTION_BUTTON_X+i*(QUESTION_BUTTON_WIDTH+QUESTION_BUTTON_SEPARATION),
			QUESTION_BUTTON_Y,
			QUESTION_BUTTON_WIDTH,
			QUESTION_BUTTON_HEIGHT,
			i, i,
			!i, !i
		);


	__Draw_Window(512, 128, title);
	MRC_Draw_String(100, 200, BLACK, text);

	// Loop
	hotSpot=hotSpotPrev=-1;

	ret=MENU_SELECT_THEME;
	for(;;){
		hotSpot=Wpad_Scan();

		// If hot spot changed
		if((hotSpot!=hotSpotPrev && hotSpot<2) || repaint){
			hotSpotPrev=hotSpot;

			__Draw_Button(0, a1, hotSpot==0);
			__Draw_Button(1, a2, hotSpot==1);

			repaint=false;
		}
		MRC_Draw_Cursor(Wpad_GetWiimoteX(), Wpad_GetWiimoteY(), 0);

		if(((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_A) || (PAD_ButtonsDown(0) & PAD_BUTTON_A)) && hotSpot!=-1){
			if(hotSpot==0)
				ret=true;
			break;
		}
	}

	return ret;
}
char *Sversion(u32 num){
	switch(num){
		case 0: return "00000042";// usa
		break;
		case 1: return "00000072";
		break;
		case 2: return "0000007b";
		break;
		case 3: return "00000087";
		break;
		case 4: return "00000097";// usa
		break;
		case 5: return "00000045";// pal
		break;
		case 6: return "00000075";
		break;
		case 7: return "0000007e";
		break;
		case 8: return "0000008a";
		break;
		case 9: return "0000009a";// pal
		break;
		case 10: return "00000040";// jpn
		break;
		case 11: return "00000070";
		break;
		case 12: return "00000078";
		break;
		case 13: return "00000084";
		break;
		case 14: return "00000094";// jpn
		break;
		case 15: return "0000008d";// kor
		break;
		case 16: return "00000081";
		break;
		case 17: return "0000009d";// kor
		break;
		default: return "UNK";
		break;
	}
}

char *getapplist(int j){
	switch(j){
		case 1: return "00000042";// usa
		break;
		case 2: return "00000072";
		break;
		case 3: return "0000007b";
		break;
		case 4: return "00000087";
		break;
		case 5: return "00000097";// usa
		break;
		case 6: return "00000045";// pal
		break;
		case 7: return "00000075";
		break;
		case 8: return "0000007e";
		break;
		case 9: return "0000008a";
		break;
		case 10: return "0000009a";// pal
		break;
		case 11: return "00000040";// jpn
		break;
		case 12: return "00000070";
		break;
		case 13: return "00000078";
		break;
		case 14: return "00000084";
		break;
		case 15: return "00000094";// jpn
		break;
		case 16: return "0000008d";// kor
		break;
		case 17: return "00000081";
		break;
		case 18: return "0000009d";// kor
		break;
		default: return "UNK";
		break;
	}
}
char *getappname(u32 Versionsys){
	switch(Versionsys){
		case 289: return "00000042";// usa
		break;
		case 417: return "00000072";
		break;
		case 449: return "0000007b";
		break;
		case 481: return "00000087";
		break;
		case 513: return "00000097";// usa
		break;
		case 290: return "00000045";// pal
		break;
		case 418: return "00000075";
		break;
		case 450: return "0000007e";
		break;
		case 482: return "0000008a";
		break;
		case 514: return "0000009a";// pal
		break;
		case 288: return "00000040";// jpn
		break;
		case 416: return "00000070";
		break;
		case 448: return "00000078";
		break;
		case 480: return "00000084";
		break;
		case 512: return "00000094";// jpn
		break;
		case 486: return "0000008d";// kor
		break;
		case 454: return "00000081";
		break;
		case 518: return "0000009d";// kor
		break;
		default: return "UNK";
		break;
	}
}

char *Sversion2(u32 num){
	switch(num){
		case 0:{
			//curthemestats.version = 32;
			//curthemestats.region = (u8*)85;
			return "00000042.app";// usa
		}
		break;
		case 1:{
			//curthemestats.version = 40;
			//curthemestats.region = (u8*)85;
			return "00000072.app";
		}
		break;
		case 2:{
			//curthemestats.version = 41;
			//curthemestats.region = (u8*)85;
			return "0000007b.app";
		}
		break;
		case 3:{
			//curthemestats.version = 42;
			//curthemestats.region = (u8*)85;
			return "00000087.app";
		}
		break;
		case 4:{
			//curthemestats.version = 43;
			//curthemestats.region = (u8*)85;
			return "00000097.app";// usa
		}
		break;
		case 5:{
			//curthemestats.version = 32;
			//curthemestats.region = (u8*)69;
			return "00000045.app";// pal
		}
		break;
		case 6:{
			//curthemestats.version = 40;
			//curthemestats.region = (u8*)69;
			return "00000075.app";
		}
		break;
		case 7:{
			//curthemestats.version = 41;
			//curthemestats.region = (u8*)69;
			return "0000007e.app";
		}
		break;
		case 8:{
			//curthemestats.version = 42;
			//curthemestats.region = (u8*)69;
			return "0000008a.app";
		}
		break;
		case 9:{
			//curthemestats.version = 43;
			//curthemestats.region = (u8*)69;
			return "0000009a.app";// pal
		}
		break;
		case 10:{
			//curthemestats.version = 32;
			//curthemestats.region = (u8*)74;
			return "00000040.app";// jpn
		}
		break;
		case 11:{
			//curthemestats.version = 40;
			//curthemestats.region = (u8*)74;
			return "00000070.app";
		}
		break;
		case 12:{
			//curthemestats.version = 41;
			//curthemestats.region = (u8*)74;
			return "00000078.app";
		}
		break;
		case 13:{
			//curthemestats.version = 42;
			//curthemestats.region = (u8*)74;
			return "00000084.app";
		}
		break;
		case 14:{
			//curthemestats.version = 43;
			//curthemestats.region = (u8*)74;
			return "00000094.app";// jpn
		}
		break;
		case 15:{
			//curthemestats.version = 41;
			//curthemestats.region = (u8*)75;
			return "0000008d.app";// kor
		}
		break;
		case 16:{
			//curthemestats.version = 42;
			//curthemestats.region = (u8*)75;
			return "00000081.app";
		}
		break;
		case 17:{
			//curthemestats.version = 43;
			//curthemestats.region = (u8*)75;
			return "0000009d.app";// kor
		}
		break;
		default: return "UNK";
		break;
	}
}



u32 checkcustom(u32 m){
	s32 rtn;
	u32 nandfilecnt,j;
	char *CHECK;
	dirent_t *filelist;
			
		rtn = getdir("/title/00000001/00000002/content",&filelist,&nandfilecnt);
		gprintf("rtn(%d) filecnt =%d \n",rtn,nandfilecnt);
		int k;
		for(k = 0;k <nandfilecnt ;k++){
			gprintf("name = %s k=%d \n",filelist[k].WorkingName,k);
			for(j = 0;j < KNVERSIONS;j++){
				CHECK = Sversion2(j);
				gprintf("check =%s j = %d\n",CHECK,j);
				if(strcmp(filelist[k].WorkingName,CHECK) != 0)
					continue;
				else{
					gprintf("equal j = %d \n",j);
					gprintf("check = %s \n",CHECK);
					switch(j){
						case 0: return 289;
						break;
						case 1: return 417;
						break;
						case 2: return 449;
						break;
						case 3: return 481;
						break;
						case 4: return 513;
						break;
						case 5: return 290;
						break;
						case 6: return 418;
						break;
						case 7: return 450;
						break;
						case 8: return 482;
						break;
						case 9: return 514;
						break;
						case 10: return 288;
						break;
						case 11: return 416;
						break;
						case 12: return 448;
						break;
						case 13: return 480;
						break;
						case 14: return 512;
						break;
						case 15: return 454;
						break;
						case 16: return 486;
						break;
						case 17: return 518;
						break;
						
					}
				}
			}
			if(k >= 6)
				break;
		}
	return 0;
}
	
bool checknandapp(){
	gprintf("check nandapp():\n");
	switch(Current_System_Menu_Version)
	{
		case 288:{
			curthemestats.version = 32;
			curthemestats.region = (u8*)74;
		}
		break;
		case 289:{
			curthemestats.version = 32;
			curthemestats.region = (u8*)85;
		}
		break;
		case 290:{
			curthemestats.version = 32;
			curthemestats.region = (u8*)69;
		}
		break;
		case 416:{
			curthemestats.version = 40;
			curthemestats.region = (u8*)74;
		}
		break;
		case 417:{
			curthemestats.version = 40;
			curthemestats.region = (u8*)85;
		}
		break;
		case 418:{
			curthemestats.version = 40;
			curthemestats.region = (u8*)69;

		}
		break;
		case 448:{
			curthemestats.version = 41;
			curthemestats.region = (u8*)74;
		}
		break;
		case 449:{
			curthemestats.version = 41;
			curthemestats.region = (u8*)85;
		}
		break;
		case 450:{
			curthemestats.version = 41;
			curthemestats.region = (u8*)69;
		}
		break;
		case 454:{
			curthemestats.version = 41;
			curthemestats.region = (u8*)75;
		}
		break;
		case 480:{
			curthemestats.version = 42;
			curthemestats.region = (u8*)74;
		}
		break;
		case 481:{
			curthemestats.version = 42;
			curthemestats.region = (u8*)85;
		}
		break;
		case 482:{
			curthemestats.version = 42;
			curthemestats.region = (u8*)69;
		}
		break;
		case 486:{
			curthemestats.version = 42;
			curthemestats.region = (u8*)75;
		}
		break;
		case 512:{
			curthemestats.version = 43;
			curthemestats.region = (u8*)74;
		}
		break;
		case 513:{
			curthemestats.version = 43;
			curthemestats.region = (u8*)85;
		}
		break;
		case 514:{
			curthemestats.version = 43;
			curthemestats.region = (u8*)69;
		}
		break;
		case 518:{
			curthemestats.version = 43;
			curthemestats.region = (u8*)75;
		}
		break;
		default:
			gprintf("default case going to custom sysmenu \n");
			return 0;
		break;
	}
	//gprintf("cur theme .region(%c)  .version(%d) \n",curthemestats.region,curthemestats.version);
	
	return 1;
}
char *getdownloadregion(u32 num){
    switch(num)
    {
    case 289:
    case 417:
    case 449:
    case 481:
    case 513:
        return "U";
        break;
    case 290:
    case 418:
    case 450:
    case 482:
    case 514:
        return "E";
        break;
    case 288:
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
        return "UNK";
        break;
    }
}
char *getregion(u32 num){
    switch(num)
    {
    case 289:
    case 417:
    case 449:
    case 481:
    case 513:
        return "Usa";
        break;
    case 290:
    case 418:
    case 450:
    case 482:
    case 514:
        return "Pal";
        break;
    case 288:
    case 416:
    case 448:
    case 480:
    case 512:
        return "Jpn";
        break;
    case 486:
    case 454:
    case 518:
        return "Kor";
        break;
    default:
        return "UNK";
        break;
    }
}
char *getsysvernum(u32 num){
    switch(num)
    {
    case 288:
    case 289:
    case 290:
        return "3.2";
        break;
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
        return "UNK";
        break;
    }
}


u32 GetSysMenuVersion(){
    //Get sysversion from TMD
    u64 TitleID = 0x0000000100000002LL;
    u32 tmd_size;
    s32 r = ES_GetTMDViewSize(TitleID, &tmd_size);
    if(r<0)
    {
        gprintf("error getting TMD views Size. error %d\n",r);
        return 0;
    }

    tmd_view *rTMD = (tmd_view*)memalign( 32, (tmd_size+31)&(~31) );
    if( rTMD == NULL )
    {
        gprintf("error making memory for tmd views\n");
        return 0;
    }
    memset(rTMD,0, (tmd_size+31)&(~31) );
    r = ES_GetTMDView(TitleID, (u8*)rTMD, tmd_size);
    if(r<0)
    {
        gprintf("error getting TMD views. error %d\n",r);
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

int getcurrentregion(){
	int ret = 0;
	
	if(curthemestats.region == (u8*)69)
		ret = 1;
	else if(curthemestats.region == (u8*)74)
		ret = 2;
	else if(curthemestats.region == (u8*)75)
		ret = 3;
	else if(curthemestats.region == (u8*)85)
		ret = 4;
	else 
		ret = -97;
		
	return ret;
}
int getinstallregion(){
	int ret = 0;
	
	//ModTheme *Thetheme = &themelist[orden[selectedtheme]];
	
	if(themelist[Selected_Theme].region == (u8*)69)
		ret = 1;
	else if(themelist[Selected_Theme].region == (u8*)74)
		ret = 2;
	else if(themelist[Selected_Theme].region == (u8*)75)
		ret = 3;
	else if(themelist[Selected_Theme].region == (u8*)85)
		ret = 4;
	else
		ret = -99;
		
	return ret;
}
void Check_List() {
	int i;
	
	for(i = 0; i < themecnt; i++) {
		gprintf("\n\ni[%i] WorkingName[%s] type[%i] \n\n",i , themelist[i].WorkingName, themelist[i].type);
	}
}
s32 filelist_retrieve(int dev) {
	char *dirpath = memalign(32, 256);
	static int NumberOfThemes = 71;
	struct dirent *entry = NULL;
	themelist = NULL;
	DIR *dir;
	themecnt = 0;
	int i, x = 0;
	char *tmpname = NULL;
	int strlength = 0;
	
     /* Generate dirpath */
     if(dev == 0)
		sprintf(dirpath, "sd:/themes");
	else
		sprintf(dirpath, "usb:/themes");
     //gprintf("Path: %s\n",dirpath);
	dir = opendir(dirpath);
	if (!dir) {
		gprintf("could not open dir(%s)\n",dirpath);
		return -1;
	}
	while((entry = readdir(dir))) {
		if(entry->d_type != 4) {
			if(strncmp(entry->d_name, ".", 1) != 0 && strncmp(entry->d_name, "..", 2) != 0)
			{	
				x += 1;
			}
		}
	}
	closedir(dir);
	int z = x + NumberOfThemes;
	//ThemeList = allocate_memory((sizeof(struct ModTheme)*(x+NumberOfThemes)));
	// *ThemeList = NULL;
	themelist = allocate_memory(sizeof(dirent_t) * z);
	memset(themelist, 0, sizeof(dirent_t) * z);
	dir = opendir(dirpath);
	if (!dir) {
		gprintf("could not open dir(%s)\n",dirpath);
		return -1;
	}
	
	for(i = 0;i < NumberOfThemes;i++) {
		snprintf(themelist[i].WorkingName, strlen(Theme_Name(i)) + 1, Theme_Name(i));
		snprintf(themelist[i].DisplayName, strlen(Theme_Title_Name(i)) + 1, Theme_Title_Name(i));
		themelist[i].type = 10;
	//	gprintf("theme =%s .type%d %d \n",themelist[i].name, themelist[i].type, i);
	}
    // gprintf("\n\ni [%i] \n\n",i);
	
	while((entry = readdir(dir))) {
		if(entry->d_type != 4) {
			if(strncmp(entry->d_name, ".", 1) != 0 && strncmp(entry->d_name, "..", 2) != 0) {	
				snprintf(themelist[i].WorkingName, strlen(entry->d_name) + 1, entry->d_name);
				tmpname = entry->d_name;
				strlength = strlen(tmpname);
				strlength = strlength - 3;
				snprintf(themelist[i].DisplayName, strlength, "%s", tmpname);
				themelist[i].type = 20;
	//			gprintf("theme =%s .type%d %d \n",themelist[i].name, themelist[i].type, i);
				i += 1;
			}
		}
	}
	
	closedir(dir);
	themecnt = i;
	gprintf("themecnt = %i \n", themecnt);
	free(dirpath);
	
    return 0;
}
void __Load_Config(void){
	/*
	int ret, i, j, k;
	char *Dev;
	
	s16* posiciones = allocate_memory(sizeof(u16)*5);

	orden=allocate_memory(sizeof(u16)*5);

	for(i = 0; i < 4; i++)
		posiciones[i] = -1;

	//sprintf(Dev, "%s:/%s%s", getdevicename(thememode), THEMEWII_PATH, THEMEWII_CONFIG_FILE);
	// Ordenar los juegos segun archivo de configuracion
	//char *archivoLeido = NULL;

	ret = 0; //Fat_ReadFile(Dev, (void *)&archivoLeido);

	if(ret>0){
		
		// Parse config file
		//i=16;
		//while(i<ret){
		//	j=archivoLeido[i]+archivoLeido[i+1]*256;

		//	for(k=0; k<4; k++)
		//		tempString[k]=archivoLeido[i+2+k];
		//	tempString[4]='\0';

		//	for(k=0; k<themecnt; k++){
		//		if(strcmp(tempString, themelist[k].name)==0){
					//printf("%s found at %d\n", tempString, k);
					//ThemeList[k].videoMode=archivoLeido[i+6];
					//ThemeList[k].videoPatch=archivoLeido[i+7];
					//ThemeList[k].language=archivoLeido[i+8];
					//ThemeList[k].hooktype=archivoLeido[i+9];
					//ThemeList[k].ocarina=archivoLeido[i+10];
					//ThemeList[k].debugger=archivoLeido[i+11];
					//ThemeList[k].bootMethod=archivoLeido[i+12];
					//ThemeList[k].ios=archivoLeido[i+13];

		//			posiciones[k]=j;
		//			break;
		//		}
		//	}
		//	i+=16;
		//}
		//free(archivoLeido);
		
	}
	
	//Initialize empty
	for(i=0; i<5; i++)
		orden[i]=EMPTY;

	//Place games
	for(i=0; i<4; i++)
		if(posiciones[i]!=-1)
			orden[posiciones[i]]=i;

	//Check if some game was not placed
	for(i=0; i<4; i++){
		if(posiciones[i]==-1){
			for(j=0; j<5; j++){
				if(orden[j]==EMPTY){
					posiciones[i]=j;
					break;
				}
			}
		}
		orden[posiciones[i]]=i;
	}
	free(posiciones);
	*/
	//Selected_Theme_Preview_Page = 0;
	//page = 0;
	//findnumpages();
}
void __Free_Last_Images(int in){
	if(themelist[in].preview_banner1 != NULL){
		MRC_Free_Texture(themelist[in].preview_banner1);
		MRC_Free_Texture(themelist[in].preview_banner2);
		MRC_Free_Texture(themelist[in].preview_banner3);
		MRC_Free_Texture(themelist[in].preview_banner4);
	}
	
	pageLoaded[in] = false;

	return;
}
void __Finish_ALL_GFX(void){
	int i;

	for(i = 0; i < MAX_TEXTURES; i++){
		MRC_Free_Texture(textures[i]);
	}
	
	free(themelist);
	MRC_Finish();
	saveconfig = false;
}
void __Load_Images_From_Page(){
	/*
	void *imgBuffer=NULL;
	int i, max, pos, ret, theme;

	//#ifdef DEBUG_MODE
	gprintf("Loading images...\n");
	//#endif

	max = COLS[wideScreen]*ROWS;
	pos = max*page;

	for(i = 0; i < max; i++){
		theme = orden[pos+i];
		if(theme != EMPTY){
			__Draw_Loading();

			// Load image from FAT
			//if(themelist[theme].type == 20)
			//	sprintf(tempString,"%s:/config/themewii/previewpics/%s.png",getdevicename(thememode), themelist[theme].name);
			//if(themelist[theme].type == 10)
			//	sprintf(tempString,"%s:/config/themewii/previewpics/%s.png",getdevicename(thememode), themelist[theme].name);
			//gprintf("\n\ntempString[%s]\n\n", tempString);
			ret = 0; //Fat_ReadFile(tempString, &imgBuffer);
			//gprintf("ret from fat read images %d\n",ret);
			
			// Decode image
			if(ret>0){
				themelist[theme].banner = MRC_Load_Texture(imgBuffer);
				free(imgBuffer);
			}
			else{
				themelist[theme].banner = __Create_No_Banner(themelist[theme].name, ANCHOIMAGEN[wideScreen], ALTOIMAGEN);
			}

			MRC_Resize_Texture(themelist[theme].banner, ANCHOIMAGEN[wideScreen], ALTOIMAGEN);
			__MaskBanner(themelist[theme].banner);
			MRC_Center_Texture(themelist[theme].banner, 1);
		}
		//MRC_Draw_Texture(64, 440, configuracionJuegos[theme].banner);
	}
	pageLoaded[page] = true;
	*/
	void *imgBuffer1 = NULL;
	void *imgBuffer2 = NULL;
	void *imgBuffer3 = NULL;
	void *imgBuffer4 = NULL;
	int i, ret = -1, type;
	type = themelist[Selected_Theme].type;
	//gprintf("\n\nselected_theme->type[%i] \n\n",themelist[Selected_Theme].type);
	/*
	//if(type == 20) {
	//	ret = _Extract_Csm();
	//	if(ret != 0) {
			
	//	}
	//}
	*/
	for(i = 0; i < 4; i++){
		
		__Draw_Loading();

		switch(i) {// Load image from FAT
			case 0:
				if(type == 10) {
					sprintf(tempString,"%s:/config/themewii/previewpics/%s_health.png",
						getdevicename(thememode), themelist[Selected_Theme].WorkingName);
					ret = Fat_ReadFile(tempString, &imgBuffer1);
				}
			break;
			case 1:
				if(type == 10) {
					sprintf(tempString,"%s:/config/themewii/previewpics/%s_screen1.png",getdevicename(thememode), themelist[Selected_Theme].WorkingName);
					ret = Fat_ReadFile(tempString, &imgBuffer2);
				}
			break;
			case 2:
				if(type == 10) {
					sprintf(tempString,"%s:/config/themewii/previewpics/%s_screen2.png",getdevicename(thememode), themelist[Selected_Theme].WorkingName);
					ret = Fat_ReadFile(tempString, &imgBuffer3);
				}
			break;
			case 3:
				if(type == 10) {
					sprintf(tempString,"%s:/config/themewii/previewpics/%s_screen3.png",getdevicename(thememode), themelist[Selected_Theme].WorkingName);
					ret = Fat_ReadFile(tempString, &imgBuffer4);
				}
			break;
			default:
			
			break;
			
			//gprintf("\n\ntempString[%s]\n\n", tempString);
		}
		//ret = Fat_ReadFile(tempString, &imgBuffer);
		//gprintf("\n\nafter first switch in load images\n\n");
			
			// Decode image
		if(ret > 0) {
			switch(i) {// Load image from FAT
				case 0:
					themelist[Selected_Theme].preview_banner1 = MRC_Load_Texture(imgBuffer1);
					free(imgBuffer1);
					MRC_Resize_Texture(themelist[Selected_Theme].preview_banner1, ANCHOIMAGEN[wideScreen], ALTOIMAGEN);
					__MaskBanner(themelist[Selected_Theme].preview_banner1);
					MRC_Center_Texture(themelist[Selected_Theme].preview_banner1, 1);
				break;
				case 1:
					themelist[Selected_Theme].preview_banner2 = MRC_Load_Texture(imgBuffer2);
					free(imgBuffer2);
					MRC_Resize_Texture(themelist[Selected_Theme].preview_banner2, ANCHOIMAGEN[wideScreen], ALTOIMAGEN);
					__MaskBanner(themelist[Selected_Theme].preview_banner2);
					MRC_Center_Texture(themelist[Selected_Theme].preview_banner2, 1);
				break;
				case 2:
					themelist[Selected_Theme].preview_banner3 = MRC_Load_Texture(imgBuffer3);
					free(imgBuffer3);
					MRC_Resize_Texture(themelist[Selected_Theme].preview_banner3, ANCHOIMAGEN[wideScreen], ALTOIMAGEN);
					__MaskBanner(themelist[Selected_Theme].preview_banner3);
					MRC_Center_Texture(themelist[Selected_Theme].preview_banner3, 1);
				break;
				case 3:
					themelist[Selected_Theme].preview_banner4 = MRC_Load_Texture(imgBuffer4);
					free(imgBuffer4);
					MRC_Resize_Texture(themelist[Selected_Theme].preview_banner4, ANCHOIMAGEN[wideScreen], ALTOIMAGEN);
					__MaskBanner(themelist[Selected_Theme].preview_banner4);
					MRC_Center_Texture(themelist[Selected_Theme].preview_banner4, 1);
				break;
				default:
				
				break;
			}
		}
		else {
			switch(i) {// Load image from FAT
				case 0:
					
					gprintf("\n\nname[%s] \n\n", themelist[Selected_Theme].WorkingName);
					themelist[Selected_Theme].preview_banner1 = __Create_No_Banner(" Health Screen ", ANCHOIMAGEN[wideScreen], ALTOIMAGEN);
					MRC_Resize_Texture(themelist[Selected_Theme].preview_banner1, ANCHOIMAGEN[wideScreen], ALTOIMAGEN);
					__MaskBanner(themelist[Selected_Theme].preview_banner1);
					MRC_Center_Texture(themelist[Selected_Theme].preview_banner1, 1);
					gprintf("\n\nincase 0 of else switch\n\n");
				break;
				case 1:
					
					themelist[Selected_Theme].preview_banner2 = __Create_No_Banner(" Screen 1 ", ANCHOIMAGEN[wideScreen], ALTOIMAGEN);
					MRC_Resize_Texture(themelist[Selected_Theme].preview_banner2, ANCHOIMAGEN[wideScreen], ALTOIMAGEN);
					__MaskBanner(themelist[Selected_Theme].preview_banner2);
					MRC_Center_Texture(themelist[Selected_Theme].preview_banner2, 1);
				break;
				case 2:
					
					themelist[Selected_Theme].preview_banner3 = __Create_No_Banner(" Screen 2 ", ANCHOIMAGEN[wideScreen], ALTOIMAGEN);
					MRC_Resize_Texture(themelist[Selected_Theme].preview_banner3, ANCHOIMAGEN[wideScreen], ALTOIMAGEN);
					__MaskBanner(themelist[Selected_Theme].preview_banner3);
					MRC_Center_Texture(themelist[Selected_Theme].preview_banner3, 1);
				break;
				case 3:
					themelist[Selected_Theme].preview_banner4 = __Create_No_Banner(" Screen 3 ", ANCHOIMAGEN[wideScreen], ALTOIMAGEN);
					MRC_Resize_Texture(themelist[Selected_Theme].preview_banner4, ANCHOIMAGEN[wideScreen], ALTOIMAGEN);
					__MaskBanner(themelist[Selected_Theme].preview_banner4);
					MRC_Center_Texture(themelist[Selected_Theme].preview_banner4, 1);
				break;
				default:
					
				break;
			}
		}
	}
	pageLoaded[Selected_Theme] = true;
}
void __Load_Skin_From_FAT(void){
	const char* fileNames[MAX_TEXTURES] = {
		"_arrows", "_background", (wideScreen? "_container_preview_wide" : "_container_preview"), "_empty",
		"_loading", "_numbers"};
	const u8* defaultTextures[MAX_TEXTURES] = {
		themewii_arrows_png, themewii_background_png, (wideScreen? themewii_container_preview_wide_png : themewii_container_preview_png), themewii_empty_png,
		themewii_loading_png, themewii_numbers_png};

	int i, ret;
	char *imgData = NULL;

	for(i = 0; i < MAX_TEXTURES; i++) {
		sprintf(tempString, THEMEWII_PATH "config/themewii/themewii%s.png", fileNames[i]);
		ret = Fat_ReadFile(tempString, (void*)&imgData);
		if(ret > 0) {
			textures[i] = MRC_Load_Texture(imgData);
			free(imgData);
		}
		else {
			textures[i] = MRC_Load_Texture((void *)defaultTextures[i]);
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

u32 filesize(FILE * file){
	u32 curpos, endpos;
	
	if(file == NULL)
		return 0;
	
	curpos = ftell(file);
	fseek(file, 0, 2);
	endpos = ftell(file);
	fseek(file, curpos, 0);
	
	return endpos;
}

s32 InstallFile(FILE * fp){

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
				
			if(newtmdc[i].type & 0x8000) {//Shared content! This is the hard part :P.
				return -1;
			}
			else {//Not shared content, easy
				sprintf(filename, "/title/00000001/%08lx/content/%08lx.app", ios, newtmdcontent->cid);
				break;
			}
		}
		else if(i == (((tmd *) SIGNATURE_PAYLOAD(newtmd))->num_contents) - 1) {
			return -1;
		}
	}

	free(newtmd);
	
	nandfile = ISFS_Open(filename, ISFS_OPEN_RW);
	ISFS_Seek(nandfile, 0, SEEK_SET);
	
	length = filesize(fp);
	gprintf("length of file %d \n",length);
	numchunks = length/CHUNKS + ((length % CHUNKS != 0) ? 1 : 0);
	
	gprintf("[+] Total parts: %d\n", numchunks);
	
	for(i = 0; i < numchunks; i++)
	{
		wiilight(1);
		data = memalign(32, CHUNKS);
		if(data == NULL)
		{
			return -1;
		}
		char *ms = memalign(32,256);
		
		sprintf(ms,"Installing part %ld",i+1);

		gprintf("	Installing part %ld\n",(i + 1) );

		__Draw_Message(ms,0);
		
		ret = fread(data, 1, CHUNKS, fp);
		if (ret < 0) 
		{
			gprintf("[-] Error reading from SD! (ret = %d)\n\n", ret);
			
			gprintf("	Press any button to continue...\n");
			return -1;
		}
		else
		{
			cursize = ret;
		}

		ret = ISFS_Write(nandfile, data, cursize);
		if(ret < 0)
		{
			gprintf("[-] Error writing to NAND! (ret = %d)\n\n", ret);
			gprintf("	Press any button to continue...\n");
			return ret;
		}
		free(data);
		wiilight(0);
	}
	
	ISFS_Close(nandfile);

	
	
	return 0;
}

int __Start_Install(){
	gprintf("install theme start! \n");
	char filepath[1024];
	FILE *fp = NULL;
	u8 *data;
	u32 length;
	int i;
	char *start = memalign(32,256);
	sprintf(start,"Starting Custom Theme Installation !");
	__Draw_Message(start,0);
	sleep(2);
	
	
	sprintf(filepath, "%s:/themes/%s", getdevicename(thememode),themelist[Selected_Theme].WorkingName);
	gprintf("filepath (%s) \n",filepath);
	fp = fopen(filepath, "rb");
		
     if (!fp)
     {
		gprintf("[+] File Open Error not on %s:/!\n", getdevicename(thememode));
		return -1;
     }
		
     length = filesize(fp);
     data = allocate_memory(length);
     memset(data,0,length);
     fread(data,1,length,fp);
	if(length <= 0)
     {
          printf("[-] Unable to read file !! \n");
          return MENU_HOME;
     }
     else
     {
          for(i = 0; i < length;)
          {
               if(data[i] == 83)
               {
                    if(data[i+6] == 52)  // 4
                    {
                        if(data[i+8] == 48)  // 0
                        {
						if(data[i+28] == 85)  // usa
						{
							themelist[Selected_Theme].version = 40;
							themelist[Selected_Theme].region = (u8*)85;
							break;
						}
						else if(data[i+28] == 74)  //jap
						{
							themelist[Selected_Theme].version = 40;
							themelist[Selected_Theme].region = (u8*)74;
							break;
						}
						else if(data[i+28] == 69)  // pal
						{
							themelist[Selected_Theme].version = 40;
							themelist[Selected_Theme].region = (u8*)69;
							break;
						}
                        }
                    else
                    {
					if(data[i+8] == 49)  // 4.1
					{
						if(data[i+31] == 85)  // usa
						{
							themelist[Selected_Theme].version = 41;
							themelist[Selected_Theme].region = (u8*)85;
							break;
						}
						else if(data[i+31] == 74)  //jap
                              {
                                   themelist[Selected_Theme].version = 41;
                                   themelist[Selected_Theme].region = (u8*)74;
                                   break;
                              }
                              else if(data[i+31] == 69)  // pal
                              {
                                   themelist[Selected_Theme].version = 41;
                                   themelist[Selected_Theme].region = (u8*)69;
                                   break;
                               }
                              else if(data[i+31] == 75)  // kor
                              {
                                   themelist[Selected_Theme].version = 41;
                                   themelist[Selected_Theme].region = (u8*)75;
                                   break;
                              }
                            }
                            else
                            {
                                if(data[i+8] == 50)  // 4.2
                                {
                                    if(data[i+28] == 85)  // usa
                                    {
                                        themelist[Selected_Theme].version = 42;
                                        themelist[Selected_Theme].region = (u8*)85;
                                        break;
                                    }
                                    else if(data[i+28] == 74)  // jap
                                    {
                                        themelist[Selected_Theme].version = 42;
                                        themelist[Selected_Theme].region = (u8*)74;
                                        break;
                                    }
                                    else if(data[i+28] == 69)  // pal
                                    {
                                        themelist[Selected_Theme].version = 42;
								themelist[Selected_Theme].region = (u8*)69;
                                        break;
                                    }
                                    else if(data[i+28] == 75)  // kor
                                    {
                                        themelist[Selected_Theme].version = 42;
                                        themelist[Selected_Theme].region = (u8*)75;
                                        break;
                                    }
                                }
                                else
                                {
                                    if(data[i+8] == 51) // 4.3
                                    {
                                        if(data[i+28] == 85)  // usa
                                        {
                                            themelist[Selected_Theme].version = 43;
                                            themelist[Selected_Theme].region = (u8*)85;
                                            break;
                                        }
                                        else if(data[i+28] == 74)  //jap
                                        {
                                            themelist[Selected_Theme].version = 42;
                                            themelist[Selected_Theme].region = (u8*)74;
                                            break;
                                        }
                                        else if(data[i+28] == 69)  // pal
                                        {
                                            themelist[Selected_Theme].version = 43;
                                            themelist[Selected_Theme].region = (u8*)69;
                                            break;
                                        }
                                        else if(data[i+28] == 75)  // kor
                                        {
                                            themelist[Selected_Theme].version = 43;
                                            themelist[Selected_Theme].region = (u8*)75;
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        if(data[i+6] == 51)  // 3
                        {
						if(data[i+8] == 50)  // 2
						{
							if(data[i+28] == 85)  // usa
							{
								themelist[Selected_Theme].version = 32;
								themelist[Selected_Theme].region = (u8*)85;
								break;
                                   }
							else
							{
								if(data[i+28] == 69)  // pal
								{
									themelist[Selected_Theme].version = 32;
									themelist[Selected_Theme].region = (u8*)69;
									break;
								}
								else
								{
                                        if(data[i+28] == 74)  // jap
									{
										themelist[Selected_Theme].version = 32;
										themelist[Selected_Theme].region = (u8*)74;
										break;
                                             }
                                        }
                                   }
                              }
                         }
                    }
               }
               i++;
          }
     }


        int current;
		current = getcurrentregion();
		int install;
		install = getinstallregion();
		
	//	gprintf("current(%d) install(%d) \n",current,install);
      //  gprintf("install theme .version(%d) .region(%c) \n",themelist[orden[selectedtheme]].version,themelist[orden[selectedtheme]].region);
      //  gprintf("cur theme .version(%d) .region(%c) \n",curthemestats.version,curthemestats.region);
		 if(curthemestats.version != themelist[Selected_Theme].version)
        {
            const char *badversion = "Install can not continue system versions differ ! Press any button to exit.";
			__Draw_Message(badversion,1);  
            free(data);
            fclose(fp);
            sysHBC();
        }
        else if(current != install)
        {
            const char *badregion = "Install can not continue system regions differ ! Press any button to exit.";
			__Draw_Message(badregion,1);
            free(data);
            fclose(fp);
            sysHBC();
        }
		
        free(data);
        fclose(fp);
		
        fp = fopen(filepath, "rb");
        if (!fp)
        {
            gprintf("[+] File Open Error not on SD!\n");
        }
	 
	ISFS_Initialize();
	// Install 
    InstallFile(fp);

    // Close file 
	if(fp) {
		fclose(fp);
	}
	char *done = (char*)memalign(32,256);
	sprintf(done,"Your Custom Theme has been installed !");
	__Draw_Message(done,0);
	sleep(3);
	free(done);
	ISFS_Deinitialize();
	return MENU_EXIT;
}


int __Select_Theme(void){
	/*
	int i, j, hotSpot, hotSpotPrev, ret;
	ret=MENU_EXIT;

	if(themecnt==0)
		return MENU_HOME;

	// Create/restore hotspots
	for(i=0;i<ROWS;i++){
		for(j=0;j<COLS[wideScreen];j++){
			int pos=i*COLS[wideScreen]+j;
			Wpad_AddHotSpot(pos,
				FIRSTCOL[wideScreen]-ANCHOIMAGEN[wideScreen]/2+j*SEPARACIONX[wideScreen],
				FIRSTROW-ALTOIMAGEN/2+i*SEPARACIONY,
				ANCHOIMAGEN[wideScreen],
				ALTOIMAGEN,
				(i==0? COLS[wideScreen]*(ROWS-1)+j : pos-COLS[wideScreen]),
				(i==ROWS-1? j : pos+COLS[wideScreen]),
				(j==0? pos+COLS[wideScreen]-1 : pos-1),
				(j==COLS[wideScreen]-1? pos-COLS[wideScreen]+1 : pos+1)
			);
		}
	}
	Wpad_AddHotSpot(HOTSPOT_LEFT, 0,160,32,88, HOTSPOT_LEFT, HOTSPOT_LEFT, HOTSPOT_RIGHT, 0);
	Wpad_AddHotSpot(HOTSPOT_RIGHT, 640-32,160,32,88, HOTSPOT_RIGHT, HOTSPOT_RIGHT,(COLS[wideScreen]*2)-1, HOTSPOT_LEFT);
	hotSpot = hotSpotPrev = -1;
	//gprintf("\n\npage[%i] pages[%i]\n\n", page + 1, pages);
	// Load images from actual page
	if(!pageLoaded[page])
		__Load_Images_From_Page();
	__Draw_Page(-1);
	//gprintf("\n\n2nd page[%i] pages[%i]\n\n", page + 1, pages);

	// Select game loop
	for(;;){
		hotSpot=Wpad_Scan();

		// If hot spot changed
		if(movingGame > -1){
			__Draw_Page(hotSpot);
		}
		else if(hotSpot != hotSpotPrev){
			hotSpotPrev = hotSpot;
			__Draw_Page(hotSpot);
		}

		if(hotSpot >= 0 && hotSpot <= COLS[wideScreen]*ROWS){
			selectedtheme = page*COLS[wideScreen]*ROWS + hotSpot;
		}
		else{
			selectedtheme = -1;
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
			//gprintf("\n\nmoving game \n\n");
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
			if(((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_MINUS) || (PAD_ButtonsDown(0) & PAD_TRIGGER_L)) || (hotSpot==HOTSPOT_LEFT && ((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_A) || (PAD_ButtonsDown(0) & PAD_BUTTON_A)))){
				//gprintf("in select theme page = %d maxpages = %d pages = %d\n",page,maxPages,pages);
				
				if (page == 0) {
					page = maxPages;
				}
				page-=1;
				ret=MENU_SELECT_THEME;
				break;
			}
			else if(((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_PLUS) || (PAD_ButtonsDown(0) & PAD_TRIGGER_R)) || (hotSpot==HOTSPOT_RIGHT && ((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_A) || (PAD_ButtonsDown(0) & PAD_BUTTON_A)))){
				page+=1;
				if (page >= maxPages)
					page = 0;
				ret=MENU_SELECT_THEME;
				break;
			}
			else if(((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_HOME) || (PAD_ButtonsDown(0) & PAD_BUTTON_START))){
				ret=MENU_HOME;
				break;
			}
			else if((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_B) || (PAD_ButtonsDown(0) & PAD_BUTTON_B)){
				ret=MENU_SELECT_THEME;
				break;
			}
			//if(WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_2)
			//	MRC_Capture();
		}
	}
	return ret;
	*/
	int i, j, hotSpot, hotSpotPrev, ret;
	
	ret = MENU_EXIT;
	
	if(themecnt == 0)
		return MENU_HOME;
	
	for(i = 0;i < ROWS;i++){
		for(j = 0;j < COLS[wideScreen];j++){
			int pos = (i * COLS[wideScreen]) + j;
			Wpad_AddHotSpot(pos,
				FIRSTCOL[wideScreen] - ANCHOIMAGEN[wideScreen]/2 + j * SEPARACIONX[wideScreen],
				FIRSTROW - ALTOIMAGEN/2 + i * SEPARACIONY,
				ANCHOIMAGEN[wideScreen],
				ALTOIMAGEN,
				(i == 0 ? COLS[wideScreen] * (ROWS - 1) + j : pos - COLS[wideScreen]),
				(i == ROWS - 1 ? j : pos + COLS[wideScreen]),
				(j == 0 ? pos + COLS[wideScreen] - 1 : pos - 1),
				(j == COLS[wideScreen] - 1 ? pos - COLS[wideScreen] + 1 : pos + 1)
			);
		}
	}
	Wpad_AddHotSpot(HOTSPOT_LEFT, 0, 160, 32, 88, HOTSPOT_LEFT, HOTSPOT_LEFT, HOTSPOT_RIGHT, 0);
	Wpad_AddHotSpot(HOTSPOT_RIGHT, 640-32, 160, 32, 88, HOTSPOT_RIGHT, HOTSPOT_RIGHT, (COLS[wideScreen] * 2) - 1, HOTSPOT_LEFT);
	hotSpot = hotSpotPrev = -1;
	
	if(!pageLoaded[Selected_Theme]) {
		__Load_Images_From_Page();
	}
	__Draw_Page(hotSpot);
	
	
	for(;;) {
		hotSpot = Wpad_Scan();
 
		if(hotSpot != hotSpotPrev) {
			hotSpotPrev = hotSpot;
			__Draw_Page(hotSpot);
		}

		MRC_Draw_Cursor(Wpad_GetWiimoteX(), Wpad_GetWiimoteY(), 0);

		if(((WPAD_ButtonsDown(WPAD_CHAN_0) & (WPAD_BUTTON_A | WPAD_BUTTON_B | WPAD_BUTTON_1)) || (PAD_ButtonsDown(0) & (PAD_BUTTON_A | PAD_TRIGGER_Z))) && hotSpot > -1 && hotSpot < COLS[wideScreen] * ROWS){
			if((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_A) || (PAD_ButtonsDown(0) & PAD_BUTTON_A)){
				Selected_Theme_Preview_Page = hotSpot;
				ret = MENU_SHOW_THEME;
				break;
			}
		}
		
		if(WPAD_ButtonsDown(WPAD_CHAN_0) || PAD_ButtonsDown(0)) {
			if(((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_MINUS) || (PAD_ButtonsDown(0) & PAD_TRIGGER_L)) || 
				(hotSpot == HOTSPOT_LEFT && ((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_A) || (PAD_ButtonsDown(0) & PAD_BUTTON_A)))) {
				if (Selected_Theme != 0) {
					__Free_Last_Images(Selected_Theme);
					Selected_Theme -= 1;
				}
				
				ret = MENU_SELECT_THEME;
				
				break;
			}
			else if(((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_PLUS) || (PAD_ButtonsDown(0) & PAD_TRIGGER_R)) || 
				(hotSpot == HOTSPOT_RIGHT && ((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_A) || (PAD_ButtonsDown(0) & PAD_BUTTON_A)))) {
				if (Selected_Theme != themecnt - 1){
					__Free_Last_Images(Selected_Theme);
					Selected_Theme += 1;
				}
				
				ret = MENU_SELECT_THEME;
				
				break;
			}
			else if(((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_HOME) || (PAD_ButtonsDown(0) & PAD_BUTTON_START))) {
				
				ret = MENU_HOME;
				
				break;
			}
			else if(((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_B) || (PAD_ButtonsDown(0) & PAD_BUTTON_B))) {
				if(Selected_Theme == 0)
					Selected_Theme = themecnt - 1;
				else if(Selected_Theme == themecnt - 1)
					Selected_Theme = 0;
				else
					Selected_Theme = 0;
					
				ret = MENU_SELECT_THEME;
				
				break;
			}
		}
	}
	
	return ret;
}
char *getsavename(u32 idx){
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
int __Check_App_Install()  {
	s32 rtn;
     char *savepath = (char*)memalign(32,256);
     char *oldapp = (char*)memalign(32,256);
	char *tmpstr = (char*)malloc(256);
	char *tmpstr2 = (char*)malloc(256);
	FILE *f = NULL;
	
	__Draw_Loading();
	
	sprintf(tmpstr2,"%s:/config/themewii/origtheme/%s.app", getdevicename(thememode),getappname(Current_System_Menu_Version));
	if(!Fat_CheckFile(tmpstr2)){
		sprintf(tmpstr,"%s.app for System Menu v%ld not found on %s ",getappname(Current_System_Menu_Version), Current_System_Menu_Version, getdevicedisplayname(thememode));
		__Draw_Message(tmpstr,0);
		sleep(3);
		sprintf(tmpstr, "%s.app should be in 'config/themewii/origtheme/%s.app'on SD/USB Device .", getappname(Current_System_Menu_Version), getappname(Current_System_Menu_Version)); 
		__Draw_Message(tmpstr,0);
		sleep(3);
		return -1;
	}
	sprintf(tmpstr,"Installing %s for System Menu v%ld from cache .",getappname(Current_System_Menu_Version), Current_System_Menu_Version);
	__Draw_Message(tmpstr,0);
	sleep(2);
		
	f = fopen(tmpstr2,"rb");
	if(!f)
	{
		gprintf("could not open %s \n", tmpstr2);
		return -1;
	}
	gprintf("\nInstalling %s.app ....", getappname(Current_System_Menu_Version));
	__Draw_Loading();
	rtn = InstallFile(f);
	if(rtn < 0){
		if(f)
			fclose(f);
		return rtn;
	}
		
	/* Close file */
	if(f)
		fclose(f);
		

	free(oldapp);
	free(savepath);
	free(tmpstr);
	free(tmpstr2);
	
	return MENU_SELECT_THEME;
}
#define PROJECTION_HEIGHT 64
int __Show_Theme(int selected){
	void* imageBuffer;
	MRCtex *themeImage, *projection;
	int i, j, ret;
	char *c, *r, a;
	dirent_t *theme = &themelist[Selected_Theme];
	/*
	// BLACK SCREEN
	a = 160;
	for(i = 0; i < 480; i++){
		if(a < 255 && ((i < 208 && i % 4 == 0) || i % 8 == 0))
			a++; 
		MRC_Draw_Box(0, i, 640, 1, a);
	}
*/
	
	// ANOTHER SCREEN FADE TYPE
	a = 200;
	for(i = 0; i < 480; i++){
		if(a < 255 && ((i < 100 && i % 4 == 0) || (i > 200 && i % 8 == 0)))
			a++;
		MRC_Draw_Box(0, i, 640, 1, 0x20202000+a);
	}
		

	// Load image from FAT
	//if(theme->type == 20)
	//	sprintf(tempString,"%s:/config/themewii/previewpics/%s.png",getdevicename(thememode) , theme->name);
	if(theme->type == 10) {
		switch(selected) {
			case 0:
				sprintf(tempString,"%s:/config/themewii/previewpics/%s_health.png",getdevicename(thememode) , theme->WorkingName);
			break;
			case 1:
				sprintf(tempString,"%s:/config/themewii/previewpics/%s_screen1.png",getdevicename(thememode) , theme->WorkingName);
			break;
			case 2:
				sprintf(tempString,"%s:/config/themewii/previewpics/%s_screen2.png",getdevicename(thememode) , theme->WorkingName);
			break;
			case 3:
				sprintf(tempString,"%s:/config/themewii/previewpics/%s_screen3.png",getdevicename(thememode) , theme->WorkingName);
			break;
			default:
			break;
		}
	}
	gprintf("tempstring %s \n",tempString);
	ret = Fat_ReadFile(tempString, &imageBuffer);
	// Decode image
	if(ret>0){
		themeImage = MRC_Load_Texture(imageBuffer);
		free(imageBuffer);

		MRC_Resize_Texture(themeImage, (wideScreen? 580 : 640), 340);
		//__MaskBanner(themeImage);
		//MRC_Center_Texture(themeImage, 1);

		projection = allocate_memory(sizeof(MRCtex));
		projection->buffer = allocate_memory(themeImage->width * PROJECTION_HEIGHT * 4);
		projection->width = themeImage->width;
		projection->height = PROJECTION_HEIGHT;
		//MRC_Center_Texture(projection, 1);
		projection->alpha = true;
	
		a = 128;
		r = (projection->buffer);
		for(i = 0; i < PROJECTION_HEIGHT; i++){
			c = (themeImage->buffer) + (((themeImage->height - 1) -i * 2) * themeImage->width) * 4;
			for(j = 0; j < themeImage->width; j++){
				r[0] = c[0];
				r[1] = c[1];
				r[2] = c[2];
				r[3] = a;
				c += 4;
				r += 4;
			}
			if(a > 4)
				a -= 4;
		}
	
		MRC_Draw_Texture(40, 40, themeImage);
		//MRC_Draw_Texture(175, 275, projection);
		MRC_Draw_String((640-strlen(theme->WorkingName))/2, 400, WHITE, theme->DisplayName);
		//MRC_Draw_String(30, 330, WHITE, "By ");
		MRC_Draw_String(40, 400, WHITE, "[A]-Install Theme");
		MRC_Draw_String(560, 400, WHITE, "[B]-Back");
		MRC_Free_Texture(themeImage);
		MRC_Free_Texture(projection);
	}
	else{
		MRC_Draw_String(30, 360, WHITE, "[A]-Install Theme");
		MRC_Draw_String(30, 390, WHITE, "[B]-Back");
		//MRC_Draw_String(30, 360, WHITE, "By ");
		MRC_Draw_String((640-strlen(theme->WorkingName)*8)/2, 250, WHITE, theme->DisplayName);
	}
	MRC_Render_Screen();
	
	ret = MENU_SELECT_THEME;
	for(;;){
		WPAD_ScanPads();
		PAD_ScanPads();
		if(WPAD_ButtonsDown(WPAD_CHAN_0) || PAD_ButtonsDown(0)){
			if((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_A) || (PAD_ButtonsDown(0) & PAD_BUTTON_A)) {
				if(theme->type == 20)
					ret = MENU_Install_Theme;
				if(theme->type == 10){
					ret = Menu_Start_Themeing;	
				}
				gprintf("theme->type[%d] ret [%d] \n", theme->type, ret);
				break;
			}
			if((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_B) || (PAD_ButtonsDown(0) & PAD_BUTTON_B)) {
				ret = MENU_SELECT_THEME;
				break;
			}
		}
		
	}

	return ret;
}


#define HOME_BUTTON_X			210
#define HOME_BUTTON_Y			120
#define HOME_BUTTON_WIDTH		220
#define HOME_BUTTON_HEIGHT		40
#define HOME_BUTTON_SEPARATION	5
int __Home(void){
	int i, hotSpot, hotSpotPrev, ret;
	bool repaint = true;
	
	const char *langs[5] = {
		"Saving/Loading Device ",
		"Install Original Theme .app",
		"Exit <HBC>",
		"Exit <System Menu>",
	};
	
	// Create/restore hotspots
	Wpad_CleanHotSpots();
	for(i = 0; i < 4; i++){
		Wpad_AddHotSpot(i,
			HOME_BUTTON_X,
			100 + i * (HOME_BUTTON_HEIGHT + HOME_BUTTON_SEPARATION),
			HOME_BUTTON_WIDTH,
			HOME_BUTTON_HEIGHT,
			(i == 0 ? 3 : i-1),
			(i == 3 ? 0 : i+1),
			i, i
		);
	}


	__Draw_Window(HOME_BUTTON_WIDTH + 44, 320, "Options");

	sprintf(tempString, "  IOS_%ld v_%ld", IOS_GetVersion(), IOS_GetRevision());
	MRC_Draw_String(250, 380, BLACK, tempString);
	//MRC_Draw_String(10, 440, 0x505050ff, lang[9+thememode]);
	sprintf(tempString, " ThemeWii v_%.1f (Scooby74029)", THEMEWII_VERSION);
	MRC_Draw_String(205, 350, BLACK, tempString);

	// Loop
	hotSpot = hotSpotPrev = -1;

	ret = MENU_SELECT_THEME;
	for(;;) {
		hotSpot = Wpad_Scan();

		// If hot spot changed
		if((hotSpot != hotSpotPrev && hotSpot < 4) || repaint){
			hotSpotPrev = hotSpot;

			for(i = 0; i < 4; i++) {
				__Draw_Button(i, langs[i], hotSpot == i);
			}
			repaint = false;
		}
		MRC_Draw_Cursor(Wpad_GetWiimoteX(), Wpad_GetWiimoteY(), 0);
		//gprintf("hotSpot = %d \n",hotSpot);
		if(((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_A) || (PAD_ButtonsDown(0) & PAD_BUTTON_A)) && hotSpot > -1 && hotSpot < 4){
			if(hotSpot == 0)
				ret = MENU_MANAGE_DEVICE;
			else if(hotSpot == 1)
				ret = MENU_ORIG_THEME;
			else if(hotSpot == 2)
				ret = MENU_EXIT;
			else if(hotSpot == 3)
				ret = MENU_EXIT_TO_MENU;
			break;
		}
		else if(((WPAD_ButtonsDown(WPAD_CHAN_0) & (WPAD_BUTTON_HOME | WPAD_BUTTON_B)) || (PAD_ButtonsDown(0) & (PAD_BUTTON_START | PAD_BUTTON_B)))) {
			ret = MENU_SELECT_THEME;
			break;
		}
		else {
			repaint = true;
		}
	}

	return ret;
}
bool Comparefolders(const char *src, const char *dest){
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
int Undo_U8_archive(void){
	U8_archive_header header;
	U8_node root_node;
	u32 tag;
	u32 num_nodes;
	U8_node* nodes;
	u8* string_table;
	size_t rest_size;
	unsigned int i;//,j;
	u32 data_offset;
	u16 dir_stack[16];
	int dir_index = 0;
 
	fread(&header, 1, sizeof header, fp);
	tag = be32((u8*) &header.tag);
	if (tag != 0x55AA382D) {
	 gprintf("No U8 tag");
	 return -3; 
	}
 
	fread(&root_node, 1, sizeof(root_node), fp);
	num_nodes = be32((u8*) &root_node.size) - 1;
	gprintf("Number of files: %d\n", num_nodes);
 
	nodes = malloc(sizeof(U8_node) * (num_nodes));
	fread(nodes, 1, num_nodes * sizeof(U8_node), fp);
 
	data_offset = be32((u8*) &header.data_offset);
	rest_size = data_offset - sizeof(header) - (num_nodes+1)*sizeof(U8_node);
 
	string_table = malloc(rest_size);
	fread(string_table, 1, rest_size, fp);
	
	//char *file = malloc(256);
	//char *save = malloc(256);
		
	for (i = 0; i < num_nodes; i++) {
		U8_node* node = &nodes[i];   
		u16 type = be16((u8*)&node->type);
		u16 name_offset = be16((u8*)&node->name_offset);
		u32 my_data_offset = be32((u8*)&node->data_offset);
		u32 size = be32((u8*)&node->size);
		char* name = (char*) &string_table[name_offset];
		u8* file_data;
		//char *no_ext_name = malloc(256);
		//char *check_name = malloc(256);
		//char workingname[] = {0};
		//int len = strlen(name);
		//int result = -1;
		
		//gprintf("dir_index(%d) \n",dir_index);
		if (type == 0x0100) {
		  // Directory
		  mkdir(name, 0777);
		  chdir(name);
		  dir_stack[++dir_index] = size;
		  gprintf("dir type(%d)  %*s%s/\n", dir_index, "", name);
		}
		else {
		  // Normal file
	 
			if (type != 0x0000) {
				gprintf("Unknown type");
				return -3;
			}
		 
			fseek(fp, my_data_offset, SEEK_SET);
			file_data = malloc(size);
			fread(file_data, 1, size, fp);
			write_file(file_data, size, name);
			free(file_data);
			gprintf("file type(%d)  %*s %s (%d bytes)\n", dir_index, "", name, size);
			
		}
	 
		while (dir_stack[dir_index] == i+2 && dir_index > 0) {
		  chdir("..");
		  dir_index--;
		}
	}
	return 0;
}

int Undo_U8(char *Filepath, char* Filesavepath){

	outdir = Filesavepath;
	gprintf("Extracting files to %s.\n", outdir);
	fp = fopen(Filepath, "rb");
 
	mkdir(outdir, 0777);
	chdir(outdir);
 
	Undo_U8_archive();
 
	fclose(fp);
 
	return 1;
}
#define DEVICE_BUTTON_X          200
#define DEVICE_BUTTON_Y          100
#define DEVICE_BUTTON_WIDTH      150
#define DEVICE_BUTTON_HEIGHT     30
#define DEVICE_BUTTON_SEPERATION 20

int __Select_Theme_Device(void) {
	int i, hotSpot, hotSpotPrev;
	bool repaint = true;
	
	const char *langs[3] = {
		"Sd Card",
		"Usb Device",
	};
	
	// Create/restore hotspots
	Wpad_CleanHotSpots();
	for(i = 0; i < 2; i++) {
		Wpad_AddHotSpot(i,
			DEVICE_BUTTON_X,
			180 + i * (DEVICE_BUTTON_HEIGHT + DEVICE_BUTTON_SEPERATION),
			DEVICE_BUTTON_WIDTH,
			DEVICE_BUTTON_HEIGHT,
			(i == 0 ? 2 : i - 1),
			(i == 2 ? 0 : i + 1),
			i, i
		);
	}

	// Background
	MRC_Draw_Texture(0, 0, textures[TEX_BACKGROUND]);

	MRC_Draw_String(95, 100, BLACK, "Select Device :");
	//MRC_Draw_String(95, 130, BLACK, "Theme Database file will be saved here also .");
	MRC_Draw_String(95, 360, BLACK, "[A]- Selection    [B]- Back    [HOME/start]- Options/Exit");
	
	sprintf(tempString, "IOS_%ld v_%ld", IOS_GetVersion(), IOS_GetRevision());
	MRC_Draw_String(95, 430, WHITE, tempString);
	
	sprintf(tempString, "System_Menu v%s_%s", getsysvernum(Current_System_Menu_Version), getregion(Current_System_Menu_Version));
	MRC_Draw_String(420, 430, WHITE, tempString);

	// Loop
	hotSpot = hotSpotPrev = -1;

	for(;;) {
		hotSpot = Wpad_Scan();

		// If hot spot changed
		if((hotSpot != hotSpotPrev && hotSpot < 2) || repaint){
			hotSpotPrev = hotSpot;

			for(i = 0; i < 2; i++) {
				__Draw_Button(i, langs[i], hotSpot == i);
			}
			repaint = false;
		}
		MRC_Draw_Cursor(Wpad_GetWiimoteX(), Wpad_GetWiimoteY(), 0);

		if(((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_A) || (PAD_ButtonsDown(0) & PAD_BUTTON_A))) { 	
			break;
		}
	}
	return hotSpot;
}
int Menu_Loop(int Mode) {
	
	int ret;
	char *tmpstr = NULL;

	// Check console language
	ret = CONF_GetLanguage();
	if(ret > 0 && ret <= CONF_LANG_DUTCH)
		lang = texts[ret - 1];
	else
		lang = texts[0];

	// Check if widescreen
	if(CONF_GetAspectRatio() == CONF_ASPECT_16_9)
		wideScreen = true;
		
	// Init MRC graphics
	MRC_Init();
	textures[1] = MRC_Load_Texture((void *)themewii_background_png);
	textures[4] = MRC_Load_Texture((void *)themewii_loading_png);
	
	Current_System_Menu_Version = GetSysMenuVersion();
	//gprintf("versionsys(%d) \n",Versionsys);
	if(Current_System_Menu_Version > 518) {
		ret = ISFS_Initialize();
		if(ret != 0) {
			__Draw_Message(lang[27], ret);
			sleep(3);
			return 0;
		}
		Current_System_Menu_Version = checkcustom(Current_System_Menu_Version);
		ISFS_Deinitialize();
	}
	thememode = Mode;
	//gprintf("thememode %d \n",thememode);
	
	if(Fat_Mount(thememode) < 0){
		__Draw_Message(lang[12], -1);
		return 0;
	}
	
	ret = filelist_retrieve(thememode);
	if(ret != 0) {
		__Draw_Message(lang[26], ret);
	}
	
	
	// Load skin images
	MRC_Free_Texture(textures[1]);
	MRC_Free_Texture(textures[4]);
	
	__Load_Skin_From_FAT();
	
	ret = MENU_SELECT_THEME;
	
	for(;;) {
		switch(ret) {
		
			case MENU_MANAGE_DEVICE: {
				thememode =  __Select_Theme_Device();
				gprintf("thememode = %d \n",thememode);
				free(themelist);
				themecnt = 0;
				Fat_Unmount(thememode);
				if(Fat_Mount(thememode) < 0){
					__Draw_Message(lang[12], -1);
					ret = MENU_EXIT;
					break;
				}
				
				ret = filelist_retrieve(thememode);
				
				ret = MENU_SELECT_THEME;
			}
			break;
			case MENU_SELECT_THEME:
				ret = __Select_Theme();
			break;
			case MENU_SHOW_THEME:
				ret = __Show_Theme(Selected_Theme_Preview_Page);
			break;
			case MENU_Install_Theme:
				ret = __Start_Install();
			break;
			case Menu_Start_Themeing:
				ret = MENU_MAKE_THEME;
			break;
			case MENU_MAKE_THEME:
				ret = __Theme_Make(themelist[Selected_Theme].WorkingName, themecnt);
			break;
			case MENU_ORIG_THEME:
				ret = __Check_App_Install();
			break;
			case MENU_HOME:
				ret = __Home();
			break;
			case MENU_EXIT:
			case MENU_EXIT_TO_MENU:	
			break;
		}
		if((ret == MENU_EXIT) | (ret == MENU_EXIT_TO_MENU))
			break;
	}		

	if(ret == MENU_EXIT_TO_MENU){
		__Draw_Message("Exiting to the System Menu ....",0);
		sleep(2);
		__Finish_ALL_GFX();
		free(tmpstr);
		free(themelist);
		SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
	}
	if(ret == MENU_EXIT){
		__Draw_Message("Exiting to HBC ....",0);
		sleep(2);
		__Finish_ALL_GFX();
		free(themelist);
		ret = 0;
	}
	gprintf("exiting at exit(0) \n");
	
	return ret;
}
