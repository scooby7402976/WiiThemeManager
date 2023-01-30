#include <iostream>
#include <vector>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <stddef.h>

#define VectorResize(List) if(List.capacity()-List.size() == 0) List.reserve(List.size()+100)

#include "gecko.h"
#include "miniunz.h"
#include "unzip.h"
#include "menu.h"
#include "fat_mine.h"
#include "theme_make.h"
#include "libunrar/rar.hpp"
#include "ioapi.h"
#include <libunrar/rar.hpp>

using namespace std;
vector <char> MymList;
const char *Theme_Title_Name(int input) {
	switch(input) {
		case 0:
			return "DarkWii Blue";// 0
		break;
		case 1:
			return "DarkWii White";
		break;
		case 2:
			return "DarkWii Green";
		break;
		case 3:
			return "DarkWii Orange";
		break;
		case 4:
			return "DarkWii Pink";// 5
		break;
		case 5:
			return "DarkWii Purple";
		break;
		case 6:
			return "DarkWii Red";
		break;
		case 7:
			return "DarkWii Yellow";
		break;
		case 8:
			return "Fullmetal Alchemist";
		break;
		case 9:
			return "Storm";// 10
		break;
		case 10:
			return "Bleach";
		break;
		case 11:
			return "Conduit";
		break;
		case 12:
			return "Constantine";
		break;
		case 13:
			return "Dr._Who";
		break;
		case 14:
			return "Evil Dead";// 15
		break;
		case 15:
			return "Gaara of the Sands";
		break;
		case 16:
			return "Golden Sun";
		break;
		case 17:
			return "Halo";
		break;
		case 18:
			return "Hundred's";
		break;
		case 19:	
			return "Imports";// 20
		break;
		case 20:	
			return "Kingdom Hearts";
		break;
		case 21:	
			return "Luigi";
		break;
		case 22:	
			return "Martin Abel";
		break;
		case 23:	
			return "Metroid One";
		break;
		case 24:	
			return "Metroid Two";// 25
		break;
		case 25:	
			return "Mortal Kombat";
		break;
		case 26:	
			return "Okami";
		break;
		case 27:	
			return "Punch Out";
		break;
		case 28:	
			return "Saw";
		break;
		case 29:	
			return "Shadow The_HedgeHog";//30
		break;
		case 30:	
			return "Shadow The_HedgeHog2";
		break;
		case 31:	
			return"Teenage Mutant Ninja Turtles";
		break;
		case 32:	
			return "Tomb Raider";
		break;
		case 33:	
			return "Transformers";
		break;
		case 34:	
			return "Wolverine";//35
		break;
		case 35:	
			return "Zelda Twilight Princess";
		break;
		case 36:	
			return "Leopard";
		break;
		case 37:	
			return "Batman";
		break;
		case 38:	
			return "Blackmage";
		break;
		case 39:	
			return "BlackPirate";// 40
		break;
		case 40:	
			return "CarTheme"; 
		break;
		case 41:	
			return "CodeGeass";
		break;
		case 42:	
			return "Dethklok";
		break;
		case 43:	
			return "Cars";
		break;
		case 44:	
			return "Fantasy";// 45
		break;
		case 45:	
			return "Firewii"; 
		break;
		case 46:	
			return "Ghostbusters";
		break;
		case 47:	
			return "Jurassic Park";
		break;
		case 48:	
			return "Lime";
		break;
		case 49:	
			return "Madworld";// 50
		break;
		case 50:	
			return "Madworld2"; 
		break;
		case 51:	
			return "Majora's Mask";
		break;
		case 52:	
			return "Mario";
		break;
		case 53:	
			return "Mario Jeb";
		break;
		case 54:	
			return "Matrix";// 55
		break;
		case 55:	
			return "Matrix Reloaded"; 
		break;
		case 56:	
			return "Metallica";
		break;
		case 57:	
			return "MsgTheme";
		break;
		case 58:	
			return "Muse";
		break;
		case 59:	
			return "Nightmare Before Christmas";//60
		break;
		case 60:	
			return "Old School Nintendo"; 
		break;
		case 61:	
			return "Pink Wii";
		break;
		case 62:	
			return "Psycedelic";
		break;
		case 63:	
			return "Robot Chicken";
		break;
		case 64:	
			return "RockBand"; //65
		break;
		case 65:	
			return "ScarFace";
		break;
		case 66:	
			return "The Simpsons";
		break;
		case 67:	
			return "The Conduit";
		break;
		case 68:	
			return "Vista Theme2";
		break;
		case 69:	
			return "Win XP";// 70
		break;
		case 70:	
			return "BoonDock Saints"; 
		break;
		default:
			return "Unknown";
		break;
	}
}

