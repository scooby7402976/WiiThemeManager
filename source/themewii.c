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
int debug = 1;
void __exception_setreload(int);

int main(int argc, char **argv){
	int ahbprot = -1, iospatch = 0, ios;
	//bool foundneek = false;
	__exception_setreload(5);
	/*// Parse parameters
	if(argc>1){
		int i;
		for(i=1; i<argc; i++){
			if(strcmp("--mode=",argv[i])==0){	// sd = 1 usb = 2
				mode=atoi(strchr(argv[i], '=')+1);
			}
		}
	}*/
	
	if(AHBPROT_DISABLED) {
		ahbprot = IOSPATCH_AHBPROT();
		iospatch = IOSPATCH_Apply();
	}
	else {
		ios = IOS_GetVersion();
		IOS_ReloadIOS(ios);
		if(AHBPROT_DISABLED) {
			ahbprot = IOSPATCH_AHBPROT();
			iospatch = IOSPATCH_Apply();
		}
		if(debug) {
			Fat_Mount(1);
		}
	}
	
	logfile("AHBPROT_DISABLED[%d]\n", AHBPROT_DISABLED);
	logfile("ahbprot[%d]\n", ahbprot);
	logfile("iospatch[%d]\n", iospatch);
	
	// Initialize system
	Video_Init();
	Wpad_Init();

	// Menu loop
	Menu_Loop();

	exit(0);
}
