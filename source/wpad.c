#include <stdio.h>
#include <ogcsys.h>
#include "wpad.h"

// Constants
#define MAX_WIIMOTES	4





// Variables
static hotSpot hotSpots[MAXHOTSPOTS];
static bool buttonMode=true;
static ir_t WiimoteIR;
static int WiimoteMX, WiimoteMY;
static u32 WPADKeyDown, PADKeyDown;
//u32 WPADKeyHeld;
static int i;
static int myHotSpot=-1;


/*void __Wpad_PowerCallback(s32 chan){
	// Poweroff console
	Sys_Shutdown();
}*/

void Wpad_AddHotSpot(int pos, u16 x, u16 y, u16 width, u16 height, u16 nextUp, u16 nextDown, u16 nextLeft, u16 nextRight){
	hotSpots[pos].x=x;
	hotSpots[pos].y=y;
	hotSpots[pos].width=width;
	hotSpots[pos].height=height;
	hotSpots[pos].nextUp=nextUp;
	hotSpots[pos].nextDown=nextDown;
	hotSpots[pos].nextLeft=nextLeft;
	hotSpots[pos].nextRight=nextRight;
}
void Wpad_CleanHotSpots(void){
	for(i=0; i<MAXHOTSPOTS; i++)
		Wpad_AddHotSpot(i, 0, 0, 0, 0, 0, 0, 0, 0);
}


hotSpot Wpad_GetHotSpotInfo(int pos){
	return hotSpots[pos];
}

s32 Wpad_Init(void){
	s32 ret;

	// Initialize Wiimotes
	ret = WPAD_Init();
	WPAD_SetIdleTimeout(60); // 60 seconds
	WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);

	if (ret < 0)
		return ret;

	// Initialize GC pads
	ret = PAD_Init();
	if (ret < 0)
		return ret;

#if 0
	// Set POWER button callback
	WPAD_SetPowerButtonCallback(__Wpad_PowerCallback);
#endif

	return ret;
}

void Wpad_Disconnect(void){
	// Disconnect Wiimotes
	for(i= 0; i<MAX_WIIMOTES; i++)
		WPAD_Disconnect(i);

	// Shutdown Wiimote subsystem
	WPAD_Shutdown();
}

int Wpad_Scan(void){
	WPAD_ScanPads();
	WPADKeyDown = WPAD_ButtonsDown(WPAD_CHAN_0);
	PAD_ScanPads();
	PADKeyDown = PAD_ButtonsDown(0);
	//WPADKeyHeld = WPAD_ButtonsHeld(WPAD_CHAN_0);
	WPAD_IR(WPAD_CHAN_0, &WiimoteIR);

	myHotSpot=-1;
	for(i=0; i<MAXHOTSPOTS; i++){
		if(WiimoteMX>hotSpots[i].x && WiimoteMX<hotSpots[i].x+hotSpots[i].width && WiimoteMY>hotSpots[i].y && WiimoteMY<hotSpots[i].y+hotSpots[i].height){
			myHotSpot=i;
			break;
		}
	}


	if(!buttonMode){
		WiimoteMX=WiimoteIR.sx-150; //IR viewport correction
		WiimoteMY=WiimoteIR.sy-150;

		if(PADKeyDown || (WPADKeyDown & (WPAD_BUTTON_UP | WPAD_BUTTON_DOWN | WPAD_BUTTON_LEFT | WPAD_BUTTON_RIGHT))){
			buttonMode=true;
		}
	}else{
		if(myHotSpot==-1){
			myHotSpot=0;
		}else if((WPADKeyDown & WPAD_BUTTON_UP) || (PADKeyDown & PAD_BUTTON_UP)){
			myHotSpot=hotSpots[myHotSpot].nextUp;
		}else if((WPADKeyDown & WPAD_BUTTON_DOWN) || (PADKeyDown & PAD_BUTTON_DOWN)){
			myHotSpot=hotSpots[myHotSpot].nextDown;
		}else if((WPADKeyDown & WPAD_BUTTON_LEFT) || (PADKeyDown & PAD_BUTTON_LEFT)){
			myHotSpot=hotSpots[myHotSpot].nextLeft;
		}else if((WPADKeyDown & WPAD_BUTTON_RIGHT) || (PADKeyDown & PAD_BUTTON_RIGHT)){
			myHotSpot=hotSpots[myHotSpot].nextRight;
		}
		WiimoteMX=hotSpots[myHotSpot].x+hotSpots[myHotSpot].width-8;
		WiimoteMY=hotSpots[myHotSpot].y+hotSpots[myHotSpot].height-8;

		if(WiimoteIR.valid)
			buttonMode=false;
	}

	return myHotSpot;
}

int Wpad_GetWiimoteX(void){
	return WiimoteMX;
}
int Wpad_GetWiimoteY(void){
	return WiimoteMY;
}

u32 Wpad_WaitButtons(void){
	u32 buttons=0;

	for(;;){
		//Obtener botones pulsados de Wiimotes
		WPAD_ScanPads();
		buttons=0;

		buttons=WPAD_ButtonsDown(WPAD_CHAN_0);
		if(buttons){
			return buttons;
		}
		VIDEO_WaitVSync();

		//Obtener botones pulsados de mandos GC
		buttons=0;
		PAD_ScanPads();
		buttons=PAD_ButtonsDown(0);
		if(buttons){
			if(buttons & PAD_BUTTON_UP)
				return WPAD_BUTTON_UP;
			else if(buttons & PAD_BUTTON_DOWN)
				return WPAD_BUTTON_DOWN;
			else if(buttons & PAD_BUTTON_LEFT)
				return WPAD_BUTTON_LEFT;
			else if(buttons & PAD_BUTTON_RIGHT)
				return WPAD_BUTTON_RIGHT;
			else if(buttons & PAD_BUTTON_A)
				return WPAD_BUTTON_A;
			else if(buttons & PAD_BUTTON_B)
				return WPAD_BUTTON_B;
			else if(buttons & PAD_BUTTON_Y)
				return WPAD_BUTTON_1;
			else if(buttons & PAD_BUTTON_X)
				return WPAD_BUTTON_2;
			else if(buttons & PAD_TRIGGER_R)
				return WPAD_BUTTON_PLUS;
			else if(buttons & PAD_TRIGGER_L)
				return WPAD_BUTTON_MINUS;
			else if(buttons & PAD_BUTTON_START)
				return WPAD_BUTTON_HOME;
		}
		VIDEO_WaitVSync();

	}
}