const char *Theme_Name(int input) {
	switch(input) {
		case 0:
			return "blue";// 0
		break;
		case 1:
			return "fullmetal";
		break;
		case 2:
			return "green";
		break;
		case 3:
			return "orange";
		break;
		case 4:
			return "pink";// 5
		break;
		case 5:
			return "purple";
		break;
		case 6:
			return "red";
		break;
		case 7:
			return "yellow";
		break;
		case 8:
			return "white";
		break;
		case 9:
			return "storm";// 10
		break;
		case 10:
			return "bleach";
		break;
		case 11:
			return "conduit";
		break;
		case 12:
			return "constantine";
		break;
		case 13:
			return "drwho";
		break;
		case 14:
			return "evil";// 15
		break;
		case 15:
			return "gaara";
		break;
		case 16:
			return "golden";
		break;
		case 17:
			return "halo";
		break;
		case 18:
			return "hundred";
		break;
		case 19:	
			return "imports";// 20
		break;
		case 20:	
			return "kingdom";
		break;
		case 21:	
			return "luigi";
		break;
		case 22:	
			return "martin";
		break;
		case 23:	
			return "metroid1";
		break;
		case 24:	
			return "metroid2";// 25
		break;
		case 25:	
			return "mortal";
		break;
		case 26:	
			return "okami";
		break;
		case 27:	
			return "punchout";
		break;
		case 28:	
			return "saw";
		break;
		case 29:	
			return "shadow";//30
		break;
		case 30:	
			return "shadow2";
		break;
		case 31:	
			return"tmnt";
		break;
		case 32:	
			return "tomb";
		break;
		case 33:	
			return "transformers";
		break;
		case 34:	
			return "wolverine";//35
		break;
		case 35:	
			return "zelda";
		break;
		case 36:	
			return "leopard";
		break;
		case 37:	
			return "batman";
		break;
		case 38:	
			return "blackmage";
		break;
		case 39:	
			return "blackPirate";// 40
		break;
		case 40:	
			return "carTheme"; 
		break;
		case 41:	
			return "codeGeass";
		break;
		case 42:	
			return "dethklok";
		break;
		case 43:	
			return "cars";
		break;
		case 44:	
			return "fantasy";// 45
		break;
		case 45:	
			return "firewii"; 
		break;
		case 46:	
			return "ghostbusters";
		break;
		case 47:	
			return "jurassic";
		break;
		case 48:	
			return "lime";
		break;
		case 49:	
			return "madworld";// 50
		break;
		case 50:	
			return "madworld2"; 
		break;
		case 51:	
			return "majora";
		break;
		case 52:	
			return "mario";
		break;
		case 53:	
			return "mariojeb";
		break;
		case 54:	
			return "matrix";// 55
		break;
		case 55:	
			return "matrixr"; 
		break;
		case 56:	
			return "metallica";
		break;
		case 57:	
			return "msgtheme";
		break;
		case 58:	
			return "muse";
		break;
		case 59:	
			return "nightmare";//60
		break;
		case 60:	
			return "nintendo"; 
		break;
		case 61:	
			return "pink";
		break;
		case 62:	
			return "psycedelic";
		break;
		case 63:	
			return "robot";
		break;
		case 64:	
			return "rockband"; //65
		break;
		case 65:	
			return "scarface";
		break;
		case 66:	
			return "bimpsons";
		break;
		case 67:	
			return "conduit";
		break;
		case 68:	
			return "vista2";
		break;
		case 69:	
			return "win";// 70
		break;
		case 70:	
			return "boon"; 
		break;
		default:
			return "Unknown";
		break;
	}
}
int _Extract_Csm() {
	int  ret;
	char *tmpstr = (char*)malloc(256);
	char *tmpstr2 = (char*)malloc(256);
	sprintf(tmpstr2,"%s:/config/themewii/csm_out", getdevicename(thememode));
	sprintf(tmpstr,"%s:/themes/Evil_Dead4.2u.csm",getdevicename(thememode));
	gprintf("tmpstr [%s] \n", tmpstr);
	Undo_U8(tmpstr,tmpstr2);
	
	return ret;
}
int __Theme_Make(char *ThemeFile, u32 count){
	gprintf("Starting Theme Making !! \n");
	char *msg = (char*)malloc(256);
	char *tmpstr = (char*)malloc(256);
	char *tmpstr2 = (char*)malloc(256);
	char *tmpstr3 = (char*)malloc(256);
	int result;
	int  ret;
	u32 k;
	unzFile DBtheme = NULL;
	FILE *Mymini = NULL;
	
	
	sprintf(tmpstr,"%s:/config/themewii/origtheme/%s.app", getdevicename(thememode), getappname(Current_System_Menu_Version));
	if(!Fat_CheckFile(tmpstr)) {
		sprintf(msg,"Baseapp %s.app for Sys-Menu_%s %s Not Found !", getappname(Current_System_Menu_Version), getsysvernum(Current_System_Menu_Version), getregion(Current_System_Menu_Version));
		__Draw_Message(msg,0);
		sleep(3);
		return -1;
	}
	else {
		sprintf(msg,"Baseapp Found in Cache Using it .");
		__Draw_Message(msg,0);
		sleep(3);
	}
	sprintf(tmpstr2,"%s:/config/themewii/app_out", getdevicename(thememode));
	sprintf(tmpstr3,"%s:/config/themewii/app_out/www.arc/nand/input.ini", getdevicename(thememode));
	if(!Fat_CheckFile(tmpstr3)) {
		sprintf(msg,"Extracting Baseapp %s.app for Sys-Menu_%s %s...", getappname(Current_System_Menu_Version), getsysvernum(Current_System_Menu_Version), getregion(Current_System_Menu_Version));
		__Draw_Message(msg,0);
		sleep(3);
		gprintf("tmpstr = %s \ntmpstr2 = %s\n",tmpstr,tmpstr2);
		result = Undo_U8(tmpstr,tmpstr2);
		sprintf(tmpstr,"%s:/config/themewii/app_out",getdevicename(thememode));
		gprintf("tmpstr(%s) \n",tmpstr);
		sprintf(msg,"Extraction of Baseapp Complete .");
		__Draw_Message(msg,0);
		sleep(2);
	}
	sprintf(msg,"Extracting %s for Sys-Menu_%s %s...",ThemeFile, getsysvernum(Current_System_Menu_Version), getregion(Current_System_Menu_Version));
	__Draw_Message(msg,0);
	sleep(2);
	bool found = false;
	gprintf("\n\ncount[%lu] \n\n", count);
	
	for(k = 0;k < count;k++) {
		gprintf("\n\nk[%i] \n\n", k);
		gprintf("\n\nThemeFile [%s] DBThemelist2[k][%s] \n\n", ThemeFile, Theme_Name(k));
		if(strcmp(ThemeFile, Theme_Name(k)) == 0) {
			gprintf("\n\nfound itThemeFile [%s] DBThemelist2[k][%s] \n\n", ThemeFile, Theme_Name(k));	
			sprintf(tmpstr,"%s:/config/themewii/mym/%s4%s.mym",getdevicename(thememode),Theme_Name(k), getdownloadregion(Current_System_Menu_Version));		
			break;
		}
	}
	if(Fat_CheckFile(tmpstr) == false) {
		sprintf(tmpstr,"%s:/config/themewii/mym/%s4.mym",getdevicename(thememode),Theme_Name(k));
		if(Fat_CheckFile(tmpstr) == false) {
			sprintf(tmpstr,"%s:/config/themewii/mym/%s.mym",getdevicename(thememode),Theme_Name(k));
			if(Fat_CheckFile(tmpstr) == false) {
				gprintf("\n\nNot Found .Mym File \n\n");
				return -2;
			}
		}
	}
	
	gprintf("tmpstr =%s \n", tmpstr);
	DBtheme = unzOpen2(tmpstr, NULL);
	if(DBtheme == NULL) {	
		gprintf("\n\nDBTheme NULL \n\n");
	}
	ret = extractZip(DBtheme, 0, 1, NULL, thememode);
	gprintf("ret extractzip(%d) \n", ret);
	if(ret < 0){
		
		sprintf(msg,"Unable to open file to unzip it .");
		__Draw_Message(msg,0);
		sleep(2);
		
	}
	sprintf(msg,"Extraction of Mym file Complete .");
	__Draw_Message(msg,0);
	sleep(2);
	sprintf(tmpstr, "%s:/config/themewii/mym_out/mym.ini", getdevicename(thememode));
	gprintf("tmpstr =%s \n", tmpstr);
	Mymini = fopen(tmpstr, "rb+");
	if(Mymini == NULL) {
		gprintf("\n\nUnable to open file %s \n\n", tmpstr);
		
	}
	fclose(Mymini);
	
	sprintf(msg,"This is it so Far for Now ..... .....");
	__Draw_Message(msg,0);
	sleep(2);
	
	free(msg);
	free(tmpstr);
	free(tmpstr2);
	free(tmpstr3);
	
	return MENU_SELECT_THEME;
}















