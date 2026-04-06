#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <ogcsys.h>
#include <fat.h>
#include <dirent.h>
#include <sys/stat.h> //for mkdir
#include <string.h>
#include <gccore.h>
#include <sys/dir.h>
#include <unistd.h>
#include <sdcard/wiisd_io.h>
#include <ogc/usbstorage.h>


#include "fat_mine.h"
#include "menu.h"

const DISC_INTERFACE* interface;

s32 Fat_Mount(s32 dev) {
	s32 ret;
	__Draw_Loading(440, 440);
	if(dev == SD)
		interface = &__io_wiisd;
	else if(dev == USB)
		interface = &__io_usbstorage;
	
	// Initialize interface
	ret = interface->startup();
	if(!ret)
		dev = -1;
	__Draw_Loading(440, 440);
	// Mount device
	if(dev == SD) {
		ret = fatMountSimple(DEV_MOUNT_SD, interface);
		if (!ret)
			dev = -2;
		//Sd_Mounted = true;
	}
	if(dev == USB) {
		ret = fatMountSimple(DEV_MOUNT_USB, interface);
		if (!ret)
			dev = -3;
		//Usb_Mounted = true;
	}
	__Draw_Loading(440, 440);
	return dev;
}

s32 Fat_Unmount(s32 dev) {
	s32 ret;
	__Draw_Loading(440, 440);
	// Unmount device
	if(dev == SD) {
		fatUnmount(DEV_UNMOUNT_SD);
		//Sd_Mounted = false;
	}
	if(dev == USB) {
		fatUnmount(DEV_UNMOUNT_USB);
		//Usb_Mounted = false;
	}
	__Draw_Loading(440, 440);
	// Shutdown interface
	ret = interface->shutdown();
	if (!ret)
		return ret;
	__Draw_Loading(440, 440);
	return 1;
}

s32 Fat_ReadFile(const char *filepath, void **outbuf, bool needloading) {
	FILE *fp     = NULL;
	void *buffer = NULL;

	u32 filelen;
	s32 ret, h = 0;

	/* Open file */
	fp = fopen(filepath, "rb");
	if (!fp)
		goto err;

	/* Get filesize */
	fseek(fp, 0, SEEK_END);
	filelen = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	/* Allocate memory */
	buffer = malloc(filelen);
	if (!buffer)
		goto err;
	if(needloading) __Draw_Loading(440, 440);
	/* Read file */
	ret = fread(buffer, 1, filelen, fp);
	if (ret != filelen)
		goto err;
	while(h != filelen) {
		h = ret;
		if(needloading) __Draw_Loading(440, 440);
	}
	/* Set pointer */
	*outbuf = buffer;

	goto out;

err:
	/* Free memory */
	if (buffer)
		free(buffer);

	/* Error code */
	ret = -1;

out:
	/* Close file */
	if (fp)
		fclose(fp);

	return ret;
}

int Fat_MakeDir(const char *dirname) {
	int ret=-1;
	DIR *dir;

	//if(dirname[strlen(dirname)-2]=='/')
	//	return false;
		
	dir=opendir(dirname);
	if(dir){
		ret=1;
		closedir(dir);
	}else{
		mkdir(dirname, S_IREAD | S_IWRITE);
		ret=0;
	}

	return ret;
}

bool Fat_CheckFile(const char *filepath) {
	FILE *fp = NULL;

	/* Open file */
	fp = fopen(filepath, "rb");
	if(!fp)
		return false;

	fclose(fp);

	return true;
}

s32 Fat_SaveFile(const char *filepath, void **outbuf, u32 outlen) {
	s32 ret, h = 0;
	FILE *fd;
	
	//__Draw_Loading(440, 440);
	fd = fopen(filepath, "wb");
	if(fd){
		__Draw_Loading(440, 440);
		ret=fwrite(*outbuf, 1, outlen, fd);
		while(h != outlen) {
			h = ret;
			__Draw_Loading(440, 440);
		}
		fclose(fd);
		
	}
	else{
		ret=-1;
	}
	//__Draw_Loading(440, 440);
	return ret;
}

bool CheckFile(const char * filepath) {
    if(!filepath)
        return false;

    struct stat filestat;

    char dirnoslash[strlen(filepath)+2];
    snprintf(dirnoslash, sizeof(dirnoslash), "%s", filepath);

    while(dirnoslash[strlen(dirnoslash)-1] == '/') {
        dirnoslash[strlen(dirnoslash)-1] = '\0';
	}
	char * notRoot = strrchr(dirnoslash, '/');
	if(!notRoot)
	{
	    strcat(dirnoslash, "/");
	}

    if (stat(dirnoslash, &filestat) == 0)
        return true;

    return false;
}
bool Fat_CreateSubfolder(const char * fullpath) {
    if(!fullpath)
        return false;

    bool result  = false;

    char dirnoslash[strlen(fullpath)+1];
    strcpy(dirnoslash, fullpath);

    int pos = strlen(dirnoslash)-1;
    while(dirnoslash[pos] == '/')
    {
        dirnoslash[pos] = '\0';
        pos--;
    }

    if(CheckFile(dirnoslash))
    {
        return true;
    }
    else
    {
        char parentpath[strlen(dirnoslash)+2];
        strcpy(parentpath, dirnoslash);
        char * ptr = strrchr(parentpath, '/');

        if(!ptr)
        {
            //!Device root directory (must be with '/')
            strcat(parentpath, "/");
            struct stat filestat;
            if (stat(parentpath, &filestat) == 0)
                return true;

            return false;
        }

        ptr++;
        ptr[0] = '\0';

        result = Fat_CreateSubfolder(parentpath);
    }

    if(!result)
        return false;

    if (mkdir(dirnoslash, S_IREAD | S_IWRITE) == -1)
    {
        return false;
    }

    return true;
}
bool Fat_CheckDir(const char *dirname) {
	DIR *dir;
	
	dir = opendir(dirname);
	
	if(dir) {
		closedir(dir);
		return true;
	}
	else return false;
}
