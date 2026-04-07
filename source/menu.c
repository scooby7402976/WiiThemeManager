/* menu.c
 *
 * theme_manager - Wii theme installer/downloader based on the gui of mighty channels(Marc) by Scooby74029 Copyright (c) 2026 Scooby74029
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
#include <math.h>
#include <ctype.h>

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
#include "zlib.h"
#include "theme_manager_arrows_png.h"
#include "theme_manager_background_png.h"
#include "theme_manager_container_png.h"
#include "theme_manager_container_wide_png.h"
#include "theme_manager_empty_png.h"
#include "theme_manager_loading_png.h"
#include "theme_manager_numbers_png.h"
#include "theme_manager_message_bubble_png.h"
#include "theme_manager_disclaimer_background_png.h"
#include "theme_manager_installer_empty_png.h"
#include "theme_manager_net_connect_png.h"
#include "theme_manager_no_net_connect_png.h"

static bool wideScreen = false;
static MRCtex* textures[MAX_TEXTURES];
int debugcard = 0;
bool priiloadercheck = false;
u32 system_Version = 0;

static s16* orden;
static int spinselected = -1;
static int spincolorselected = -1;
static int thememode = -1;
static u32 themecnt = 0;
static u16 page, maxPages = 1;
static int selectedtheme = 0, moving_Theme = -1;
static int loadingAnim = 0;
static bool display_progress = false;
static bool saveconfig = false;
static bool pageLoaded[75];
char tempString[128];
ModTheme ThemeList[MAXTHEMES];
CurthemeStats curthemestats;
dirent_t *ent = NULL;
dirent_t *nandfilelist = NULL;
extern GXRModeObj *vmode;
extern u32* framebuffer;
//extern u32 content_length;
bool needloading = false;
static bool downloadable_theme_List = false;
static bool netconnection;
bool priiloaderackknowledgement = false;
u32 known_Versions[KNOWN_SYSTEMMENU_VERSIONS] = {
	416, 417, 418, 448, 449, 450, 454, 480, 481, 482, 486, 512, 513, 514, 518, 608, 609, 610
};
char *regions[KNOWN_SYSTEMMENU_VERSIONS] =      {"J", "U", "E", "J", "U", "E", "K", "J", "U", "E", "K", "J", "U", "E", "K", "J", "U", "E"};
char *knownappfilenames[KNOWN_SYSTEMMENU_VERSIONS] = {
	"0000006f.app", "00000072.app", "00000075.app", "00000078.app", "0000007b.app", "0000007e.app", "00000081.app", "00000084.app", "00000087.app", "0000008a.app", "0000008d.app", "00000094.app", 		"00000097.app", "0000009a.app", "0000009d.app", "0000001c.app", "0000001f.app", "00000022.app"
};
char *appfilename[2] = { "Cetk", "Tmd" };
char *updatedownloadcount = NULL;
const u8 COLS[]={3, 4};
#define ROWS 3
const u8 FIRSTCOL[]={136, 100}; // 136 112
#define FIRSTROW 130 
const u8 SEPARACIONX[]={180, 145}; //136
#define SEPARACIONY 120
const u8 ANCHOIMAGEN[]={154, 118}; //116
#define ALTOIMAGEN 90
const char *themedir = "themes";
bool system_is_vWii;
bool device_Chose = false;
char** split_result = NULL;

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
	case 609: 
		return "1f";
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
	case 610: 
		return "22";
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
	case 608:
		return "1c";
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
const char *get_storage_name(int index) {
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
const char *get_display_region(u32 num) {
    switch(num)
    {
    case 417:
    case 449:
    case 481:
    case 513:
	case 609:
        return "U";
        break;
    case 418:
    case 450:
    case 482:
    case 514:
	case 610:
        return "E";
        break;
    case 416:
    case 448:
    case 480:
    case 512:
	case 608:
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
const char *get_system_version_Display(u32 num) {
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
	case 608:
	case 609:
	case 610:
		return "4.3";
    default:
        return "UNKNOWN";
        break;
    }
}
bool __Check_HBC(void) {
	u32 *stub = (u32 *)0x80001800;
	__Draw_Loading(440, 440);
	// Check HBC stub
	if (*stub) {
		__Draw_Loading(440, 440);
		return true;
	}
	__Draw_Loading(440, 440);
	return false;
}
bool checkNinit_netconnection() {
	__Draw_Loading(440, 440);
	s32 ret = net_init();
	logfile("ret net init() = [%i]\n", ret);
	__Draw_Loading(440, 440);
	if(ret == 0) { __Draw_Loading(440, 440); return true; }
	__Draw_Loading(440, 440);
	return false;
}
bool checkforpriiloader() {
	dirent_t *priiloaderfiles = NULL;
	u32 nandfilecnt;
	u32 filecntr;
	char *searchstr;
	__Draw_Loading(440, 440);
	searchstr = "title_or.tmd";
	getdir("/title/00000001/00000002/content",&priiloaderfiles,&nandfilecnt);
	for(filecntr = 0; filecntr < nandfilecnt; filecntr++) {
		if(!strcmp(priiloaderfiles[filecntr].name, searchstr)) {
		__Draw_Loading(440, 440);
		return true;
		}
	}
	__Draw_Loading(440, 440);
	return false; 
}
void logfile(const char *format, ...) {
	char buffer[256];
	char path[256];
	va_list args;
	va_start (args, format);
	vsprintf (buffer,format, args);
	FILE *f = NULL;
	sprintf(path, "%s:/theme_manager.log", get_storage_name(thememode));
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
void __Draw_Net_Connection(int x, int y, bool connected) {
	if(!connected) { 
		MRC_Resize_Texture(textures[TEX_NET_CONNECT], 60, 60);
		MRC_Draw_Texture(x, y, textures[TEX_NET_CONNECT]);
		
	}
	else {
		MRC_Resize_Texture(textures[TEX_NO_NET_CONNECT], 60, 60);
		MRC_Draw_Texture(x, y, textures[TEX_NO_NET_CONNECT]);
		
	}
	
	MRC_Render_Box(x, y);
	return;
}
void __Draw_Loading(int x, int y) {
 	MRC_Draw_Tile(x, y, textures[TEX_LOADING], 24, loadingAnim);
	MRC_Render_Box(x, y);
	loadingAnim += 1;
	if(loadingAnim == 16)
		loadingAnim = 0;
	return;
}
void __Draw_Page(int selected) {
	int i, j, x, y, containerWidth, theme;
	//framebufferRGBA = NULL;
	containerWidth=textures[TEX_CONTAINER]->width/2;

	MRC_Draw_Texture(0, 0, textures[TEX_BACKGROUND]);
	sprintf(tempString, "System Menu v%s_%s %u", get_system_version_Display(system_Version), get_display_region(system_Version), system_Version);
	MRC_Draw_String(((640-strlen(tempString)*8)/2), 20, WHITE, tempString);
	if(downloadable_theme_List) MRC_Draw_String(((640-strlen("Downloader")*8)/2), 50, WHITE, "Downloader");
	else MRC_Draw_String(((640-strlen("Installer")*8)/2), 50, WHITE, "Installer");
	sprintf(tempString, "IOS %i", IOS_GetVersion());
	MRC_Draw_String(45, 20, WHITE, tempString);
	MRC_Draw_String(25, 450, WHITE, "[A] - Select Theme");
	MRC_Draw_String((640-strlen("[HOME] - Options")*8)-15, 450, WHITE, "[HOME] - Options");
		
	if(themecnt == 0 || !pageLoaded[page]){
		return;
	}
	logfile("drawing page before setting themes\n");
	// themes
	theme = COLS[wideScreen]*ROWS*page;
	y = FIRSTROW;
	for(i = 0; i < ROWS; i++){
		x = FIRSTCOL[wideScreen];
		for(j = 0; j < COLS[wideScreen]; j++){
			if(moving_Theme != theme){
				if(orden[theme] == EMPTY){
					MRC_Draw_Texture(x, y, textures[TEX_EMPTY]);
					MRC_Draw_Tile(x, y, textures[TEX_CONTAINER], containerWidth, 0);
				}else if(selected == i*COLS[wideScreen]+j){
					MRC_Draw_Texture(x, y, ThemeList[orden[theme]].banner);
					MRC_Draw_Tile(x, y, textures[TEX_CONTAINER], containerWidth, 1);
					sprintf(tempString, "%s", ThemeList[orden[theme]].title);
					MRC_Draw_String(x - containerWidth/2, y + 50, WHITE, tempString);
					if(downloadable_theme_List) {
						if(netconnection)
							if(ThemeList[orden[theme]].has_banner == false)
								MRC_Draw_String((640-strlen("[1] - Download Image")*8)-15, 430, WHITE, "[1] - Download Image");
					}
					else {
						if(ThemeList[orden[theme]].has_banner == false)
							MRC_Draw_String((640-strlen("[1] - Delete File")*8)-15, 430, WHITE, "[1] - Delete File");
					}
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
	logfile("drawing page after setting themes\n");
	//if(!downloadable_theme_List)
	//	MRC_Draw_String((640-strlen("[1] - Delete File")*8)-15, 430, WHITE, "[1] - Delete File");
	// Page number
	sprintf(tempString, "%d of %d %s", page + 1, maxPages, (maxPages == 1 ? "page" : "pages"));
	MRC_Draw_String(55, 60, WHITE, tempString);

	if(moving_Theme > -1){
		if(orden[moving_Theme]==EMPTY){
			MRC_Draw_Texture(Wpad_GetWiimoteX(), Wpad_GetWiimoteY(), textures[TEX_EMPTY]);
		}else{
			MRC_Draw_Texture(Wpad_GetWiimoteX(), Wpad_GetWiimoteY(), ThemeList[orden[moving_Theme]].banner);
		}
	}

	// Arrows
	MRC_Draw_Tile(ARROWS_X, ARROWS_Y, textures[TEX_ARROWS], ARROWS_WIDTH, 0+(page>=0)+(page>=0 && selected==HOTSPOT_LEFT));
	MRC_Draw_Tile(640-ARROWS_X, ARROWS_Y, textures[TEX_ARROWS], ARROWS_WIDTH, 3+(page+1<maxPages)+(page+1<maxPages && selected==HOTSPOT_RIGHT));
	return;
}

void __Draw_Button(int hot, const char* text, bool selected) {
	hotSpot button = Wpad_GetHotSpotInfo(hot);
	int textX = button.x+(button.width-strlen(text)*8)/2;
	u32 color;
	if(selected){
		MRC_Draw_Box(button.x, button.y, button.width, button.height/2, 0x000002ff);
		MRC_Draw_Box(button.x, button.y+button.height/2, button.width, button.height/2, BLACK);
		color = WHITE;
	}else{
		MRC_Draw_Box(button.x, button.y, button.width, button.height/2, 0xe3e3e3ff);
		MRC_Draw_Box(button.x, button.y+button.height/2, button.width, button.height/2, 0xeaeaeaff);
		MRC_Draw_Box(button.x, button.y, button.width, 1, 0xf3f3f3ff);
		MRC_Draw_Box(button.x, button.y+button.height-1, button.width, 1, 0xf5f5f5ff);
		MRC_Draw_Box(button.x, button.y, 1, button.height, 0xf3f3f3ff);
		MRC_Draw_Box(button.x+button.width-1, button.y, 1, button.height, 0xf5f5f5ff);
		color = BLACK;
	}
	MRC_Draw_String(textX, button.y+button.height/2-8, color, text);
	return;
}

void __Draw_Window(int width, int height, const char* title) {
	int x=(640-width)/2;
	int y=(480-height)/2-32;
	__Draw_Page(-1);
	
	
	MRC_Draw_Box(x, y, width, 48, BLACK);
	MRC_Draw_Box(x, y+48, width, height, WHITE_SMOKE);

	MRC_Draw_String2(x+(width-strlen(title)*8)/2, y+8, WHITE_SMOKE, title);
	return;
}

void __Draw_Message(const char* title, int y, u32 color) {
	MRC_Draw_String(((640-strlen(title)*8)/2), y, color, title);
	MRC_Render_Screen();
	return;
}
int __Question_Window(const char* title, const char* text, const char* a1, const char* a2) {
	int i, hotSpot, hotSpotPrev, QUESTION_BUTTON_X = 90, QUESTION_BUTTON_Y = 240, QUESTION_BUTTON_SEPARATION = 30, QUESTION_BUTTON_WIDTH = 175, QUESTION_BUTTON_HEIGHT = 30;
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
			ret=hotSpot;
			break;
		}
	}
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

	if(moving_Theme!=-1)
		maxPages++;

	if(maxPages > 75)
		maxPages = 75;
	return;
}
void __Save_Changes(void){
	int i, j, configsize;
	unsigned char *outBuffer;
	char filepath[128];
	
	configsize=1+15+themecnt*(2+4+7+3); //1=cfgversion + 1=disclaimer accepted + 1=autoboot(SD-0/USB-1) + 13=reserved   +   (2=position 6=id 8=reserved)=16
	//logfile("Saving changes... (%d bytes)\n", configsize);
	outBuffer=allocate_memory(configsize);
	for(i=0; i<16; i++)
		outBuffer[i]=0;
	outBuffer[0]=THEMEMANAGER_VERSION;
	j=16;
	for(i=0; i<themecnt; i++){
		__Draw_Loading(440, 440);
		if(orden[i]!=EMPTY){
			outBuffer[j] = i%256;
			outBuffer[j+1] = i/256;
			outBuffer[j+2] = ThemeList[orden[i]].id[0];
			outBuffer[j+3] = ThemeList[orden[i]].id[1];
			outBuffer[j+4] = ThemeList[orden[i]].id[2];
			outBuffer[j+5] = ThemeList[orden[i]].id[3];

			outBuffer[j+6] = ThemeList[orden[i]].id[4];
			outBuffer[j+7] = ThemeList[orden[i]].id[5];
			outBuffer[j+8] = 0; 
			outBuffer[j+9] = 0; 
			outBuffer[j+10] = 0; 
			outBuffer[j+11] = 0; 
			outBuffer[j+12] = 0;
			outBuffer[j+13] = 0;
			outBuffer[j+14] = 0;
			outBuffer[j+15] = 0;//ThemeList[orden[i]].bootMethod;
			j+=16;
		}
	}
	// Save changes to FAT
	sprintf(filepath, "%s:/apps/thememanager/thememanager.cfg", get_storage_name(thememode));
	Fat_SaveFile(filepath, (void *)&outBuffer, configsize);
	saveconfig=false;
	return;
}

void __save_title_list() {
	char savefile_path[256];
	FILE * savefile;
	u32 x;

	sprintf(savefile_path, "%s:/apps/thememanager/themeorder.cfg", get_storage_name(thememode));
	logfile("savefile_path = %s\n", savefile_path);
	savefile = fopen(savefile_path, "wb");
    if (!savefile){
        if(debugcard) logfile("[+] Unable to Open file Error!\n");
    }
	else {
		//if(debugcard) logfile("writing save file \n");
		
		for(x=0;x<themecnt;x++) {
			fprintf(savefile, "%s,%s,%s,%s,%s,\n", ThemeList[orden[x]].title, ThemeList[orden[x]].id, ThemeList[orden[x]].mym, ThemeList[orden[x]].png, ThemeList[orden[x]].downloads);
			//if(debugcard) logfile("line ->> %d title ->> %s\n", x, ThemeList[orden[x]].title);
		}
		fprintf(savefile, "%s", "\n");
		// Close file 
		if(savefile) fclose(savefile);
	}
	saveconfig = false;
	return;
}
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
const char *get_content_name_noextension(u32 system_Version) {
	switch(system_Version){
		case 417: return "00000072";// usa
		break;
		case 449: return "0000007b";
		break;
		case 481: return "00000087";
		break;
		case 513: return "00000097";
		break;
		case 609: return "0000001f";// usa
		break;
		case 418: return "00000075";// pal
		break;
		case 450: return "0000007e";
		break;
		case 482: return "0000008a";
		break;
		case 514: return "0000009a";
		break;
		case 610: return "00000022";// pal
		break;
		case 416: return "00000070";// jpn
		break;
		case 448: return "00000078";
		break;
		case 480: return "00000084";
		break;
		case 512: return "00000094";
		break;
		case 608: return "0000001c";// jpn
		break;
		case 486: return "0000008d";// kor
		break;
		case 454: return "00000081";
		break;
		case 518: return "0000009d";// kor
		break;
		default: return "UNKNOWN";
		break;
	}
}	
bool retreivecurrentthemeregion(u32 inputversion) {
	switch(inputversion)
	{
		case 416:
		case 448:
		case 480:
		case 512:
		case 608:
			curthemestats.region = (u8*)74;
		break;
		case 417:
		case 449:
		case 481:
		case 513:
		case 609:
			curthemestats.region = (u8*)85;
		break;
		case 418:
		case 450:
		case 482:
		case 514:
		case 610:
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
	return 1;
}

s32 GetTMD(u64 tid, signed_blob **outbuf, u32 *outlen) {
	void *p_tmd = NULL;

	u32 len;
	s32 ret;
	__Draw_Loading(440, 440);
	/* Get TMD size */
	ret = ES_GetStoredTMDSize(tid, &len);
	if (ret < 0)
		return ret;
	__Draw_Loading(440, 440);
	/* Allocate memory */
	p_tmd = allocate_memory(len);
	if (!p_tmd)
		return -1;
	__Draw_Loading(440, 440);
	/* Read TMD */
	ret = ES_GetStoredTMD(tid, p_tmd, len);
	if (ret < 0)
		goto err;
	__Draw_Loading(440, 440);
	/* Set values */
	*outbuf = p_tmd;
	*outlen = len;
	__Draw_Loading(440, 440);
	return 0;
	
