#ifndef _VIDEO_H_
#define _VIDEO_H_


typedef struct{
	void* buffer;
	int width;
	int height;
	bool alpha;
	u16 centerx;
	u16 centery;
}MRCtex;


#define CONSOLE_X		224
#define CONSOLE_Y		224
#define CONSOLE_WIDTH	384
#define CONSOLE_HEIGHT	224

/* Prototypes */
void Con_Init(u32, u32, u32, u32);
void Con_Clear(void);
void Con_ClearLine(void);

void Video_Init(void);

void MRC_Draw_Tile(int x, int y, MRCtex* tex, int width, unsigned char tile);
void MRC_Draw_Texture(int x, int y, MRCtex* tex);

void MRC_Draw_String(int x, int y, u32 color, const char* str);
void MRC_Draw_Box(int x, int y, int width, int height, u32 color);
void MRC_Center_Texture(MRCtex* tex, int nTiles);
void MRC_Resize_Texture(MRCtex* tex, int newWidth, int newHeight);

MRCtex* MRC_Load_Texture(void *img);

void MRC_Render_Box(int, int);
void MRC_Render_Screen(void);
void MRC_Draw_Cursor(int x, int y, bool grab);

void MRC_Free_Texture(MRCtex* tex);

void MRC_Init(void);
void MRC_Finish(void);

void __MaskBanner(MRCtex* tex);
MRCtex* __Create_No_Banner(const char* title, int width, int height);

//void MRC_Capture(void);

#endif
