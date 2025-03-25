#ifndef _FAT_H_
#define _FAT_H_

#define SD		0
#define USB		1
#define DEV_MOUNT_SD  "SD"
#define DEV_MOUNT_USB "USB"
#define DEV_UNMOUNT_SD  "SD:"
#define DEV_UNMOUNT_USB "USB:"
/* Prototypes */
s32 Fat_Mount(s32);
s32 Fat_Unmount(s32);
s32 Fat_ReadFile(const char *, void **, bool);
s32 Fat_MakeDir(const char *);
#ifdef __cplusplus
extern "C" {
#endif
bool Fat_CheckFile(const char *);
bool Fat_CreateSubfolder(const char *);
#ifdef __cplusplus
}
#endif
s32 Fat_SaveFile(const char *, void **, u32);
bool Fat_CheckDir(const char *);
#endif
