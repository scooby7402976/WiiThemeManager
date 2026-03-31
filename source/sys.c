#include <stdio.h>
#include <ogcsys.h>
#include <ogc/system.h>
#include <stdlib.h>
#define MAGIC_WORD_ADDRESS     0x8132FFFB
#define MAGIC_WORD_ADDRESS2    0x817FEFF0 //0x8132FFFB

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
}

int sys_loadmenu(void)
{
	wiilight(1);
	/* Return to the Wii system menu */
	SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
	return 0;
}

int sysHBC()
{
	wiilight(1);
	exit(0);
	return 0;
}
int system_Exit_Priiloader() {
	wiilight(1);
	//retarded that this is the only way without touching the settings of priiloader or load the dol...
	//logfile("magic word is %x\n",*(vu32*)MAGIC_WORD_ADDRESS);
	*(vu32*)MAGIC_WORD_ADDRESS = 0x4461636f; // "Daco" , causes priiloader to skip autoboot and load the priiloader menu
	//*(vu32*)MAGIC_WORD_ADDRESS = 0x50756e65; // "Pune" , causes priiloader to skip autoboot and load Sys Menu
	*(vu32*)MAGIC_WORD_ADDRESS2 = *(vu32*)MAGIC_WORD_ADDRESS;
	DCFlushRange((void*)MAGIC_WORD_ADDRESS, 4);
	DCFlushRange((void*)MAGIC_WORD_ADDRESS2, 4);
	//logfile("magic word changed to %x\n",*(vu32*)MAGIC_WORD_ADDRESS);
	SYS_ResetSystem(SYS_RETURNTOMENU,0,0);
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
