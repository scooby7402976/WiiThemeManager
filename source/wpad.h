#ifndef _WPAD_H_
#define _WPAD_H_

#include <wiiuse/wpad.h>

typedef struct{
	u16 x, y;
	u16 width, height;

	u16 nextUp, nextDown, nextLeft, nextRight;
} hotSpot;
#define MAXHOTSPOTS		32

// Prototypes
void Wpad_AddHotSpot(int pos, u16, u16, u16, u16, u16, u16, u16, u16);
void Wpad_CleanHotSpots(void);
hotSpot Wpad_GetHotSpotInfo(int pos);

s32  Wpad_Init(void);
void Wpad_Disconnect(void);

int Wpad_Scan(void);
int Wpad_GetWiimoteX(void);
int Wpad_GetWiimoteY(void);
u32 Wpad_WaitButtons(void);

#endif
