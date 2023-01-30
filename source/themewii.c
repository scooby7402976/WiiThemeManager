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
#include "gecko.h"
#include "runtimeiospatch.h"

void __exception_setreload(int);



int main(int argc, char **argv){
	int mode = 0, ios = 58, ret;
	u32 Current_Ios = 0, Current_Ios_Revision = 0;
	
	// Parse parameters
	if(argc > 1){
		int i;
		for(i = 1; i < argc; i++){
			if(strcmp("--ios=", argv[i]) == 0){ // 58
				ios = atoi(strchr(argv[i], '=') + 1);
			}
			if(strcmp("--mode=",argv[i]) == 0){	// sd = 0 usb = 1
				mode = atoi(strchr(argv[i], '=') + 1);
			}
		}
	}
	
	if(InitGecko() == true){
		USBGeckoOutput();
	}
	__exception_setreload(5);
	
	Current_Ios = IOS_GetVersion();
	if(Current_Ios != ios)
		Current_Ios = ios;
	Current_Ios_Revision = IOS_GetRevision();
	gprintf("\n\nIOS: v[%lu] r[%lu] \n\n", Current_Ios, Current_Ios_Revision);
	
	ret = IosPatch_FULL(true, false, false, false, Current_Ios);
	gprintf("\n\nUsing %i patches on ios %lu \n\n", ret, Current_Ios);
	
	// Initialize system
	Video_Init();
	Wpad_Init();	

	// Menu loop
	Menu_Loop(mode);
	// Should always exit at this point
	exit(0);
}