err:
	/* Free memory */
	free(p_tmd);
	__Draw_Loading(440, 440);
	return ret;
}
s32 GetVersion(u64 tid, u16 *outbuf, bool *vWii) {
	signed_blob *p_tmd = NULL;
	tmd      *tmd_data = NULL;

	u32 len;
	s32 ret;
	__Draw_Loading(440, 440);
	/* Get title TMD */
	ret = GetTMD(tid, &p_tmd, &len);
	if (ret < 0)
		return ret;

	/* Retrieve TMD info */
	tmd_data = (tmd *)SIGNATURE_PAYLOAD(p_tmd);

	/* Set values */
	*outbuf = tmd_data->title_version;
	*vWii = (bool)tmd_data->vwii_title;
	/* Free memory */
	free(p_tmd);
	__Draw_Loading(440, 440);
	return 0;
}
u32 Get_system_version() {
    //Get sysversion from TMD
    u64 TitleID = 0x0000000100000002LL;
	u16 version;
	bool is_vWii;
	__Draw_Loading(440, 440);
	GetVersion(TitleID, &version, &is_vWii);
	system_is_vWii = is_vWii;
	__Draw_Loading(440, 440);
    return version;
}
u32 check_custom_system_version() {
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


char** split_string(char* str, const char delimiter, int* count) {
    
    int num_tokens = 0;
    char* token;
    // Delimiter string for strtok can contain multiple characters.
    // We create one here with just the single character delimiter.
    char delim_str[2]; 
    delim_str[0] = delimiter;
    delim_str[1] = '\0';

    // 1. Count the number of tokens
    char* temp_str = strdup(str); // Use a copy to count without modifying original's structure prematurely
    if (temp_str == NULL) {
        return NULL;
    }
    token = strtok(temp_str, delim_str);
	
    while (token != NULL) {
        num_tokens++;
        token = strtok(NULL, delim_str);
		
    }
    free(temp_str); // Free the temporary copy

    // 2. Allocate memory for the array of string pointers
    // Add space for a final NULL pointer to mark the end of the array
    split_result = malloc(sizeof(char*) * (num_tokens + 1));
    if (split_result == NULL) {
        if (count != NULL) *count = 0;
        return NULL;
    }

    // 3. Populate the array with tokens
    int idx = 0;
    // Use strtok again on the actual input string 'str'
    token = strtok(str, delim_str);
	
    while (token != NULL) {
        split_result[idx++] = token; // strtok returns pointers to locations within the original string
        token = strtok(NULL, delim_str);
		
    }
    split_result[idx] = NULL; // Add the NULL terminator at the end

    if (count != NULL) {
        *count = num_tokens;
    }

    return split_result;
}
u32 filelist_retrieve(bool downloadable_theme_list, int filter) {
    char dirpath[MAX_FILEPATH_LEN];
	u32 cnt = 0, x= 0, j = 0;
	struct dirent *entry = NULL;
	needloading = true;
	char filepath[128];
	FILE *themelist;
	char filebuf[256];
	//int linelen = 0;
	char* str_copy = NULL;
	char **tokens =NULL;
	__Draw_Loading(440, 440);
	if(downloadable_theme_list) {
		if(themecnt > 0) {
			for(int r = 0;r<themecnt;r++) {
				ThemeList[r].title = "";
				ThemeList[r].id = "";
				ThemeList[r].mym = "";
				ThemeList[r].png = "";
				ThemeList[r].downloads = "";
				ThemeList[r].has_banner = 0;
				ThemeList[r].banner = NULL;
				ThemeList[r].type = 0;
				ThemeList[r].size = 0;
				ThemeList[r].downloadcount = 0;
				__Draw_Loading(440, 440);
			}
		}
		sprintf(filepath, "%s:/apps/thememanager/themeorder.cfg", get_storage_name(thememode));
		themelist = fopen(filepath, "r");
		if(!themelist) {
			logfile("Unable to open %s\n", filepath);
			return 0;
		}
		
		//logfile("filter[%c]\n", filter);
		while (fgets(filebuf, 256, themelist) != NULL) {
			//logfile("line[%d]wfilter[%d] -> %c\n", j, x, filebuf[0]);
			if(filebuf[0] == filter || filter == 0) {
            
				str_copy = strdup(filebuf); // Use strdup to create a writable copy
                if (str_copy == NULL) {
                    logfile("strdup failed\n");
                    return 0;
                }
                int count;
                tokens = split_string(str_copy, ',', &count);
                
                if (tokens) {
                    //logfile("Found %d tokens:\n", count);
                    
                    for (int z = 0; z < count; z++) {
                        //logfile(" x[%d] Token[%d]: %s\n", x, z, tokens[z]);
                        switch(z) {
                            case 0:
                            ThemeList[x].title = tokens[z];
                            break;
                            case 1:
                            ThemeList[x].id = tokens[z];
                            break;
                            case 2:
                            ThemeList[x].mym = tokens[z];
                            break;
                            case 3:
                            ThemeList[x].png = tokens[z];
                            break;
                            case 4:
                            ThemeList[x].downloads = tokens[z];
                            break;
                        }
                    }
                    ThemeList[x].type = 10;
                    ThemeList[x].has_banner = 0;
					ThemeList[x].banner = NULL;
					ThemeList[x].size = 0;
					ThemeList[x].downloadcount = 0;
					x++;
                }
				
			}
            __Draw_Loading(440, 440);
		}
		
		
		free(str_copy);
		free(tokens);
		free(split_result);
        if(x == 0) cnt = 0;
		else cnt = x -1;
		if(filter != 0) cnt = x;
    }
	else {
		logfile("in retreive list installs\n");
		//Generate dirpath 
		DIR *dir;
		sprintf(dirpath, "%s:/themes", get_storage_name(thememode));
		/* Open directory */
		dir = opendir(dirpath);
		themedir = "themes";
		if (!dir)
		{
			sprintf(dirpath, "%s:/modthemes", get_storage_name(thememode));
			dir = opendir(dirpath);
			themedir = "modthemes";
			if (!dir) {
				if(debugcard) logfile("Unable to open %s \n", get_storage_name(thememode));
				sprintf(dirpath, "%s:/themes", get_storage_name(thememode));
				themedir = "themes";
				mkdir(dirpath,0777);
				return 0;
			}
		}else logfile("dir open\n");
		cnt = 0;
		// Get directory entries 
		while((entry = readdir(dir))) // If we get EOF, the expression is 0 and
										 // the loop stops. 
		{
			if(strncmp(entry->d_name, ".", 1) != 0 && strncmp(entry->d_name, "..", 2) != 0)
			cnt += 1;
            
		}
		rewinddir(dir);
		ent = allocate_memory(sizeof(dirent_t) * cnt);
		cnt = 0;
		while((entry = readdir(dir))) // If we get EOF, the expression is 0 an // the loop stops. 
		{
			if(needloading) __Draw_Loading(440, 440);
			if(strncmp(entry->d_name, ".", 1) != 0 && strncmp(entry->d_name, "..", 2) != 0){
				strcpy(ent[cnt].name, entry->d_name);
				ThemeList[cnt].title = ent[cnt].name;
				ThemeList[cnt].type = 20;
				ThemeList[cnt].has_banner = 0;
				cnt += 1;
				
			}
            __Draw_Loading(440, 440);
		}
		qsort(ThemeList, cnt, sizeof(ModTheme), __themeCmp);
		closedir(dir);
	}
	logfile("leaving retreive\n");
    return cnt;
}
void __Set_Theme_Order() {
	int i, j;
	if(orden) free(orden);
	s16* posiciones=allocate_memory(sizeof(u16)*themecnt);
	orden=allocate_memory(sizeof(u16)*MAXTHEMES);
	
	for(i=0; i<themecnt; i++)
		posiciones[i]=-1;
	
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
	logfile("almost out of set theme order()\n");
	selectedtheme=0;
	page=0;
	findnumpages();
	return;
}
void __Free_Channel_Images(){
	int i;
	int imagesPerScreen = COLS[wideScreen]*ROWS;

	for(i=0; i<maxPages*imagesPerScreen; i++){
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
	return;
}
void __Finish_ALL_GFX() {
	int i;
	if(thememode >= 0)
	__Free_Channel_Images();
	for(i=0; i<MAX_TEXTURES; i++){
		MRC_Free_Texture(textures[i]);
	}
	MRC_Finish();
	saveconfig=false;
	return;
}
void __Load_Images_From_Page() {
	void *imgBuffer=NULL;
	int i, max, pos, ret = -1, theme;
	max = COLS[wideScreen]*ROWS;
	pos = max*page;
	logfile("before loading banner img\n");
	for(i = 0; i < max; i++){
		theme = orden[pos+i];
		if(theme != EMPTY){
			if(ThemeList[theme].type == 10) 
				sprintf(tempString,"%s:/apps/thememanager/imgs/%s", get_storage_name(thememode), ThemeList[theme].png);
			ret = Fat_ReadFile(tempString, &imgBuffer, 1);
			
			// Decode image
			if(ret > 0){
				ThemeList[theme].banner = MRC_Load_Texture(imgBuffer);
				free(imgBuffer);
				ThemeList[theme].has_banner = true;
			}
			else{
				ThemeList[theme].banner = __Create_No_Banner("Theme Manager", ANCHOIMAGEN[wideScreen], ALTOIMAGEN);
				
			}
			MRC_Resize_Texture(ThemeList[theme].banner, ANCHOIMAGEN[wideScreen], ALTOIMAGEN);
			__MaskBanner(ThemeList[theme].banner);
			MRC_Center_Texture(ThemeList[theme].banner, 1);
		}
		//MRC_Draw_Texture(64, 440, configuracionJuegos[theme].banner);
	}
	logfile("after loading banner img\n");
	pageLoaded[page] = TRUE;
	return;
}
void __load_textures() {
	const char* fileNames[MAX_TEXTURES]={
		"_arrows", "_background", (wideScreen? "_container_wide" : "_container"), "_empty",
		"_loading", "_numbers", "_message_bubble", "_disclaimer_background", "_installer_empty", "_no_net_connect", "_net_connect"}; //, "_qmark"};
	const u8* defaultTextures[MAX_TEXTURES]={
		theme_manager_arrows_png, theme_manager_background_png, (wideScreen? theme_manager_container_wide_png : theme_manager_container_png), theme_manager_empty_png,
		theme_manager_loading_png, theme_manager_numbers_png, theme_manager_message_bubble_png, theme_manager_disclaimer_background_png, theme_manager_installer_empty_png, theme_manager_no_net_connect_png, theme_manager_net_connect_png};

	int i, ret;
	char *imgData = NULL;
	for(i = 0; i < MAX_TEXTURES; i++){
		if(thememode == -1) {
			textures[i] = MRC_Load_Texture((void *)defaultTextures[i]);
		}
		else {
			sprintf(tempString, "%s:/apps/theme_manager/theme_manager%s.png", get_storage_name(thememode), fileNames[i]);
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
	return;
}
/* Constant */
#define BLOCK_SIZE	0x1000
#define CHUNKS 1000000

/* Macros */
#define round_up(x,n)	(-(-(x) & -(n)))
#define PROJECTION_HEIGHT 256
#define MB_SIZE		1048576.0
#define KB_SIZE     1024.0
u32 filesize(FILE * file) {
	u32 curpos, endpos;
	
	if(file == NULL)
		return 0;
	__Draw_Loading(440, 440);
	curpos = ftell(file);
	fseek(file, 0, 2);
	endpos = ftell(file);
	fseek(file, curpos, 0);
	__Draw_Loading(440, 440);
	return endpos;
}

s32 InstallFile(FILE * fp) {

	char * data;
	s32 ret, nandfile;
	u32 length = 0,numchunks, cursize, i;
	char filename[ISFS_MAXPATH] ATTRIBUTE_ALIGN(32);
	
	char tite[256];
	__Draw_Loading(440, 440);
	MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
	sprintf(tite,"[+] Installing %s .", ThemeList[orden[selectedtheme]].title);
	__Draw_Message(tite, 240, BLACK);
	//sleep(2);
		
	char* content_name = stpcpy(filename, "/title/00000001/00000002/content/");
	sprintf(content_name, "000000%s.app", getsavename(system_Version));
	
	nandfile = ISFS_Open(filename, ISFS_OPEN_RW);
	ISFS_Seek(nandfile, 0, SEEK_SET);
	__Draw_Loading(440, 440);
	length = filesize(fp);
	//if(debugcard) logfile("length of file %d \n",length);
	numchunks = length/CHUNKS + ((length % CHUNKS != 0) ? 1 : 0);
	__Draw_Loading(440, 440);
	//if(debugcard) logfile("[+] Total parts: %d\n", numchunks);
	
	for(i = 0; i < numchunks; i++)
	{
		
		data = memalign(32, CHUNKS);
		if(data == NULL)
		{
			return -1;
		}
		
		

		//if(debugcard) logfile("	Installing part %d\n",(i + 1) );

		//__Draw_Message(ms,250, WHITE_SMOKE);
		
		__Draw_Loading(440, 440);
		MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
		sprintf(tite,"Installing part %d of %d parts ,",i + 1, numchunks);
		__Draw_Message(tite, 240, BLACK);
		//sleep(2);
		
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
		__Draw_Loading(440, 440);
		wiilight(1);
		ret = ISFS_Write(nandfile, data, cursize);
		if(ret < 0)
		{
			//if(debugcard) logfile("[-] Error writing to NAND! (ret = %d)\n\n", ret);
			//gprintf("	Press any button to continue...\n");
			return ret;
		}
		free(data);
		wiilight(0);
		__Draw_Loading(440, 440);
	}
	
	ISFS_Close(nandfile);

	return 0;
}


 /*   bool checkofficialthemesig(const char * name) {
	char filepath[256];
    FILE *fp = NULL;
    u32 length, i;
    u8 *themedata = NULL;
	//int readsiglen = strlen(name);
	//char readsig[readsiglen];
	
	sprintf(filepath, "%s:/%s/%s", get_storage_name(thememode), themedir, name);
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
				if((themedata[i] == 119- 116(t)) && (themedata[i + 8] == 109)) { // t__________r
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
	
	sprintf(filepath, "%s:/themes/%s", get_storage_name(thememode), name);
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
        //if(debugcard) logfile("[-] Unable to read file !! \n");
		//if(debugcard) logfile("[-] Unable to read file !! \n");
        return 0;
    }
    else {
        for(i = 0; i < length; i++)
        {
            
			if(data[i] == 83)
            {
                //if(debugcard) logfile("data at [%d]\n", i);
				if(data[i+6] == 52)  // 4
                {
                    //if(debugcard) logfile("data at [%d]\n", i);
					if(data[i+8] == 48)  // 0
                    {
                        //if(debugcard) logfile("data at [%d]\n", i);
						if(data[i+28] == 85)  // usa
                        {
                            //if(debugcard) logfile("data at [%d]\n", i);
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
                            //if(debugcard) logfile("data at [%d]\n", i);
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
			else if(data[i] == 67) { // C (Compat) vwii
				if(data[i+6] == 52) {  // 4
					if(data[i+8] == 51) { // 3
						if(data[i+28] == 85)  // U
                        {
                            if(data[i+29] == 83) { // S
								rtn = 609;
								break;
							}
                        }
						else if(data[i+28] == 74)  //J
                        {
                            if(data[i+29] == 80) { //P
								rtn = 608;
								break;
							}
                        }
						else if(data[i+28] == 69)  // E
                        {
                            if(data[i+29] == 85) { // U
								rtn = 610;
								break;
							}
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
	
	switch(system_Version)
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
		MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
		__Draw_Message(nopriiloader, 240, BLACK);
		sleep(3);
		return MENU_SELECT_THEME;
	}
	char *start = "Starting Custom Theme Installation !";
	MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
	__Draw_Message(start, 240, BLACK);
	
	sprintf(filepath, "%s:/themes/%s", get_storage_name(thememode), ThemeList[orden[selectedtheme]].title);
	//if(debugcard) logfile("filepath (%s) \n",filepath);
	curthemestats.version = Get_system_version();
	if(curthemestats.version > 610) curthemestats.version = check_custom_system_version();
	retreivecurrentthemeregion(curthemestats.version);
	//if(debugcard) logfile("cur theme .version(%d) .region(%c) \n",curthemestats.version, curthemestats.region);
	ThemeList[orden[selectedtheme]].version = findinstallthemeversion(ThemeList[orden[selectedtheme]].title);
    installregion(ThemeList[orden[selectedtheme]].version);
	//if(debugcard) logfile("install theme .version(%d) .region(%c) \n",ThemeList[orden[selectedtheme]].version,ThemeList[orden[selectedtheme]].region);
	
	if(curthemestats.version != ThemeList[orden[selectedtheme]].version) {
        const char *badversion = "Install can not continue system versions differ ! Press any button to exit.";
		MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
		__Draw_Message(badversion, 240, BLACK);
		sleep(2);  
        sysHBC();
    }
    else if(curthemestats.region != ThemeList[orden[selectedtheme]].region) {
        const char *badregion = "Install can not continue system regions differ ! Press any button to exit.";
		MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
		__Draw_Message(badregion, 240, BLACK);
		sleep(2);  
        sysHBC();
    }
   
    fp = fopen(filepath, "rb");
    
	// Install 
    InstallFile(fp);

    // Close file 
    if(fp) fclose(fp);
		
	char *done = "Your Custom Theme has been installed !";
	MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
	__Draw_Message(done, 240, BLACK);  
	sleep(2);
	
	return MENU_EXIT;
}


int __Select_Theme(){
	int i, j, hotSpot, hotSpotPrev, ret;
	ret = MENU_EXIT;
	//u32 buttons;
	u32 outlen = 0;
	u32 http_status = 0;
	u32 Maxsize = 4294967295;
	u8* outbuf = NULL;
	MRCtex *themeImage, *projection;
	char *c, *r, a;
	char savepath[256];
	char tmpstr[128];
	if(themecnt == -1) {
		MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
		__Draw_Message("No Files or directories Found .", 240, BLACK);
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
	
	u32 buttons = -1;
	// Select game loop
	for(;;){
		hotSpot=Wpad_Scan();

		// If hot spot changed
		if(moving_Theme>-1){
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

		MRC_Draw_Cursor(Wpad_GetWiimoteX(), Wpad_GetWiimoteY(), (moving_Theme > -1));

		if(moving_Theme == -1){
			if(((WPAD_ButtonsDown(WPAD_CHAN_0) & (WPAD_BUTTON_A | WPAD_BUTTON_B)) || (PAD_ButtonsDown(0) & (PAD_BUTTON_A | PAD_TRIGGER_Z | PAD_BUTTON_B))) && hotSpot>-1 && hotSpot<COLS[wideScreen]*ROWS && orden[selectedtheme] != EMPTY){
				if(WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_B || PAD_ButtonsDown(0) & PAD_BUTTON_B){
					moving_Theme = selectedtheme;
					findnumpages();
				}else if((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_A) || (PAD_ButtonsDown(0) & PAD_BUTTON_A)){
					ret = MENU_SHOW_THEME;
					//logfile("select theme - going to show theme - page[%d] selectedtheme[%d]\n", page, selectedtheme);
					break;
				}
			}
		}
		else{
			//moving game
			if(!(WPAD_ButtonsHeld(WPAD_CHAN_0) & WPAD_BUTTON_B) || PAD_ButtonsHeld(0) & PAD_BUTTON_B){
				if(selectedtheme != -1 && selectedtheme != moving_Theme){
					u16 copia0 = orden[moving_Theme];
					u16 copia1 = orden[selectedtheme];

					orden[moving_Theme] = copia1;
					orden[selectedtheme] = copia0;
					saveconfig = true;
				}
				moving_Theme = -1;
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
				MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
				__Draw_Message("Loading Page .     Please Wait ...", 240, BLACK);
				break;
			}
			else if(((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_PLUS) || (PAD_ButtonsDown(0) & PAD_TRIGGER_R)) || (hotSpot==HOTSPOT_RIGHT && ((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_A) || (PAD_ButtonsDown(0) & PAD_BUTTON_A)))){
				page++;
				if (page >= maxPages)
					page = 0;
				ret = MENU_SELECT_THEME;
				MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
				__Draw_Message("Loading Page .     Please Wait ...", 240, BLACK);
				break;
			}
			else if(((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_HOME) || (PAD_ButtonsDown(0) & PAD_BUTTON_START))){
				ret = MENU_HOME;
				break;
			}
			/*else if((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_B) || (PAD_ButtonsDown(0) & PAD_BUTTON_B)){
				ret = MENU_SELECT_THEME;
				break;
			}*/
			else if((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_1) || (PAD_ButtonsDown(0) & PAD_BUTTON_Y)){
				if(!downloadable_theme_List) {
					if(ThemeList[selectedtheme].title != NULL) {
						MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
						__Draw_Message("Delete file ?", 235, BLACK);
						__Draw_Message("[A] - Confirm     [B] - Return", 255, BLACK);
						buttons = Wpad_WaitButtons();
						if(buttons == WPAD_BUTTON_A || buttons == PAD_BUTTON_A) {
							//logfile("delete file here.");
							char command[128];
							sprintf(command, "%s:/themes/%s", get_storage_name(thememode), ThemeList[selectedtheme].title);
							remove(command);
							if(themecnt >= 0) __Free_Channel_Images();
							themecnt = filelist_retrieve(downloadable_theme_List, 0);
							__Set_Theme_Order();
							if(themecnt <= 0) ret = MENU_HOME;
							ret = MENU_SELECT_THEME;
							break;
						}
						else if(buttons == WPAD_BUTTON_B || buttons == PAD_BUTTON_B) { ret = MENU_SELECT_THEME; break; }
					}
				}
				else {
					//logfile("ThemeList[selectedtheme].has_banner ->> %d", ThemeList[selectedtheme].has_banner);
					if(ThemeList[selectedtheme].has_banner == false) {
						//logfile("theme ->> %s\nnetconnection ->> ", ThemeList[selectedtheme].title, netconnection);
						if(netconnection) {
							if(netconnection) {
								MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
								__Draw_Message("Downloading Image . Please wait .", 235, BLACK);
								sprintf(tempString,"http://www.wiithemer.org/resources/wii/main/%s" ,ThemeList[orden[selectedtheme]].png);
								//if(debugcard) logfile("tempstring = %s\n", tempString);
								display_progress = true;
								ret = http_request(tempString, Maxsize, display_progress);
								display_progress = false;
								if(ret != 0 ) {
									ret = http_get_result(&http_status, &outbuf, &outlen);
									if(ret != 0 ) {
										if(outlen > 0 && http_status == 200) {
											//if(debugcard) logfile("outlen = %d\ndraw image here", outlen);
											a=250;
											for(i=0; i<480; i++) {
												if(a<255 && ((i<100 && i%4==0) || (i>200 && i%8==0)))
													a++;
												MRC_Draw_Box(0, i, 640, 1, 0x20202000+a);
											}
											themeImage = MRC_Load_Texture(outbuf);
											MRC_Resize_Texture(themeImage, 640, 400);
											__MaskBanner(themeImage);
											projection = allocate_memory(sizeof(MRCtex));
											projection->buffer=allocate_memory(themeImage->width*PROJECTION_HEIGHT*4);
											projection->width=640;
											projection->height=PROJECTION_HEIGHT;
											MRC_Center_Texture(projection, 0);
											projection->alpha = true;
											__MaskBanner(projection);
											
											a = 225;
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
											MRC_Draw_Texture(10, 0, themeImage);
											MRC_Draw_Texture(10, 530, projection);
											MRC_Draw_String(((640 - strlen("[B] - Back")*8)/2), 420, WHITE, "[B] - Back");
											
											MRC_Render_Screen();
											MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
											sprintf(tmpstr, "Saving %s Theme Image .", ThemeList[selectedtheme].title);
											__Draw_Message(tmpstr, 240, BLACK);
											sleep(1);
											sprintf(savepath,"%s:/apps/thememanager/imgs", get_storage_name(thememode));
											if(!Fat_CheckDir(savepath))
												Fat_CreateSubfolder(savepath);
											//__Draw_Loading(440, 440);
											sprintf(savepath,"%s:/apps/thememanager/imgs/%s", get_storage_name(thememode), ThemeList[selectedtheme].png);
											ret = Fat_SaveFile(savepath, (void *)&outbuf, outlen);
											MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
											sprintf(tmpstr, "Saving %s Theme Image Complete .", ThemeList[selectedtheme].title);
											__Draw_Message(tmpstr, 240, BLACK);
											sleep(2);
											MRC_Draw_Texture(10, 0, themeImage);
											MRC_Draw_Texture(10, 530, projection);
											MRC_Draw_String(((640 - strlen("[B] - Back")*8)/2), 420, WHITE, "[B] - Back");
											MRC_Render_Screen();
											for(;;) {
												buttons = Wpad_WaitButtons();
												if(buttons == WPAD_BUTTON_B || buttons == PAD_BUTTON_B) {
													MRC_Free_Texture(themeImage);
													MRC_Free_Texture(projection);
													break;
												}
											}
										}
									}
								}
								if(themecnt >= 0) __Free_Channel_Images();
								themecnt = 0;
								themecnt = filelist_retrieve(downloadable_theme_List, 0);
								__Set_Theme_Order();
								ret = MENU_SELECT_THEME;
								break;
							}
						}
					}
				}
			}
			else if((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_2) || (PAD_ButtonsDown(0) & PAD_BUTTON_X)) {
				if(downloadable_theme_List) {
					logfile("put the abc filter here\n");
					const char *filter_display[27] = { "-", "A", "B", "C", "D","E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z"};
					int start = 64;
					int filter_num = 0;
					char drawmessage[64];
					ret = MENU_SELECT_THEME;
					
					for(;;) {
						
						MRC_Draw_Box(120, 165, 400, 150, BLACK);
						MRC_Draw_String2((640-strlen("Filter Themes :")*8)/2, 210, WHITE, "Filter Themes :");
						sprintf(drawmessage, "%s", filter_display[filter_num]);
						MRC_Draw_String2((640-strlen(drawmessage)*8)/2, 250, WHITE, drawmessage);
						MRC_Render_Screen();
						
						Wpad_Scan();
						PAD_ScanPads();
						
						if((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_B) || (PAD_ButtonsDown(0) & PAD_BUTTON_B)) break; 
						if((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_LEFT) || (PAD_ButtonsDown(0) & PAD_BUTTON_LEFT)) {
							filter_num--;
							if(filter_num <= 0) filter_num = 26;
						}
						if((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_RIGHT) || (PAD_ButtonsDown(0) & PAD_BUTTON_RIGHT)) {
							filter_num++;
							if(filter_num >= 27) filter_num = 0;
						}
						if((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_A) || (PAD_ButtonsDown(0) & PAD_BUTTON_A)) {
							logfile("USE this filter type.\n");
							if(themecnt >= 0) __Free_Channel_Images();
							themecnt = 0;
							__Draw_Loading(440, 440);
							themecnt = filelist_retrieve(downloadable_theme_List, filter_num == 0 ? filter_num : filter_num+start);
							__Draw_Loading(440, 440);
							if(themecnt == 0) {
								logfile("no files found with filter [%c]\n", filter_num+start);
								sprintf(drawmessage, "No Themes found with filter[%c]", filter_num+start);
								MRC_Draw_String((640-strlen(drawmessage)*8)/2, 290, WHITE, drawmessage);
								MRC_Render_Screen();
								sleep(3);
							}
							else {
								__Set_Theme_Order();
								break;
							}
						}
					}
					break;
				}
			}
		}
	}

	return ret;
}
void get_title_key(signed_blob *s_tik, u8 *key){
    static u8 iv[16] ATTRIBUTE_ALIGN(0x20);
    static u8 keyin[16] ATTRIBUTE_ALIGN(0x20);
    static u8 keyout[16] ATTRIBUTE_ALIGN(0x20);
	u8 wii_common_key[16] = { 0xeb, 0xe4, 0x2a, 0x22, 0x5e, 0x85, 0x93, 0xe4, 0x48,0xd9, 0xc5, 0x45, 0x73, 0x81, 0xaa, 0xf7 };
	u8 vWii_common_key[16] = { 0x30, 0xBF, 0xC7, 0x6E, 0x7C, 0x19, 0xAF, 0xBB, 0x23, 0x16, 0x33, 0x30, 0xCE, 0xD7, 0xC2, 0x8D };
	
    const tik *p_tik;
    p_tik = (tik*) SIGNATURE_PAYLOAD(s_tik);
    u8 *enc_key = (u8 *) &p_tik->cipher_title_key;
    memcpy(keyin, enc_key, sizeof keyin);
    memset(keyout, 0, sizeof keyout);
    memset(iv, 0, sizeof iv);
    memcpy(iv, &p_tik->titleid, sizeof p_tik->titleid);

    if(!system_is_vWii)
		aes_set_key(wii_common_key);
	else
		aes_set_key(vWii_common_key);
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
        if(system_is_vWii) return "http://ccs.cdn.wup.shop.nintendo.net/ccs/download/0000000700000002/cetk";
		else return "http://nus.cdn.shop.wii.com/ccs/download/0000000100000002/cetk";
        break;
    case 1:
        if(system_is_vWii) return "http://ccs.cdn.wup.shop.nintendo.net/ccs/download/0000000700000002/tmd.";
		else return "http://nus.cdn.shop.wii.com/ccs/download/0000000100000002/tmd.";
        break;
	case 2:
        return "http://nus.cdn.shop.wii.com/ccs/download/0000000100000002/0000006f";
        break;
    case 3:
        return "http://nus.cdn.shop.wii.com/ccs/download/0000000100000002/00000072";
        break;
    case 4:
        return "http://nus.cdn.shop.wii.com/ccs/download/0000000100000002/00000075";
        break;
    case 5:
        return "http://nus.cdn.shop.wii.com/ccs/download/0000000100000002/00000078";
        break;
    case 6:
        return "http://nus.cdn.shop.wii.com/ccs/download/0000000100000002/0000007b";
        break;
    case 7:
        return "http://nus.cdn.shop.wii.com/ccs/download/0000000100000002/0000007e";
        break;
    case 8:
        return "http://nus.cdn.shop.wii.com/ccs/download/0000000100000002/00000081";
        break;
    case 9:
        return "http://nus.cdn.shop.wii.com/ccs/download/0000000100000002/00000084";
        break;
    case 10:
        return "http://nus.cdn.shop.wii.com/ccs/download/0000000100000002/00000087";
        break;
    case 11:
        return "http://nus.cdn.shop.wii.com/ccs/download/0000000100000002/0000008a";
        break;
    case 12:
        return "http://nus.cdn.shop.wii.com/ccs/download/0000000100000002/0000008d";
        break;
    case 13:
        return "http://nus.cdn.shop.wii.com/ccs/download/0000000100000002/00000094";
        break;
    case 14:
        return "http://nus.cdn.shop.wii.com/ccs/download/0000000100000002/00000097";
        break;
    case 15:
        return "http://nus.cdn.shop.wii.com/ccs/download/0000000100000002/0000009a";
        break;
    case 16:
        return "http://nus.cdn.shop.wii.com/ccs/download/0000000100000002/0000009d";
        break;
	case 17:
        return "http://ccs.cdn.wup.shop.nintendo.net/ccs/download/0000000700000002/0000001c";
        break;
	case 18:
        return "http://ccs.cdn.wup.shop.nintendo.net/ccs/download/0000000700000002/0000001f";
        break;
	case 19:
        return "http://ccs.cdn.wup.shop.nintendo.net/ccs/download/0000000700000002/00000022";
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
        return 2;
        break;
    case 417:
        return 3;
        break;
    case 418:
        return 4;
        break;
    case 448:
        return 5;
        break;
    case 449:
        return 6;
        break;
    case 450:
        return 7;
        break;
    case 454:
        return 8;
        break;
    case 480:
        return 9;
        break;
    case 481:
        return 10;
        break;
    case 482:
        return 11;
        break;
    case 486:
        return 12;
        break;
    case 512:
        return 13;
        break;
    case 513:
        return 14;
        break;
    case 514:
        return 15;
        break;
    case 518:
        return 16;
        break;
	case 608:
		return 17;
		break;
	case 609:
		return 18;
		break;
	case 610:
		return 19;
		break;
    default:
        return -1;
        break;
    }
}
int __downloadApp()  {
	//__Draw_Loading(440, 440);
	//if(debugcard) logfile("__downloadApp - thememode[%d] downloadable_theme_List[%d] netconnection[%d]\n", thememode, downloadable_theme_List, netconnection);
	char *tmpstr = (char*)memalign(32,256);
	int ret, message_y = 240; //, retries;
	if(!netconnection) {
		if(debugcard) logfile("Internet connection not detected .\n");
		MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
		__Draw_Message("Internet Connection not detected .", message_y, BLACK);
		sleep(2);
		return MENU_HOME;
	}
	u32 tmpversion;
    
    int counter;
    char *savepath = (char*)malloc(256);
	u8* outbuf = NULL;
	char *app_path = (char*)memalign(32,256);
	signed_blob * s_tik = NULL;
    signed_blob * s_tmd = NULL;
	
    tmpversion = Get_system_version();
    if(tmpversion > 610) tmpversion = check_custom_system_version();
	sprintf(app_path,"%s:/themes/%s_bkup.app", get_storage_name(thememode), get_content_name_noextension(tmpversion));
	if(!Fat_CheckFile(app_path)){
		sprintf(tmpstr,"Downloading Content %s.app for System Menu v%u ", get_content_name_noextension(tmpversion), tmpversion);
		MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
		__Draw_Message(tmpstr, message_y, BLACK);
		sleep(1);
		sprintf(tmpstr,"%s:/themes", get_storage_name(thememode));
		if(!Fat_CheckDir(tmpstr)) Fat_MakeDir(tmpstr);
		for(counter = 0; counter < 3; counter++) {	
			__Draw_Loading(440, 440);
			if(counter == 2) {
				sprintf(tmpstr,"Downloading Content %s.app for System Menu v%u .", get_content_name_noextension(tmpversion), tmpversion);
				MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
				message_y = 235;
				__Draw_Message(tmpstr, message_y, BLACK);
			}
			message_y = 240;
			sleep(1);
			char *path = (char*)memalign(32, 256);
			int aa = getslot(tmpversion);
			if(counter == 0) sprintf(path,"%s",getPath(counter));
			if(counter == 1) sprintf(path,"%s%u",getPath(counter), tmpversion);
			if(counter == 2) sprintf(path,"%s",getPath(aa));
			//if(debugcard) logfile("path[%s]\n", path);
			u32 outlen = 0;
			u32 http_status = 0;
			u32 Maxsize = 0xFFFFFFFF;
			if(counter >= 2)
				display_progress = true;
				
			ret = http_request(path, Maxsize, display_progress);
			
			free(path);
			display_progress = false;
			//if(counter == 0 || counter == 1) 
			//	sprintf(tmpstr,"DownLoading %s Complete .", appfilename[counter]);
			//else
			if(counter == 2) {
				sprintf(tmpstr,"DownLoading Content %s.app Complete .", get_content_name_noextension(tmpversion));
				MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
				__Draw_Message(tmpstr,message_y, BLACK);
			} 
			//if(counter == 0 || counter == 1)
			//	sleep(1);
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
			
			//MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
			//__Draw_Message("Decrypting file .", message_y, BLACK);
			//sleep(1);
			//set aes key
			u8 key[16];
			u16 index;
			get_title_key(s_tik, key);
			aes_set_key(key);
			u8* outbuf2 = (u8*)malloc(outlen);
			//MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
			//__Draw_Message("Decrypting file Complete .", message_y, BLACK);
			//sleep(1);
			if(counter == 2) {
				if(outlen > 0) {//suficientes bytes
					index = 01;
					//then decrypt buffer
					decrypt_buffer(index,outbuf,outbuf2,outlen);
					sprintf(savepath,"%s:/themes/%s_bkup.app", get_storage_name(thememode), get_content_name_noextension(tmpversion));
					sprintf(tmpstr,"Saving Content %s.app for System Menu v%u .", get_content_name_noextension(tmpversion), tmpversion);
					MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
					__Draw_Message(tmpstr,message_y, BLACK);
					sleep(1);
					ret = Fat_SaveFile(savepath, (void *)&outbuf2, outlen);
					if(ret <= 0) {
						sprintf(tmpstr,"Saving Content %s.app for System Menu v%u  Failed .", get_content_name_noextension(tmpversion), tmpversion);
						MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
						__Draw_Message(tmpstr,message_y, BLACK);
						sleep(2);
						goto end;
					}
				}
			}
			
			if(outbuf!=NULL)
				free(outbuf);
		}
		//net_deinit();
		
		sprintf(tmpstr,"Saving Content %s.app for System Menu v%u Complete .", get_content_name_noextension(tmpversion), tmpversion);
		MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
		__Draw_Message(tmpstr,message_y, BLACK);
		sleep(2);
	}
	else {
		sprintf(tmpstr,"Content %s.app found .", get_content_name_noextension(tmpversion));
		MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
		__Draw_Message(tmpstr,message_y, BLACK);
		sleep(2);
	}
end:	
	free(savepath);
	free(tmpstr);
	free(app_path);
	free(outbuf);
	if(themecnt >= 0) __Free_Channel_Images();
	themecnt = filelist_retrieve(downloadable_theme_List, 0);
	__Set_Theme_Order();
	page = (selectedtheme+1)/12;
	return MENU_SELECT_THEME;
}


