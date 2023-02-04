#include <stdio.h>
#include <stdlib.h>
#include <ogcsys.h>
#include <string.h> //for parsing parameters
#include <ogc/isfs.h>
#include <errno.h>
#include <gccore.h>

#include "menu.h"
#include "video.h"
#include "wpad.h"
#include "iospatch.h"
#include "fat_mine.h"

void __exception_setreload(int);

int main(int argc, char **argv){
	int ios;
	
	__exception_setreload(5);
	if(AHBPROT_DISABLED) {
		IOSPATCH_AHBPROT();
		IOSPATCH_Apply();
	}
	else {
		ios = IOS_GetVersion();
		IOS_ReloadIOS(ios);
		if(AHBPROT_DISABLED) {
			IOSPATCH_AHBPROT();
			IOSPATCH_Apply();
		}
	}
	Video_Init();
	Wpad_Init();
	ISFS_Initialize();
	Menu_Loop();
	exit(0);
}
