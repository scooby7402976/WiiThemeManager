#ifndef _SYS_H_
#define _SYS_H_

/* Prototypes */
void wiilight(int enable);
void sys_init(void);
int sys_loadmenu(void);
int sysHBC();
int system_Exit_Priiloader();
s32  sys_getcerts(signed_blob **, u32 *);

#endif
