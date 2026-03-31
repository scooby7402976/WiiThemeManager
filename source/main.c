#include <stdio.h>
#include <stdlib.h>
#include <ogc/isfs.h>

#include "menu.h"
#include "video.h"
#include "wpad.h"
#include "iospatch.h"

void __exception_setreload(int);

int main(int argc, char **argv){
	
	
	/* Parse parameters
	if(argc>1){
		int i;
		for(i=1; i<argc; i++){
			if(strncmp("--ios=", argv[i], 6)==0){
				ios=atoi(strchr(argv[i], '=')+1);
			}else if(strncmp("--auto=SD", argv[i], 9)==0){
				mode=EMU_SD;
			}else if(strncmp("--auto=USB", argv[i], 10)==0){
				mode=EMU_USB;
			}else if(strncmp("--partition=", argv[i], 12)==0){
				Set_Partition(atoi(strchr(argv[i], '=')+1));
			}else if(strncmp("--path=", argv[i], 7)==0){
				Set_Path(strchr(argv[i], '=')+1);
			}else if(strncmp("--fullmode=", argv[i], 11)==0){
				Set_FullMode(atoi(strchr(argv[i], '=')+1));
			}
		}
	}*/
	
	__exception_setreload(5);
	if(AHBPROT_DISABLED) {
		IOSPATCH_AHBPROT();
		IOSPATCH_Apply();
	}
	else 
		IOSPATCH_Apply();
	
	Video_Init();
	Wpad_Init();
	ISFS_Initialize();
	Menu_Loop();
	return 0;
}