bool is_theme_2stage(int input_theme) {
	char *str;
	str = strstr(ThemeList[input_theme].mym, "stage1");
	if(str) return true;
	//if (debugcard) logfile("theme =%s\nstr =%d\n", Theme_Mym_File[input_theme], str);
	return false;
}
bool is_theme_region_specific(int input_theme) {
	switch(input_theme) { 
		case 70:
		case 73:
		case 74:
		case 75:
		case 76:
		case 77:
		case 78:
		case 79:
		case 80:
		case 124:
		case 312:
			return true;
			break;
		default:
			return false;
			break;
	}
	return false;
}
int retrieve_themefilesize(int pos) {
	//char *size = "0";
	int ret, size1 = 0, basetheme_len = 0;
	char sitepath[128];
	const char *siteUrl = "http://www.wiithemer.org/resources/mym/";
	//u32 outlen = 0;
	//u32 http_status = 0;
	u32 Maxsize = 0xFFFFFFFF;
	//u8* outbuf = NULL;
	
	bool mym_type;
	bool is2stage;
	char theme_noextention[64];
	//content_length = 0;
	
	__Draw_Loading(440, 440);
	is2stage = is_theme_2stage(pos);
	__Draw_Loading(440, 440);
	mym_type = is_theme_region_specific(pos);
	//if(debugcard) logfile("is2stage = %d\n", is2stage);
	__Draw_Loading(440, 440);
	if(mym_type) sprintf(sitepath, "%s%s%s.mym", siteUrl, ThemeList[pos].mym, get_display_region(system_Version));
	else sprintf(sitepath, "%s%s", siteUrl, ThemeList[pos].mym);
	__Draw_Loading(440, 440);
	//if(debugcard) logfile("sitepath[%s]\n", sitepath);
	ret = http_request_content_length(sitepath, Maxsize);
	//logfile("ret1[%d]\n", ret);
	__Draw_Loading(440, 440);
	if(is2stage) {
		__Draw_Loading(440, 440);
		basetheme_len = strlen(ThemeList[pos].mym);
		snprintf(theme_noextention, basetheme_len-4, ThemeList[pos].mym);
		//logfile("theme_noextention= %s\n", theme_noextention);
		size1 = ret;
		sprintf(sitepath, "%s%s2.mym", siteUrl, theme_noextention); 
		ret = http_request_content_length(sitepath, Maxsize);
		//logfile("ret2[%d]\n", ret);
		__Draw_Loading(440, 440);
		*theme_noextention = 0;
	}
	
	*sitepath = 0;
	__Draw_Loading(440, 440);
	return ret + size1;
}
int retrieve_downloadcount(int pos) {
	char *count = NULL;
	int ret = -2;
	char sitepath[128];
	const char *siteUrl = "http://www.wiithemer.org/resources/stats/indthemecnt/";
	u32 outlen = 0;
	u32 http_status = 0;
	u32 Maxsize = 0xFFFFFFFF;
	u8* outbuf = NULL;
	int convert_to_int = 0;
	//content_length = 0;
	//, pathlen = 0;
	//bool mym_type;
	//if(debugcard) logfile("in retreive downloadcount\n");
	
	//char *d1;
	//mym_type = is_theme_region_specific(pos);
	__Draw_Loading(440, 440);
	//if(mym_type) sprintf(sitepath, "%s%s%s.txt", siteUrl, Theme_Mym_File[pos], get_display_region(system_Version));
	//else 
	//snprintf(d1, strlen(txt)-1, "%s", txt);
	//logfile("d1[%s]\n", d1);
	sprintf(sitepath, "%s%s", siteUrl, ThemeList[pos].downloads);
	
	if(debugcard) logfile("download count sitepath[%s]\n", sitepath);
	ret = http_request(sitepath, Maxsize, display_progress);
	
	if(ret) {
		
		ret = http_get_result(&http_status, &outbuf, &outlen);
		
		if(ret) {
			if(outlen > 0 && http_status == 200) {
				
				count = (char*)outbuf;
				count[outlen] = '\0';
				//if(debugcard) logfile("count = %s\n", count);
				convert_to_int = atoi(count);
				//if(debugcard) logfile("convert_to_int[%d]\n", convert_to_int);
				//sprintf(count, "%d", convert_to_int);
				//if(debugcard) logfile("count[%s]\n", count);
			}
		}
	}
	if(outbuf) free(outbuf);
	*sitepath = 0;
	__Draw_Loading(440, 440);
	return convert_to_int;
}
int __Show_Theme(){
	void* imageBuffer;
	MRCtex *themeImage, *projection, *themeImage2, *projection2;
	int i, j, ret, size = 0, downloadcount = 0;
	char *c, *r, a;
	//int count = -1;
	//char *countstr = NULL;
	//char *sizestr = NULL;
	char file_size[128];
	ModTheme *thetheme = &ThemeList[selectedtheme];
	char filepath[256];
	//char checkpath[256];
	//char tmpstr[128];
	FILE *fptr = NULL;
	u32 outlen = 0, buttons;
	u32 http_status = 0;
	u32 Maxsize = 0xFFFFFFFF;
	u8* outbuf = NULL;
	//char *officialthemes[5] = { "Unsigned Theme", "Original Theme Unmodified", "Wii Themer Signed", "Theme Manager Signed", "ModMii Signed"};
	//if(!sigchecked) checkofficialthemesig(thetheme->title);
	
	// BLACK SCREEN
	/*a=160;
	for(i=0; i<480; i++){
		if(a<255 && ((i<208 && i%4==0) || i%8==0))
			a++;
		MRC_Draw_Box(0, i, 640, 1, a);
	}*/
	//if(!netconnection) netconnection = checkNinit_netconnection();
	if(downloadable_theme_List) {
		//__Draw_Loading(440, 440);
		if(netconnection) {
			if(thetheme->size == 0) {
				size = retrieve_themefilesize(selectedtheme);
				if(size > 0) ThemeList[selectedtheme].size = size;
			}
			else size = thetheme->size;
			/*
			logfile("1st downloads[%d}\n", thetheme->downloadcount);
			if(thetheme->downloadcount == 0) {
				downloadcount = retrieve_downloadcount(selectedtheme);
				logfile("2nd downloads[%d}\n", downloadcount);
				if(downloadcount > 0) ThemeList[selectedtheme].downloadcount = downloadcount;
			}
			else
				downloadcount = thetheme->downloadcount;
			logfile("3rd downloads[%d}\n", downloadcount);*/
		}
		sprintf(file_size, "Size: %.2f MB  Downloads : %d", size/MB_SIZE, downloadcount);
	}
	else {
		//__Draw_Loading(440, 440);
		sprintf(filepath, "%s:/%s/%s", get_storage_name(thememode), themedir , thetheme->title);
		//if(debugcard) logfile("filepath[%s]\n", filepath);
		fptr = fopen(filepath, "rb");
		if(!fptr) logfile("unable to open path[%s]\n", filepath);
		size = filesize(fptr);
		fclose(fptr);
		 
		sprintf(file_size, "Size: %.2f MB", size/MB_SIZE);
	}
	//__Draw_Loading(440, 440);
	// ANOTHER SCREEN FADE TYPE
	a=250;
	for(i=0; i<480; i++) {
		if(a<255 && ((i<100 && i%4==0) || (i>200 && i%8==0)))
			a++;
		MRC_Draw_Box(0, i, 640, 1, 0x20202000+a);
	}

	
	// Load image from FAT
	if(ThemeList[selectedtheme].type == 20)
		sprintf(tempString,"%s:/apps/thememanager/imgs/theme_manager_installer_empty.png",get_storage_name(thememode));
	else if(ThemeList[selectedtheme].type == 10)
		sprintf(tempString,"%s:/apps/thememanager/imgs/%s" , get_storage_name(thememode), ThemeList[selectedtheme].png);
		
	//logfile("tempstring %s \n",tempString);
	ret = Fat_ReadFile(tempString, &imageBuffer, false);
	//logfile("ret fat read file ->> %d\n", ret);
	// Decode image
	if(ret > 0) {
		themeImage = MRC_Load_Texture(imageBuffer);
		free(imageBuffer);

		MRC_Resize_Texture(themeImage, 640, 375);
		__MaskBanner(themeImage);
		
		projection = allocate_memory(sizeof(MRCtex));
		projection->buffer=allocate_memory(themeImage->width*PROJECTION_HEIGHT*4);
		projection->width=640;
		projection->height=PROJECTION_HEIGHT;
		MRC_Center_Texture(projection, 0);
		projection->alpha = true;
		__MaskBanner(projection);
		
		a = 225;
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
	
		MRC_Draw_Texture(10, 0, themeImage);
		MRC_Draw_Texture(10, 510, projection);
		
		//logfile("downloadable_theme_List ->> %d\nnetconnection ->> %d\n", downloadable_theme_List, netconnection);
		if(downloadable_theme_List) {
			//logfile("in downloadable theme list in ret from read fat file .\nnetconnection = %d\n", netconnection);
			if(netconnection) {
				//MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
				MRC_Draw_String(((640-strlen(file_size)*8)/2), 450, WHITE, file_size);
				MRC_Draw_String(((640-strlen(thetheme->title)*8)/2), 420, WHITE, thetheme->title);
				MRC_Draw_String((640-strlen("[1/Y] - Show Image 2")*8)-10, 450, WHITE, "[1/Y] - Show Image 2");
				sprintf(tempString, "%s", (downloadable_theme_List == 1 ? "[A] - Download Theme" : "[A] - Install Theme"));
				MRC_Draw_String(40, 420, WHITE, tempString);
			}
			else {
				sprintf(tempString, "%s", (downloadable_theme_List == 1 ? "" : "[A] - Install Theme"));
				MRC_Draw_String(40, 420, WHITE, tempString);
				MRC_Draw_String(((640-strlen(thetheme->title)*8)/2), 420, WHITE, thetheme->title);
			}
		}
		else {
			//MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
			MRC_Draw_String(((640-strlen(thetheme->title)*8)/2), 420, WHITE, thetheme->title);
			MRC_Draw_String(((640-strlen(file_size)*8)/2), 450, WHITE, file_size);
		}
		
		
		
		MRC_Draw_String(40, 450, WHITE, "[B] - Back");
		MRC_Free_Texture(themeImage);
		MRC_Free_Texture(projection);
	}
	else{
		
		
		
		MRC_Resize_Texture(textures[TEX_NO_IMG], 640, 380);
		MRC_Draw_Texture(10, 0, textures[TEX_NO_IMG]);
		__MaskBanner(textures[TEX_NO_IMG]);
		MRC_Draw_String(((640-strlen(thetheme->title)*8)/2), 390, WHITE, thetheme->title);
		//if(ThemeList[selectedtheme].type == 20)
		//else if(ThemeList[selectedtheme].type == 10)
		if(downloadable_theme_List) {
			//MRC_Draw_String(((640-strlen(thetheme->title)*8)/2), 420, BLACK, thetheme->title);
			if(netconnection) {
				MRC_Draw_String(((640-strlen(file_size)*8)/2), 450, WHITE, file_size);
				MRC_Draw_String((640-strlen("[1/Y] - Show Image 2")*8)-10, 420, WHITE, "[1/Y] - Show Image 2");
			}
		}
		else {
			MRC_Draw_String(((640-strlen(file_size)*8)/2), 450, WHITE, file_size);
			
			//sprintf(tempString, "Theme Signature : %s", officialthemes[official_theme]);
			//MRC_Draw_String(((640-strlen(tempString)*8)/2), 380, WHITE, tempString);
		}
		if(!netconnection) sprintf(tempString, "%s", (downloadable_theme_List == 1 ? "" : "[A] - Install Theme"));
		else sprintf(tempString, "%s", (downloadable_theme_List == 1 ? "[A] - Download Theme" : "[A] - Install Theme"));
		MRC_Draw_String(30, 420, WHITE, tempString);
		MRC_Draw_String(30, 450, WHITE, "[B] - Back");
	}
	MRC_Render_Screen();
	
	ret = MENU_SELECT_THEME;
	for(;;) {
		WPAD_ScanPads();
		PAD_ScanPads();
		if(WPAD_ButtonsDown(WPAD_CHAN_0) || PAD_ButtonsDown(0)){
			if((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_LEFT) || (PAD_ButtonsDown(0) & PAD_BUTTON_LEFT)){
				if(selectedtheme <= 0) selectedtheme = themecnt;
				selectedtheme--;
				ret = MENU_SHOW_THEME;
				//logfile("prev page - page[%d] selectedtheme[%d]\n", page, selectedtheme);
				break;
			}
			if((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_RIGHT) || (PAD_ButtonsDown(0) & PAD_BUTTON_RIGHT)){
				selectedtheme++;
				if(selectedtheme >= themecnt) selectedtheme = 0;
				ret = MENU_SHOW_THEME;
				
				//logfile("next page - page[%d] selectedtheme[%d]\n", page, selectedtheme);
				break;
			}
			if((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_A) || (PAD_ButtonsDown(0) & PAD_BUTTON_A)){
				ret = MENU_INSTALL_THEME;
				break;
			}
			if((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_B) || (PAD_ButtonsDown(0) & PAD_BUTTON_B)) {
				page = selectedtheme/12;
				//logfile("going back from show theme - page[%d] selectedtheme[%d]\n", page, selectedtheme);
				//ret = MENU_SELECT_THEME;
				//official_theme = 0;
				//sigchecked = false;
				break;
			}
			if(WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_1 || (PAD_ButtonsDown(0) & PAD_BUTTON_Y)) {
				
				//if(debugcard) logfile("add more png views here .");
				a=250;
				if(downloadable_theme_List)
				if(netconnection) {
					
					MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
					__Draw_Message("Downloading Image . Please wait .", 235, BLACK);
					sprintf(tempString,"http://www.wiithemer.org/resources/wii/secondary/%s" ,ThemeList[selectedtheme].png);
					//if(debugcard) logfile("tempstring = %s\n", tempString);
					display_progress = true;
					ret = http_request(tempString, Maxsize, display_progress);
					display_progress = false;
					if(ret != 0 ) {
						ret = http_get_result(&http_status, &outbuf, &outlen);
						if(ret != 0 ) {
							if(outlen > 0 && http_status == 200) {
								//if(debugcard) logfile("outlen = %d\ndraw image here", outlen);
								for(i=0; i<480; i++) {
									if(a<255 && ((i<100 && i%4==0) || (i>200 && i%8==0)))
										a++;
									MRC_Draw_Box(0, i, 640, 1, 0x20202000+a);
								}
								themeImage2 = MRC_Load_Texture(outbuf);
								MRC_Resize_Texture(themeImage2, 640, 400);
								__MaskBanner(themeImage2);
								projection2 = allocate_memory(sizeof(MRCtex));
								projection2->buffer=allocate_memory(themeImage2->width*PROJECTION_HEIGHT*4);
								projection2->width=640;
								projection2->height=PROJECTION_HEIGHT;
								MRC_Center_Texture(projection2, 0);
								projection2->alpha = true;
								__MaskBanner(projection2);
								
								a = 225;
								r = (projection2->buffer);
								for(i=0; i<PROJECTION_HEIGHT; i++){
									c=(themeImage2->buffer)+(((themeImage2->height-1)-i*2)*themeImage2->width)*4;
									for(j=0; j<themeImage2->width; j++){
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
								MRC_Draw_Texture(10, 0, themeImage2);
								MRC_Draw_Texture(10, 530, projection2);
								MRC_Draw_String(((640 - strlen("[B] - Back")*8)/2), 450, WHITE, "[B] - Back");
								MRC_Free_Texture(themeImage2);
								MRC_Free_Texture(projection2);
								MRC_Render_Screen();
								for(;;) {
									buttons = Wpad_WaitButtons();
									//if(debugcard) logfile("buttons [%u]\n", buttons);
									if(buttons == WPAD_BUTTON_B || buttons == PAD_BUTTON_B) break;
								}
							}
						}
					}
					ret = MENU_SHOW_THEME;
					if(outbuf) free(outbuf);
				}
			}
		}
		
	}
	return ret;
}
int __Swap_Menu() {
	int i, hotSpot, hotSpotPrev, ret = -1, SWAP_MENU_X = 240, SWAP_MENU_Y = 250, SWAP_MENU_BUTTON_WIDTH = 150, SWAP_MENU_BUTTON_HEIGHT = 20, SWAP_MENU_BUTTON_SEPARATION = 20;
	bool repaint = true;
	//bool showinstructions = false;
	const char *swaps[2] = { "Installer", "Downloader" };
	
	// Create/restore hotspots
	Wpad_CleanHotSpots();
	
	for(i = 0; i < 2; i++){
		Wpad_AddHotSpot(i,
			SWAP_MENU_X,
			SWAP_MENU_Y+i*(SWAP_MENU_BUTTON_HEIGHT+SWAP_MENU_BUTTON_SEPARATION),
			SWAP_MENU_BUTTON_WIDTH,
			SWAP_MENU_BUTTON_HEIGHT,
			(i == 0 ? 1: i - 1),
			(i == 1 ? 0 : i + 1),
			(i == 0 ? 1: i - 1),
			(i == 1 ? 0 : i + 1)
		);
	}
	// Loop
	hotSpot = hotSpotPrev = -1;

	
	for(;;) {
		hotSpot = Wpad_Scan();
		
		MRC_Draw_Texture(0, 0, textures[TEX_BACKGROUND]);
		
		sprintf(tempString, "System Menu v%s_%s %u", get_system_version_Display(system_Version), get_display_region(system_Version), system_Version);
		MRC_Draw_String(((640-strlen(tempString)*8)/2), 20, WHITE, tempString);
		sprintf(tempString, "IOS %i", IOS_GetVersion());
		MRC_Draw_String(20, 20, WHITE, tempString);
		MRC_Draw_String(20, 450, WHITE, "[A] - Select Mode");
		MRC_Draw_String((640-strlen("[B] - Return")*8)-5, 450, WHITE, "[B] - Return");
		MRC_Draw_String2((640-strlen("Choose Mode :")*8)/2,160, WHITE, "Choose Mode :");
		
		// If hot spot changed
		if(((hotSpot != hotSpotPrev) && (hotSpot < 2)) || repaint){
			hotSpotPrev = hotSpot;

			for(i = 0; i < 2; i++){
				__Draw_Button(i, swaps[i], hotSpot == i);
			}
			repaint = false;
		}
		MRC_Draw_Cursor(Wpad_GetWiimoteX(), Wpad_GetWiimoteY(), 0);
		
		
		if(((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_A) || (PAD_ButtonsDown(0) & PAD_BUTTON_A)) && hotSpot != -1) {
			
			//if(debugcard) logfile("hotspot[%i]\n", hotSpot);
			downloadable_theme_List = hotSpot;
			break;
		}
		else if(((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_B) || (PAD_ButtonsDown(0) & PAD_BUTTON_B))) {
			ret = MENU_SELECT_THEME;
			
			break;
		}
		else repaint = true;
	}
	return ret;
}
int __Home() {
	int i, hotSpot, hotSpotPrev, ret, HOME_BUTTON_X = 210, HOME_BUTTON_Y = 125, HOME_BUTTON_WIDTH = 220, HOME_BUTTON_HEIGHT = 30, HOME_BUTTON_SEPARATION = 15;
	bool repaint = true;
	
	// Create/restore hotspots
	Wpad_CleanHotSpots();
	for(i = 0; i < 5; i++){
		Wpad_AddHotSpot(i,
			HOME_BUTTON_X,
			HOME_BUTTON_Y+i*(HOME_BUTTON_HEIGHT+HOME_BUTTON_SEPARATION),
			HOME_BUTTON_WIDTH,
			HOME_BUTTON_HEIGHT,
			(i == 0 ? 4 : i - 1),
			(i == 4 ? 0 : i + 1),
			(i == 0 ? 4 : i - 1),
			(i == 4 ? 0 : i + 1)
		);
	}
	
	__Draw_Window(HOME_BUTTON_WIDTH + 30, 300, "Options");
	sprintf(tempString, "Device : %s", get_storage_name(thememode));
	MRC_Draw_String(((640-strlen(tempString)*8)/2), 350, BLACK, tempString);
	__Draw_Net_Connection(535, 5, netconnection);
	if(downloadable_theme_List) MRC_Draw_String(((640-strlen("Mode -> Downloader")*8)/2), 380, BLACK, "Mode -> Downloader");
	else MRC_Draw_String(((640-strlen("Mode -> Installer")*8)/2), 380, BLACK, "Mode -> Installer");
	hotSpot = hotSpotPrev = -1;
	MRC_Draw_String(25, 430, WHITE, "[B] - Return");
	
	ret = MENU_SELECT_THEME;
	for(;;) {
		hotSpot = Wpad_Scan();

		// If hot spot changed
		if((((hotSpot != hotSpotPrev) && (hotSpot < 5)) || repaint)) {
			hotSpotPrev = hotSpot;
			__Draw_Button(0, "Device Menu", hotSpot == 0);
			__Draw_Button(1, "Swap Mode", hotSpot == 1);
			if(thememode < 0) {
				__Draw_Button(2, "", hotSpot == 2);
				__Draw_Button(3, "", hotSpot == 3);
			}
			else {
				if(!downloadable_theme_List) __Draw_Button(2, "", hotSpot == 2);
				else __Draw_Button(2, "Download Theme Images", hotSpot == 2);
				if(thememode < 0) __Draw_Button(3, "", hotSpot == 3);
				else __Draw_Button(3, "Download Original Theme", hotSpot == 3);
			}
			__Draw_Button(4, "Exit Menu", hotSpot == 4);
			repaint = false;
		}
		MRC_Draw_Cursor(Wpad_GetWiimoteX(), Wpad_GetWiimoteY(), 0);
		if(((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_A) || (PAD_ButtonsDown(0) & PAD_BUTTON_A)) && hotSpot > -1 && hotSpot < 5){
			if(hotSpot == 0) { ret = MENU_MANAGE_DEVICE; break; }
			else if(hotSpot == 1) {
				if(downloadable_theme_List)
					if(saveconfig) 
						__save_title_list();
                
                __Swap_Menu();
				//__Question_Window("Swap Modes", "Choose Installer Mode or Downloader Mode", "Installer", "Downloader");
                logfile("downloadable_theme_List[%d]\n", downloadable_theme_List);
                MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
				__Draw_Message("Loading Page .     Please Wait ...", 240, BLACK);
				
				
				if(thememode == 0 || thememode == 1) {
					ret = MENU_SELECT_THEME;
					if(themecnt > 0) __Free_Channel_Images();
					themecnt = filelist_retrieve(downloadable_theme_List, 0);
					__Set_Theme_Order();
					break;
				}else {
					ret = MENU_HOME;
					break;
				}
			}
			else if(hotSpot == 2) { ret = MENU_DOWNLOAD_IMAGE; break; }
			else if(hotSpot == 3) { ret = MENU_ORIG_THEME; break; }
			else if(hotSpot == 4) { ret = MENU_EXIT; break; }
		}
		else if(((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_HOME) || (PAD_ButtonsDown(0) & PAD_BUTTON_START))) {
			ret = MENU_EXIT;
			break;
		}
		else if(((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_B) || (PAD_ButtonsDown(0) & PAD_BUTTON_B))) {
			if(thememode < 0 || thememode == 2) { ret = MENU_HOME; break; }
			else {
				ret = MENU_SELECT_THEME;
				MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
				__Draw_Message("Loading Page .     Please Wait ...", 240, BLACK);
				if(themecnt > 0) __Free_Channel_Images();
				themecnt = filelist_retrieve(downloadable_theme_List, 0);
				__Set_Theme_Order();
				page = (selectedtheme+1)/12;
				break;
			}
		}
		else if(((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_2) || (PAD_ButtonsDown(0) & PAD_BUTTON_X))) {
			net_deinit();
			netconnection = checkNinit_netconnection();
			ret = MENU_HOME;
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
	__Draw_Loading(440, 440);
	if(thememode < 0) return MENU_HOME;
	if(!downloadable_theme_List) return MENU_HOME;
	if(!netconnection) {
		MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
		__Draw_Message("Internet Connection not detected .", 240, BLACK);
		sleep(2);
		return MENU_HOME;
	}
	char fatpath[128];
	char tmpstr[128];
	u32 outlen=0;
	u32 http_status=0;
	u32 Maxsize = 0xFFFFFFFF;
	u8* outbuf = NULL;
	int img_button_x = 270, img_button_y = 135, img_button_width = 150, img_button_height = 20, img_button_seperation = 5;
	int i, hotSpot, hotSpotPrev, ret, group_num = 0;
	bool repaint = true;
	char filename[64];
	char filepath[128];
	
	// Create/restore hotspots
	Wpad_CleanHotSpots();
	for(i = 0; i < 9; i++){
		Wpad_AddHotSpot(i,
			img_button_x,
			img_button_y+i*(img_button_height+img_button_seperation),
			img_button_width,
			img_button_height,
			(i == 0 ? 8 : i - 1),
			(i == 8 ? 0 : i + 1),
			i, i
		);
	}
	__Draw_Window(240, 260, "Download Theme Images :");
	MRC_Draw_String(270, 365, BLACK, "[B] - Cancel");
	// Loop
	hotSpot = hotSpotPrev = -1;
	
	for(;;){
		hotSpot = Wpad_Scan();

		// If hot spot changed
		if((hotSpot != hotSpotPrev && hotSpot < 9) || repaint){
			hotSpotPrev = hotSpot;
			
			for(i = 0; i < 9; i++) {
				sprintf(filename, "%s %d", "Image Zip File", i + 1); 
				__Draw_Button(i, filename, hotSpot == i);
				sprintf(filepath, "%s:/apps/thememanager/thememanagerimgs%d.zip",get_storage_name(thememode) , i + 1);
				if(Fat_CheckFile(filepath)) MRC_Draw_Box(img_button_x-50, img_button_y+i*(img_button_height+img_button_seperation), 20, 20, GREEN);
				else MRC_Draw_Box(img_button_x-50, img_button_y+i*(img_button_height+img_button_seperation), 20, 20, RED);
			}
			//__Draw_Button(1, "Spin", hotSpot == 1);
			//__Draw_Button(2, "Fast Spin", hotSpot == 2);
			repaint = false;
		}
		MRC_Draw_Cursor(Wpad_GetWiimoteX(), Wpad_GetWiimoteY(), 0);
		if(((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_A) || (PAD_ButtonsDown(0) & PAD_BUTTON_A)) && hotSpot > -1 && hotSpot < 9) {
			
			group_num = hotSpot + 1;
			break;
		}
		else if(WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_B || (PAD_ButtonsDown(0) & PAD_BUTTON_B)) {
			group_num = -1;
			break;
		}
		else repaint = true;
	}
	if(group_num == -1) return MENU_HOME;
	
	__Draw_Loading(440, 440);
	//if(debugcard) logfile("thememode = %d \n",thememode);
	sprintf(fatpath,"%s:/apps/thememanager/thememanagerimgs%d.zip", get_storage_name(thememode), group_num);
	//if(debugcard) logfile("fatpath(%s) \n",fatpath);
	if(!Fat_CheckFile(fatpath)) {
		if(!netconnection) { 
			sprintf(tmpstr,"Unable to connect to the internet . Exiting .");
			MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
			__Draw_Message(tmpstr,240, BLACK);
			sleep(3);
			return MENU_HOME;
		}
		sprintf(tmpstr,"Downloading Image Zip File %d .", group_num);
		MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
		__Draw_Message(tmpstr,240, BLACK);
		sleep(2);
		//__Draw_Loading(420, 250);
		sprintf(tmpstr, "http://www.wiithemer.org/resources/wii/thememanagerimgs%d.zip", group_num);
		//if(debugcard) logfile("tmpstr(%s) \n",tmpstr);
		//sprintf(fatpath,"%s:/apps/thememanager/thememanager_images.zip", get_storage_name(thememode));
		//if(debugcard) logfile("fatpath(%s) \n",fatpath);
		display_progress = true;
		ret = http_request(tmpstr, Maxsize, display_progress);//Maxsize);
		//if(debugcard) logfile("ret(%i) request \n",ret);
		if(ret) {
			ret = http_get_result(&http_status, &outbuf, &outlen);
			//if(debugcard) logfile("ret(%i) result \n",ret);
			if(ret) {
				//if(debugcard) logfile("outlen(%u) \n",outlen);
				if(outlen > 0) {
					sprintf(tmpstr,"Downloading Image Zip File %d Complete .", group_num);
					MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
					__Draw_Message(tmpstr,240, BLACK);
					sleep(2);
					sprintf(tmpstr,"Saving Image Zip File %d to %s .", group_num, get_storage_name(thememode));
					MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
					__Draw_Message(tmpstr,240, BLACK);
					
					ret = Fat_SaveFile(fatpath, (void *)&outbuf,outlen);
					if(ret < 0) {
						sprintf(tmpstr,"Saving Image Zip File %d to %s Failed .", group_num, get_storage_name(thememode));
						MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
						__Draw_Message(tmpstr,240, BLACK);
						sleep(2);
					}
					sprintf(tmpstr,"Saving Image Zip File %d to %s Complete .", group_num, get_storage_name(thememode));
					MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
					__Draw_Message(tmpstr,240, BLACK);
					sleep(2);
				}
			}
		}
		else return ret;
	}
	
	free(outbuf);
	unzFile *fptr = NULL;
	char outpath[1024];
	if(Fat_CheckFile(fatpath)) {
		//if(debugcard) logfile("file exists\n");
		sprintf(tmpstr,"%s:/apps/thememanager/imgs",get_storage_name(thememode));
		if(!Fat_CheckFile(tmpstr)) Fat_CreateSubfolder(tmpstr);
		fptr = unzOpen(fatpath);
		if(!fptr) { if(debugcard) logfile("unable to open %s", fatpath); }
		unz_global_info global_info;
		if(unzGetGlobalInfo( fptr, &global_info ) != UNZ_OK ) {
			if(debugcard) logfile( "could not read file global info\n" );
			unzClose( fptr );
			return MENU_HOME;
		}
		//if(debugcard) logfile("global_info.number_entry(%d)\n", global_info.number_entry);
		char read_buffer[ READ_SIZE ];
		uLong i;
		for ( i = 0; i < global_info.number_entry; ++i)
		{
			__Draw_Loading(440, 440);
			unz_file_info file_info;
			char filename[ MAX_FILENAME ];
			if ( unzGetCurrentFileInfo(fptr, &file_info, filename, MAX_FILENAME, NULL, 0, NULL, 0 ) != UNZ_OK ) {
				if(debugcard) logfile( "could not read file info\n" );
				unzClose( fptr );
				return MENU_HOME;
			}
			//if(debugcard) logfile("filename[%s]\n", filename);
			// Check if this entry is a directory or file.
			uLong filename_length = strlen(filename);
			if(filename[filename_length - 1] == dir_delimter ) {
				// Entry is a directory, so create it.
				if(debugcard) logfile( "dir:%s\n", filename );
				mkdir(filename, S_IREAD | S_IWRITE);
			}
			else {
					// Entry is a file, so extract it.
					//if(debugcard) logfile( "file:%s\n", filename );
					sprintf(outpath, "%s:/apps/thememanager/imgs/%s", get_storage_name(thememode), filename);
					if(!Fat_CheckFile(outpath)) {
					//if(debugcard) logfile("outpath(%s)\n", outpath);
					if ( unzOpenCurrentFile( fptr ) != UNZ_OK )
					{
						if(debugcard) logfile( "could not open file\n" );
						unzClose( fptr );
						return MENU_HOME;
					}

					// Open a file to write out the data.
					FILE *out = fopen( outpath, "wb" );
					if ( out == NULL )
					{
						if(debugcard) logfile( "could not open destination file\n" );
						unzCloseCurrentFile( fptr );
						unzClose( fptr );
						return MENU_HOME;
					}
					wiilight(1);
					sprintf(tmpstr,"Extracting image %li of %li .", i + 1, global_info.number_entry);
					MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
					__Draw_Message(tmpstr,240, BLACK);
					int error = UNZ_OK;
					do    
					{
						__Draw_Loading(440, 440);
						error = unzReadCurrentFile( fptr, read_buffer, READ_SIZE );
						if ( error < 0 )
						{
							if(debugcard) logfile( "error %d\n", error );
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
					//sprintf(tmpstr,"Saving %s Complete .", filename);
					//MRC_Draw_Texture(20, 300, textures[TEX_MESSAGE_BUBBLE]);
					//__Draw_Message(tmpstr,320, BLACK);
					wiilight(0);
				}
			}
			unzCloseCurrentFile( fptr );
			//if(debugcard) logfile("going to the next file .\n");
			 // Go the the next entry listed in the zip file.
			if((i + 1) < global_info.number_entry ) {
				if ( unzGoToNextFile( fptr ) != UNZ_OK )
				{
					if(debugcard) logfile( "cound not read next file\n" );
					unzClose( fptr );
					return MENU_HOME;
				}
				//if(debugcard) logfile("going to the next file Complete .\n");
			}
		}
		unzClose( fptr );
		*read_buffer = 0;
	}
	
	MRC_Draw_Box(img_button_x-50, img_button_y+group_num*(img_button_height+img_button_seperation), 20, 20, GREEN);
	*fatpath = 0;
	*filepath = 0;
	*tmpstr = 0;
	*filename = 0;
	
	return MENU_DOWNLOAD_IMAGE;
}

int __exit_Menu() {
	int i, hotSpot, hotSpotPrev, ret = -1, EXIT_MENU_X = 50, EXIT_MENU_Y = 250, EXIT_MENU_BUTTON_WIDTH = 150, EXIT_MENU_BUTTON_HEIGHT = 20, EXIT_MENU_BUTTON_SEPARATION = 50;
	bool repaint = true;
	//bool showinstructions = false;
	const char *exits[3] = { "HomeBrew Channel", "Priiloader", "System Menu" };
	
	// Create/restore hotspots
	Wpad_CleanHotSpots();
	
	Wpad_AddHotSpot(0, EXIT_MENU_X + 0*(EXIT_MENU_BUTTON_WIDTH+EXIT_MENU_BUTTON_SEPARATION), EXIT_MENU_Y, EXIT_MENU_BUTTON_WIDTH, EXIT_MENU_BUTTON_HEIGHT, 2, 1, 2, 1);
	Wpad_AddHotSpot(1, EXIT_MENU_X + 1*(EXIT_MENU_BUTTON_WIDTH+EXIT_MENU_BUTTON_SEPARATION), EXIT_MENU_Y, EXIT_MENU_BUTTON_WIDTH, EXIT_MENU_BUTTON_HEIGHT, 0, 2, 0, 2);
	Wpad_AddHotSpot(2, EXIT_MENU_X + 2*(EXIT_MENU_BUTTON_WIDTH+EXIT_MENU_BUTTON_SEPARATION), EXIT_MENU_Y, EXIT_MENU_BUTTON_WIDTH, EXIT_MENU_BUTTON_HEIGHT, 1, 0, 1, 0);
	// Loop
	hotSpot = hotSpotPrev = -1;

	
	for(;;) {
		hotSpot = Wpad_Scan();
		
		MRC_Draw_Texture(0, 0, textures[TEX_BACKGROUND]);
		
		sprintf(tempString, "System Menu v%s_%s %u", get_system_version_Display(system_Version), get_display_region(system_Version), system_Version);
		MRC_Draw_String(((640-strlen(tempString)*8)/2), 20, WHITE, tempString);
		sprintf(tempString, "IOS %i", IOS_GetVersion());
		MRC_Draw_String(20, 20, WHITE, tempString);
		MRC_Draw_String(20, 450, WHITE, "[A] - Select Exit To");
		MRC_Draw_String((640-strlen("[B] - Select Device Menu")*8)-5, 420, WHITE, "[B] - Select Device Menu");
		MRC_Draw_String((640-strlen("[HOME/Start] - Exit To HBC")*8)-5, 450, WHITE, "[HOME/Start] - Exit To HBC");
		MRC_Draw_String2(150,160, WHITE, "Leaving Already ? Where You Going :");
		
		// If hot spot changed
		if(((hotSpot != hotSpotPrev) && (hotSpot < 3)) || repaint){
			hotSpotPrev = hotSpot;

			for(i = 0; i < 3; i++){
				__Draw_Button(i, exits[i], hotSpot == i);
			}
			repaint = false;
		}
		MRC_Draw_Cursor(Wpad_GetWiimoteX(), Wpad_GetWiimoteY(), 0);
		

		if(((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_A) || (PAD_ButtonsDown(0) & PAD_BUTTON_A)) && hotSpot != -1) {
			
			//if(debugcard) logfile("hotspot[%i]\n", hotSpot);
			if(hotSpot == 2) {
				__Draw_Message("Exiting to the System Menu ....", 350, WHITE);
				sleep(1);
				sys_loadmenu();
			}
			else if(hotSpot == 0) {
				__Draw_Message("Exiting to HBC ....", 350, WHITE);
				sleep(1);
				sysHBC();
			}
			else if(hotSpot == 1) {
				__Draw_Message("Exiting to PriiLoader ....", 350, WHITE);
				sleep(1);
				system_Exit_Priiloader();
			}
			break;
		}
		else if(((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_B) || (PAD_ButtonsDown(0) & PAD_BUTTON_B))) {
			ret = MENU_MANAGE_DEVICE;
			
			break;
		}
		else if(((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_HOME) || (PAD_ButtonsDown(0) & PAD_BUTTON_START))) {
			__Draw_Message("Exiting to HBC ....", 350, WHITE);
			sleep(1);
			sysHBC();
			
			break;
		}
		else repaint = true;
	}
	return ret;
}
int __Select_Device() {
	int i, hotSpot, hotSpotPrev, selected_device, mode = 3, DEVICE_X = 260, DEVICE_Y = 230, DEVICE_BUTTON_WIDTH = 120, DEVICE_BUTTON_HEIGHT = 20, DEVICE_BUTTON_SEPARATION = 30;
	bool repaint = true;
	//bool showinstructions = false;
	const char *dev[3] = { "SD", "USB", "Choose Later" };
	
	// Create/restore hotspots
	Wpad_CleanHotSpots();
	for(i = 0; i < 3; i++){
		
		Wpad_AddHotSpot(i,
			DEVICE_X,
			DEVICE_Y + i*(DEVICE_BUTTON_HEIGHT+DEVICE_BUTTON_SEPARATION),
			DEVICE_BUTTON_WIDTH,
			DEVICE_BUTTON_HEIGHT,
			(i == 0 ? 2 : i - 1),
			(i == 2 ? 0 : i + 1),
			(i == 0 ? 2 : i - 1), 
			(i == 2 ? 0 : i + 1)
		);
	}
	hotSpot = hotSpotPrev = -1;
	
	selected_device = 0;
	for(;;) {
		hotSpot = Wpad_Scan();
		MRC_Draw_Texture(0, 0, textures[TEX_BACKGROUND]);
		sprintf(tempString, "System Menu v%s_%s %u", get_system_version_Display(system_Version), get_display_region(system_Version), system_Version);
		MRC_Draw_String(((640-strlen(tempString)*8)/2), 20, WHITE, tempString);
		sprintf(tempString, "IOS %i", IOS_GetVersion());
		MRC_Draw_String(20, 20, WHITE, tempString);
		
		sprintf(tempString, "Select %s Device :", (downloadable_theme_List == 1 ? "Save" : "Theme"));
		MRC_Draw_String2(((640-strlen(tempString)*8)/2), 150, WHITE, tempString);
			
		sprintf(tempString, "Device : %s", get_storage_name(thememode));
		MRC_Draw_String(((640-strlen(tempString)*8)/2), 400, WHITE, tempString);
			
		MRC_Draw_String(25, 450, WHITE, "[A] - Select Device");
		MRC_Draw_String((640-strlen("[HOME] - Exit Menu")*8)-15, 450, WHITE, "[HOME] - Exit Menu");
		
		// If hot spot changed
		if(((hotSpot != hotSpotPrev) && (hotSpot < 3)) || repaint){
			hotSpotPrev = hotSpot;

			for(i = 0; i < 3; i++){
				__Draw_Button(i, dev[i], hotSpot == i);
			}
			repaint = false;
		}
		MRC_Draw_Cursor(Wpad_GetWiimoteX(), Wpad_GetWiimoteY(), 0);
		
		if(((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_A) || (PAD_ButtonsDown(0) & PAD_BUTTON_A)) && hotSpot != -1) {
			
			selected_device = hotSpot;
			//if(debugcard) logfile("selected_device[%i]\n", selected_device);
			if(selected_device == 2) {
				mode = 2;
				break;
			}
			__Draw_Loading(440, 440);
			mode = Fat_Mount(selected_device);
			//if(debugcard) logfile("mode[%i] \n", mode);
			__Draw_Loading(440, 440);
			if(mode < 0) { 
				sprintf(tempString, "Unable to mount %s .", dev[selected_device]);
				MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
				__MaskBanner(textures[TEX_MESSAGE_BUBBLE]);
				__Draw_Message(tempString, 240, BLACK);
				sleep(2);
			}
			else {
				device_Chose = true;
				thememode = mode;
				break;
			}
			
		}
		else if(((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_HOME) || (PAD_ButtonsDown(0) & PAD_BUTTON_START))) {
			mode = MENU_EXIT;
			break;
		}
		else if(((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_B) || (PAD_ButtonsDown(0) & PAD_BUTTON_B))) {
			//if(!device_Chose) continue;
			mode = MENU_HOME;
			break;
		}
		else repaint = true;
	}
	__Draw_Loading(440, 440);
	logfile("leaving select device mode ->> %d\n", mode);
	return mode;
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
int __Spin_Question() {
	int i, hotSpot, hotSpotPrev, ret, SPIN_BUTTON_X = 270, SPIN_BUTTON_Y = 205, SPIN_BUTTON_WIDTH = 100, SPIN_BUTTON_HEIGHT = 20, SPIN_BUTTON_SEPARATION = 10;
	bool repaint = true;
	char * spin_type[3] = {"No Spin", "Spin", "Fast Spin"};
	
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
			(i == 0 ? 2 : i - 1),
			(i == 2 ? 0 : i + 1)
		);
	}

	__Draw_Window(240, 120, "Channel Spin Option :");
	MRC_Draw_String(270, 295, BLACK, "[B] - Return");
	hotSpot = hotSpotPrev = -1;

	ret = MENU_SELECT_THEME;
	for(;;){
		hotSpot = Wpad_Scan();

		// If hot spot changed
		if((hotSpot != hotSpotPrev && hotSpot < 3) || repaint){
			hotSpotPrev = hotSpot;
			
			for(i = 0; i < 3; i++) {
				__Draw_Button(i, spin_type[i], hotSpot == i);
			}
			
			//__Draw_Button(1, "Spin", hotSpot == 1);
			//__Draw_Button(2, "Fast Spin", hotSpot == 2);
			repaint = false;
		}
		MRC_Draw_Cursor(Wpad_GetWiimoteX(), Wpad_GetWiimoteY(), 0);
		if(((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_A) || (PAD_ButtonsDown(0) & PAD_BUTTON_A)) && hotSpot > -1 && hotSpot < 3) {
			
			ret = hotSpot;
			break;
		}
		else if(WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_B || (PAD_ButtonsDown(0) & PAD_BUTTON_B)) {
			ret = -100;
			break;
		}
		else repaint = true;
	}

	return ret;
}
int __Spin_Color_Question() {
	int i, hotSpot, hotSpotPrev, ret, COLOR_BUTTON_X = 270, COLOR_BUTTON_Y = 135, COLOR_BUTTON_WIDTH = 100, COLOR_BUTTON_HEIGHT = 20, COLOR_BUTTON_SEPARATION = 5;
	bool repaint = true;
	char * spin_color[10] ={"Black", "Blue", "Green", "Orange", "Pink", "Purple", "Red", "White", "Yellow", "Original"};
	// Create/restore hotspots
	Wpad_CleanHotSpots();
	for(i = 0; i < 10; i++){
		Wpad_AddHotSpot(i,
			COLOR_BUTTON_X,
			COLOR_BUTTON_Y+i*(COLOR_BUTTON_HEIGHT+COLOR_BUTTON_SEPARATION),
			COLOR_BUTTON_WIDTH,
			COLOR_BUTTON_HEIGHT,
			(i == 0 ? 9 : i - 1),
			(i == 9 ? 0 : i + 1),
			(i == 0 ? 9 : i - 1),
			(i == 9 ? 0 : i + 1)
		);
	}

	__Draw_Window(240, 300, "Channel Spin Color Option :");
	MRC_Draw_String(270, 390, BLACK, "[B] - Return");
	// Loop
	hotSpot = hotSpotPrev = -1;

	ret = MENU_SELECT_THEME;
	for(;;){
		hotSpot = Wpad_Scan();

		// If hot spot changed
		if((hotSpot != hotSpotPrev && hotSpot < 10) || repaint){
			hotSpotPrev = hotSpot;
			
			for(i = 0; i < 10; i++) {
				__Draw_Button(i, spin_color[i], hotSpot == i);
			}
			repaint = false;
		}
		MRC_Draw_Cursor(Wpad_GetWiimoteX(), Wpad_GetWiimoteY(), 0);
		if(((WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_A) || (PAD_ButtonsDown(0) & PAD_BUTTON_A)) && hotSpot > -1 && hotSpot < 10) {
			ret = hotSpot;
			break;
		}
		else if(WPAD_ButtonsDown(WPAD_CHAN_0) & WPAD_BUTTON_B || (PAD_ButtonsDown(0) & PAD_BUTTON_B)) {
			ret = -100;
			break;
		}
		else repaint = true;
	}

	return ret;
}
int __download_Theme() {
	__Draw_Loading(440, 440);
	if(!netconnection) {
		if(debugcard) logfile("Internet connection not detected .\n");
		MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
		__Draw_Message("Internet connection not detected .", 240, BLACK);
		sleep(3);
		return MENU_SELECT_THEME;
	}
	spinselected = __Spin_Question();
	if(spinselected == -100) {
		page = (selectedtheme+1)/12;
		return MENU_SHOW_THEME;
	}
	spincolorselected = __Spin_Color_Question();
	if(spincolorselected == -100) {
		page = (selectedtheme+1)/12;
		return MENU_SHOW_THEME;
	}
	
	u32 buttons;
	char sitepath[256];
	char tmpstr[128];
	char sessionId[32];
	char themedownloadlink[128];
	char *actions[4] = { "prep_Dir", "copy_mym_files", "download_content", "build_theme" };
	char *spin_type[3] = {"nospin.mym", "spin.mym", "fastspin.mym"};
	char *spin_color[10] = {"outline_Black.mym", "outline_Blue.mym", "outline_Green.mym", "outline_Orange.mym", "outline_Pink.mym", "outline_Purple.mym", "outline_Red.mym", "outline_White.mym", "outline_Yellow.mym", "None"};
	int ret = -2, i = 0, mymtype = -1, retries =0;
	//int theme_selected = selectedtheme;
	//logfile("spinselected[%i] selected = %d \n selectedtheme = %d %s\n", spinselected, theme_selected, selectedtheme, ThemeList[selectedtheme].title);
	u32 uret;
	mymtype = is_theme_region_specific(selectedtheme);
	u32 outlen = 0;
	u32 http_status = 0;
	u32 Maxsize = 0xFFFFFFFF;
	u8* outbuf = NULL;
	char *savename = NULL;
	//extern u32 received;
	
	for(i = 0; i < 4; i++) {
		__Draw_Loading(440, 440);
		//if(debugcard) logfile("i(%d)\n", i);
		switch(i) {
			case 0:
				sprintf(sitepath, "http://www.wiithemer.org/resources/wii/index.php?action=%s", actions[i]);
				MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
				__Draw_Message("Requesting Server to Start Build Process .", 240, BLACK);
				sleep(1);
			break;
			case 1:
				if(mymtype == 1) sprintf(sitepath, "http://www.wiithemer.org/resources/wii/index.php?action=%s&mymfile=%s%s.mym&spin=%s&spincolor=%s&sessionId=%s", actions[i], ThemeList[selectedtheme].mym, get_display_region(system_Version), spinoptions(spinselected), spin_color[spincolorselected], sessionId);
				else sprintf(sitepath, "http://www.wiithemer.org/resources/wii/index.php?action=%s&mymfile=%s&spin=%s&spincolor=%s&sessionId=%s", actions[i], ThemeList[selectedtheme].mym, spin_type[spinselected], spin_color[spincolorselected], sessionId);
				MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
				__Draw_Message("Copying needed Files and Directory .", 240, BLACK);
				sleep(1);
			break;
			case 2:
				if(mymtype == 1)
					sprintf(sitepath, "http://www.wiithemer.org/resources/wii/index.php?action=%s&mymfile=%s%s.mym&spin=%s&version=%i&sessionId=%s", actions[i], ThemeList[selectedtheme].mym, get_display_region(system_Version), spinoptions(spinselected), system_Version, sessionId);
				else
					sprintf(sitepath, "http://www.wiithemer.org/resources/wii/index.php?action=%s&mymfile=%s&spin=%s&version=%i&sessionId=%s", actions[i], ThemeList[selectedtheme].mym, spin_type[spinselected], system_Version, sessionId);
				
				MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
				sprintf(tmpstr, "Downloading Content %s.app from System Menu %s_%s .", get_content_name_noextension(system_Version), get_system_version_Display(system_Version), get_display_region(system_Version));
				__Draw_Message(tmpstr, 240, BLACK);
				sleep(1);
			break;
			case 3:
				if(mymtype == 1)
				sprintf(sitepath, "http://www.wiithemer.org/resources/wii/index.php?action=%s&mymfile=%s%s.mym&spin=%s&spincolor=%s&version=%i&sessionId=%s&selected=%i", actions[i], ThemeList[selectedtheme].mym, get_display_region(system_Version), spinoptions(spinselected), spin_color[spincolorselected], system_Version, sessionId, selectedtheme);
				else
					sprintf(sitepath, "http://www.wiithemer.org/resources/wii/index.php?action=%s&mymfile=%s&spin=%s&spincolor=%s&version=%i&sessionId=%s&selected=%i", actions[i], ThemeList[selectedtheme].mym, spin_type[spinselected], spin_color[spincolorselected], system_Version, sessionId, selectedtheme);
				
				MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
				sprintf(tmpstr, "Server Building %s Theme .", ThemeList[selectedtheme].title);
				__Draw_Message(tmpstr, 240, BLACK);
				sleep(1);
			break;
		}
		//if(debugcard) logfile("sitepath[%s]\n", sitepath);
		//if(i >= 2)
		//	display_progress = true;
		ret = http_request(sitepath, Maxsize, display_progress);
		int d = -2;
		time_t start = time(NULL);
		while(d != 1) {
			__Draw_Loading(440, 440);
			d = ret;
			time_t now = time(NULL);
			if (difftime(now, start) >= 45.0 || ret == 0){
				if(ret != 0) logfile("At least 60 seconds have passed. Timed out\n");
				else logfile("ret from request 0\n");
				break;
			}
		}
		//display_progress = false;
		//if(debugcard) logfile("ret request[%i]\n", ret);
		if(ret) {
			ret = http_get_result(&http_status, &outbuf, &outlen);
			//if(debugcard) logfile("ret result[%i]\n", ret);
			int e = -2;
			time_t start = time(NULL);
			while(e != 1) {
				__Draw_Loading(440, 440);
				e = ret;
				time_t now = time(NULL);
				if (difftime(now, start) >= 45.0 || ret == 0){
					if(ret != 0) logfile("At least 60 seconds have passed. Timed out\n");
					else logfile("ret from request 0\n");
					break;
				}
			}
			if(ret) {
				char output[outlen];
				if(outlen > 0 && http_status == 200) {
					memcpy(output, outbuf, outlen);
					output[outlen] = 0;
					//if(debugcard) logfile("message:\n%s\n", output);
					switch(i) {
						case 0:
							strcpy(sessionId, output);
							if(strlen(sessionId) < 16) logfile("bad id build failed [%s] case 0\n", sessionId);
							MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
							__Draw_Message("Requesting Server to Start Build Process Complete.", 240, BLACK);
							sleep(2);
						break;
						case 1:
							char *check1str = "ERROROKOK";
							if(output == check1str) logfile("return here building failed case 1 theme copy\n");
							char *check2str = "OKERROROK";
							if(output == check2str) logfile("return here building failed case 1 spin copy\n");
							char *check3str = "OKOKERROR";
							if(output == check3str) logfile("return here building failed case 1 spincolor copy\n");
							char *check4str = "ERRORERROROK";
							if(output == check4str) logfile("return here building failed case 1 theme/spin copy\n");
							char *check5str = "ERROROKERROR";
							if(output == check5str) logfile("return here building failed case 1 theme/spincolor copy\n");
							char *check6str = "ERRORERRORERROR";
							if(output == check6str) logfile("return here building failed case 1 theme/spin/color copy\n");
							MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
							__Draw_Message("Copying needed Files and Directory Complete .", 240, BLACK);
							sleep(2);
						break;
						case 2:
							char *checkstr2 = "Error";
							if(output == checkstr2) logfile("return here case 2 app download failed\n");
							MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
							sprintf(tmpstr, "Downloading Content %s.app from System Menu %s_%s Complete .", get_content_name_noextension(system_Version), get_system_version_Display(system_Version), get_display_region(system_Version));
							__Draw_Message(tmpstr, 240, BLACK);
							sleep(2);
						break;
						case 3:
							strcpy(themedownloadlink, output);
							if(strlen(themedownloadlink) < 48) logfile("bad link to short [%s] case 3\n", themedownloadlink);
							MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
							sprintf(tmpstr, "Server Building %s Theme Complete .", ThemeList[selectedtheme].title);
							__Draw_Message(tmpstr, 240, BLACK);
							sleep(2);
						break;
					}
				}
				*output = 0;
			}
		}
		else {
			MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
			sprintf(tmpstr, "ERROR:[%u request failed] Downloading theme failed . Press A to Continue .", ret);
			__Draw_Message(tmpstr, 240, BLACK);
			for(;;) {
				buttons = Wpad_WaitButtons(); 
				if(buttons == WPAD_BUTTON_A || buttons == PAD_BUTTON_A) break;
			}
			return MENU_HOME;
		}	
		//free(sitepath);
	}
	
	//if(debugcard) logfile("link[%s]\n", themedownloadlink);
	//sprintf(sitepath, "%s", themedownloadlink);
	
	
	//if(debugcard) logfile("sitepath[%s]\n", sitepath);
	char savepath[128];
	uret = http_request_content_length(themedownloadlink, Maxsize);
	//logfile("ret from request length = %u\n", uret);
	if(uret < 1000) {
		MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
		sprintf(tmpstr, "ERROR:[%u]	Downloading theme failed . Press A to Continue .", uret);
		__Draw_Message(tmpstr, 240, BLACK);
		for(;;) {
			buttons = Wpad_WaitButtons(); 
			if(buttons == WPAD_BUTTON_A || buttons == PAD_BUTTON_A) break;
		}
		return MENU_HOME;
	}
retry:	
	MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
	sprintf(tmpstr, "Downloading %s Theme . size: %.02f MB", ThemeList[selectedtheme].title, uret/MB_SIZE);
	__Draw_Message(tmpstr, 235, BLACK);
	
	display_progress = true;
	ret = http_request(themedownloadlink, uret, display_progress);
	display_progress = false;
	int f = -2;
	time_t start = time(NULL);
	while(f != 1) {
		__Draw_Loading(440, 440);
		f = ret;
		time_t now = time(NULL);
		if (difftime(now, start) >= 45.0 || ret == 0){
			if(ret != 0) logfile("At least 60 seconds have passed.\n");
			else logfile("ret from request 0\n");
			break;
		}
	}
	if(ret) {
		ret = http_get_result(&http_status, &outbuf, &outlen);
		//if(outlen != uret) return -1000;
		int g = -2;
		time_t start = time(NULL);
		while(g != 1) {
			__Draw_Loading(440, 440);
			g = ret;
			time_t now = time(NULL);
			if (difftime(now, start) >= 45.0 || ret == 0){
				if(ret != 0) logfile("At least 60 seconds have passed.\n");
				else logfile("ret from request 0\n");
				break;
			}
		}
		if(outlen == uret && http_status == 200) {
			MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
			sprintf(tmpstr, "Downloading %s Theme Complete . size: %.02f MB", ThemeList[selectedtheme].title, uret/MB_SIZE);
			__Draw_Message(tmpstr, 240, BLACK);
			sleep(2);
			MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
			sprintf(tmpstr, "Saving %s .", ThemeList[selectedtheme].title);
			__Draw_Message(tmpstr, 240, BLACK);
			char * token = strtok(themedownloadlink, "/");
			while(token != NULL) {
				token = strtok(NULL, "/");
				if(token != NULL)
				savename = token;
			}
			
			//if(debugcard) logfile("savename = %s\n", savename);
			sprintf(savepath,"%s:/%s", get_storage_name(thememode), themedir);
			
			if(!Fat_CheckDir(savepath))
				Fat_CreateSubfolder(savepath);
			//__Draw_Loading(440, 440);
			sprintf(savepath,"%s:/%s/%s", get_storage_name(thememode), themedir, savename);
			ret = Fat_SaveFile(savepath, (void *)&outbuf, outlen);
			MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
			sprintf(tmpstr, "Saving %s Complete .", ThemeList[selectedtheme].title);
			__Draw_Message(tmpstr, 240, BLACK);
			sleep(3);
			token = NULL;
		}
		else {
			logfile("ret from get result = %d\noutlen = %u\nhttp status = %u\n", ret, outlen, http_status);
			MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
			sprintf(tmpstr, "ERROR:[%d]", ret);
			__Draw_Message(tmpstr, 240, BLACK);
			sleep(1);
			sprintf(tmpstr, "ERROR:[%d]  Downloading %s . size: %.02f MB Press A to Continue .", ret, ThemeList[selectedtheme].title, uret/MB_SIZE);
			__Draw_Message(tmpstr, 240, BLACK);
			for(;;) {
				buttons = Wpad_WaitButtons(); 
				if(buttons == WPAD_BUTTON_A || buttons == PAD_BUTTON_A) break;
			}
			if(retries >= 1) return MENU_HOME;
			retries++;
			goto retry;
		}
	}
	else {
		logfile("ret from request = %d\noutlen = %u\nhttp status = %u\n", ret, outlen, http_status);
		MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
		sprintf(tmpstr, "ERROR:[%d]", ret);
		__Draw_Message(tmpstr, 240, BLACK);
		sleep(1);
		sprintf(tmpstr, "ERROR:[%d]  Downloading %s . size: %.02f MB Press A to Continue .", ret, ThemeList[selectedtheme].title, uret/MB_SIZE);
		__Draw_Message(tmpstr, 240, BLACK);
		for(;;) {
			buttons = Wpad_WaitButtons(); 
			if(buttons == WPAD_BUTTON_A || buttons == PAD_BUTTON_A) break;
		}
		if(retries >= 1) return MENU_HOME;
		retries++;
		goto retry;
	}
	//if(debugcard) logfile("\ndelete server session dir here .\n");
	sprintf(sitepath, "http://www.wiithemer.org/resources/wii/index.php?action=%s&sessionId=%s", "remove_session_Dir", sessionId);
	if(debugcard) logfile("sitepath[%s]\n", sitepath);
	ret = http_request(sitepath, Maxsize, display_progress);
	if(debugcard) logfile(" ret delete session folder ->> %d\n", ret);
	if(!Fat_CheckFile(savepath)) {
		
		//sprintf(sitepath, "http://wiithemer.org/resources/wii/index.php?action=%s&downloadcount=%d&themetoupdate=%s", "updatedownloadcount", 1, Theme_Download_Txt[orden[selectedtheme]]);
		//ret = http_request(sitepath, Maxsize);
		//if(debugcard) logfile("sitepath[%s] ret[%d]\n", sitepath, ret);
	}
	//net_deinit();
	savename = 0;
	*sitepath = 0;
	free(outbuf);
	outbuf = NULL;
	page = (selectedtheme+1)/12;
	return MENU_SELECT_THEME;
}
bool read_Settings() {
	__Draw_Loading(440, 440);
	char filepath[64];
	FILE *settings_File;
	
	sprintf(filepath, "%s:/apps/thememanager/settings.txt", get_storage_name(thememode));
	
	settings_File = fopen(filepath, "rb");
	
	if(!settings_File) return false;
	fclose(settings_File);
	//if(debugcard) logfile("settings file found\nfilepath[%s]\n", filepath);
	__Draw_Loading(440, 440);
	return true;
}
bool write_Settings() {
	
	__Draw_Loading(440, 440);
	char filepath[64];
	FILE *settings_File;
	char *file_contents = NULL;
	
	sprintf(filepath, "%s:/apps/thememanager/settings.txt", get_storage_name(thememode));
	
	settings_File = fopen(filepath, "wb");
	if(!settings_File) {
		//if(debugcard) logfile("Unable to open file [%s:/apps/theme_manager/settings.txt]\n", get_storage_name(thememode));
		return false;
	}
	
	file_contents = "Disclaimer disabled";
	fprintf(settings_File, file_contents);
	fclose(settings_File);
	//if(debugcard) logfile("filepath[%s]\n", filepath);
	__Draw_Loading(440, 440);
	return true;
}
bool disclaimer() {
	MRC_Draw_Texture(0, 0, textures[TEX_DISCLAIMER_BACKGROUND]);
	MRC_Draw_String2(275, 40, WHITE, "[DISCLAIMER] :");
	MRC_Draw_String2(60, 140, WHITE, "THIS APPLICATION COMES WITH NO WARRANTY AT ALL, NEITHER EXPRESSED ");
	MRC_Draw_String2(60, 215, WHITE, "NOR IMPLIED . I DO NOT TAKE ANY RESPONSIBILITY FOR ANY DAMAGE");
	MRC_Draw_String2(60, 290, WHITE, "TO YOUR WII/WIIU CONSOLE BECAUSE OF IMPROPER USE OF THIS SOFTWARE .");
	MRC_Draw_String(40, 430, WHITE, "[A] Continue");
	MRC_Draw_String(500, 430, WHITE, "[B] Exit Menu");
	MRC_Render_Screen();
	
	u32 buttons;
	for(;;) {
		buttons = Wpad_WaitButtons();
		//if(debugcard) logfile("buttons [%u]\n", buttons);
		if(buttons == WPAD_BUTTON_B || buttons == PAD_BUTTON_B) break; 
		if(buttons == WPAD_BUTTON_A || buttons == PAD_BUTTON_A) return false;
	}
	return true;
}
int __StartUp_Page() { // Change USB to SD for release
	int ret = MENU_MANAGE_DEVICE;
	bool __exit = false, settingsfile= false;
	
	__Draw_Loading(440, 440);
	ret = Get_system_version();
	if(ret > 610) ret = system_Version = check_custom_system_version();
	else system_Version = ret;
	__Draw_Loading(440, 440);
	thememode = Fat_Mount(USB);
	__Draw_Loading(440, 440);
	settingsfile = read_Settings();
	__Draw_Loading(440, 440);
	if(!settingsfile) {
		__Draw_Loading(440, 440);
		__exit = disclaimer();
		__Draw_Loading(440, 440);
		if(__exit) {
			ret = MENU_EXIT;
			__Draw_Loading(440, 440);
			exit(0);
		}
		else {
			settingsfile = write_Settings();
		if(debugcard) logfile("wrote settings file [%i]\n", settingsfile);
		__Draw_Loading(440, 440);
		}
	}
	
	MRC_Draw_Texture(0, 0, textures[TEX_BACKGROUND]);
	MRC_Draw_Texture(20, 225, textures[TEX_MESSAGE_BUBBLE]);
	__Draw_Message("Collecting some info .", 240, BLACK);
	
	
	//if(debugcard) logfile("system_Version = %d\n", system_Version);
	__Draw_Loading(440, 440);	
	sprintf(tempString, "System Menu v%s_%s %u", get_system_version_Display(system_Version), get_display_region(system_Version), system_Version);
	MRC_Draw_String(((640-strlen(tempString)*8)/2), 20, WHITE, tempString);
	sprintf(tempString, "IOS %i", IOS_GetVersion());
	MRC_Draw_String(20, 20, WHITE, tempString);
	
	
	__Draw_Loading(440, 440);
	priiloadercheck = checkforpriiloader();
	__Draw_Loading(440, 440);
	if(!priiloadercheck) {
		priiloaderackknowledgement = __Question_Window("Priiloader not Detected", "Continue at Your Own Risk . ", "Exit", "Continue");
		__Draw_Loading(440, 440);
		if(!priiloaderackknowledgement) 
			ret = MENU_EXIT;
	}
	
	__Draw_Loading(440, 440);
	if(!netconnection) {
		__Draw_Loading(440, 440);
		netconnection = checkNinit_netconnection();
		//__Draw_Net_Connection(225, 440, netconnection);
	}
	__Draw_Loading(440, 440);
	thememode = -1;
	Fat_Unmount(USB);
	__Draw_Loading(440, 440);
	return ret;
}
void Menu_Loop(){
	int ret;
	
	// Check if widescreen
	if(CONF_GetAspectRatio() == CONF_ASPECT_16_9)
		wideScreen = true;

	// Init MRC graphics
	MRC_Init();
	// Load skin images*/
	__load_textures();
	__Draw_Loading(440, 440);
	ret = __StartUp_Page();
	__Draw_Loading(440, 440);
	if(ret >= 0)
		ret = MENU_MANAGE_DEVICE;
	__Draw_Loading(440, 440);
	for(;;) {
		
		if(ret == MENU_MANAGE_DEVICE) {
			__Draw_Loading(440, 440);
			ret = __Select_Device();
			if(ret == 0 || ret == 1) {
				if(themecnt > 0) __Free_Channel_Images();
				themecnt = 0;
				themecnt = filelist_retrieve(downloadable_theme_List, 0);
				logfile("themecnt[%d]\n", themecnt);
				if(themecnt<=0) {
					ret = MENU_HOME;
				}
				else { //if(debugcard) logfile("downloadable_theme_List [%d]\n", downloadable_theme_List);
					__Set_Theme_Order();
					ret = MENU_SELECT_THEME;
				}
			}
			else if(ret == 2) {
				themecnt = 0;
				thememode = -1;
				ret = MENU_HOME;
			}
			__Draw_Loading(440, 440);
		}
		else if(ret == MENU_HOME) { __Draw_Loading(440, 440); ret = __Home(); __Draw_Loading(440, 440);}
		else if(ret == MENU_SELECT_THEME)  {  ret = __Select_Theme(); }
		else if(ret == MENU_SHOW_THEME) {  ret=__Show_Theme(); __Draw_Loading(440, 440);}
		else if(ret == MENU_DOWNLOAD_IMAGE) { __Draw_Loading(440, 440); ret = __Downloadthemepng(); __Draw_Loading(440, 440);}
		else if(ret == MENU_ORIG_THEME) { __Draw_Loading(440, 440); ret = __downloadApp(); __Draw_Loading(440, 440);}
		else if(ret == MENU_INSTALL_THEME) {
			__Draw_Loading(440, 440);
			if(downloadable_theme_List) {
				ret = __download_Theme();
			}
			else ret = __install_Theme();
			__Draw_Loading(440, 440);
		}
		else if(ret == MENU_EXIT) {
			__Draw_Loading(440, 440);
			if(downloadable_theme_List)
				if(saveconfig) 
					__save_title_list();
			ret = __exit_Menu();
			__Draw_Loading(440, 440);
		}
	}
	return;
}

