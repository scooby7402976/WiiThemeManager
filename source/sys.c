#include <stdio.h>
#include <ogcsys.h>
#include "gecko.h"
#include <ogc/system.h>

#define HBC_HAXX    0x0001000148415858LL
#define HBC_JODI    0x000100014A4F4449LL
#define HBC_1_0_7   0x00010001AF1BF516LL      

/* Variables */
static const char certs_fs[] ATTRIBUTE_ALIGN(32) = "/sys/cert.sys";
static vu32 *_wiilight_reg = (u32*) 0xCD0000C0;

void wiilight(int enable) // Toggle wiilight (thanks Bool for wiilight source)
{
    u32 val = (*_wiilight_reg & ~0x20);
    if (enable) val |= 0x20;
    *_wiilight_reg = val;
}

void sys_init(void)
{
	/* Initialize video subsytem */
	VIDEO_Init();
	InitGecko();
	USBGeckoOutput();
}

int sys_loadmenu(void)
{
	/* Return to the Wii system menu */
	SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
	return 0;
}

int sysHBC()
{
	WII_Initialize();

    int ret = WII_LaunchTitle(HBC_1_0_7);
    if(ret < 0)
        WII_LaunchTitle(HBC_JODI);

    //Back to system menu if all fails
    SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
	return 0;
}
s32 sys_getcerts(signed_blob **certs, u32 *len)
{
	static signed_blob certificates[0x280] ATTRIBUTE_ALIGN(32);
	s32 fd, ret;

	/* Open certificates file */
	fd = IOS_Open(certs_fs, 1);
	if (fd < 0)
		return fd;

	/* Read certificates */
	ret = IOS_Read(fd, certificates, sizeof(certificates));

	/* Close file */
	IOS_Close(fd);

	/* Set values */
	if (ret > 0) {
		*certs = certificates;
		*len   = sizeof(certificates);
	}

	return ret;
}
