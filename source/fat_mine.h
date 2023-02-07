#ifndef _FAT_H_
#define _FAT_H_

#define SD		1
#define USB		2

/* Prototypes */
bool Fat_Mount(int);
s32 Fat_Unmount(void);
s32 Fat_ReadFile(const char *, void **, bool);
int Fat_MakeDir(const char *);
#ifdef __cplusplus
extern "C" {
#endif
bool Fat_CheckFile(const char *);
bool Fat_CreateSubfolder(const char *);
#ifdef __cplusplus
}
#endif
s32 Fat_SaveFile(const char *, void **, u32);

#endif
