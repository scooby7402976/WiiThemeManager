#include <stdio.h>
#include <ogcsys.h>
#include <stdlib.h> //for malloc

#include "video.h"
#include "libpng/png.h"
#include "libpng/pngu/pngu.h"
#include "wpad.h"
#include "tools.h"

#include "cursor_png.h"
#include "font_png.h"

// Video variables
u32* framebuffer = NULL;
GXRModeObj *vmode = NULL;
static char* framebufferRGBA = NULL;

// Iterators
static int ix, iy;

// Common textures
static MRCtex* font;
static MRCtex* cursor;
static MRCtex* cursorBuffer;

void Con_Init(u32 x, u32 y, u32 w, u32 h){
	// Create console in the framebuffer
	CON_InitEx(vmode, x, y, w, h);
}

void Con_Clear(void){
	// Clear console
	printf("\x1b[2J");
	fflush(stdout);
}

void Con_ClearLine(void){
	int cols, rows, cnt;

	printf("\r");
	fflush(stdout);

	// Get console metrics
	CON_GetMetrics(&cols, &rows);

	// Erase line
	for(cnt = 1; cnt < cols; cnt++){
		printf(" ");
		fflush(stdout);
	}

	printf("\r");
	fflush(stdout);
}

void Video_Init(void){
	VIDEO_Init();
	vmode = VIDEO_GetPreferredMode(0);
	framebuffer = MEM_K0_TO_K1(SYS_AllocateFramebuffer(vmode));
	VIDEO_Configure(vmode);
	VIDEO_SetNextFramebuffer(framebuffer);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(vmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();

	VIDEO_ClearFrameBuffer(vmode, framebuffer, COLOR_BLACK);
}



void MRC_Draw_Tile(int x, int y, MRCtex* tex, int width, unsigned char tile){
	char *c, *bg, a;
	int initX, maxX, initY, maxY;
	x-=tex->centerx;
	y-=tex->centery;
	initX=x<0? -x : 0;
	maxX=x+width>640? 640-x: width;
	initY=y<0? -y : 0;
	maxY=y+tex->height>480? 480-y: tex->height;
	for (iy=initY; iy<maxY; iy++){
		bg = (char*)framebufferRGBA + (y+iy)*(vmode->fbWidth * 4) + (x+initX)*4;
		c=(char*)(tex->buffer)+(iy*tex->width+initX+width*tile)*4;
		for(ix=initX; ix<maxX; ix++){
			a=c[3];
			if(!tex->alpha || a==255){
				bg[0]=c[0]; //R
				bg[1]=c[1]; //G
				bg[2]=c[2]; //B
				bg[3]=255; //A
			}else{
				png_composite(bg[0], c[0], a, bg[0]); //R
				png_composite(bg[1], c[1], a, bg[1]); //G
				png_composite(bg[2], c[2], a, bg[2]); //B
			}
			c += 4;
			bg += 4;
		}
	}
}

void MRC_Draw_Texture(int x, int y, MRCtex* tex){
	MRC_Draw_Tile(x, y, tex, tex->width, 0);
}


#define FONT_WIDTH 8
void MRC_Draw_String(int x, int y, u32 color, const char* str){
	unsigned char ch, r, g, b;
	char *c, *bg;
	int it=0;
	r=(color>>24)&0xff;
	g=(color>>16)&0xff;
	b=(color>>8)&0xff;
	//a=(color)&0xff;
	
	for(;;){
		ch=(unsigned char)str[it];
		if(ch=='\0')
			break;
		else if(ch>128)
			ch-=32;
		ch-=32;
		//MRC_Draw_Tile(x, y, font, FONT_WIDTH, c);
		c = (char*)(font->buffer)+ch*FONT_WIDTH*4;
		for (iy=0; iy<font->height; iy++){
			bg = (char*)framebufferRGBA + (y+iy)*(vmode->fbWidth * 4) + x*4;
			for(ix=0; ix<FONT_WIDTH; ix++){
				png_composite(bg[0], r, c[3], bg[0]); // r
				png_composite(bg[1], g, c[3], bg[1]); // g
				png_composite(bg[2], b, c[3], bg[2]); // b
				c += 4;
				bg += 4;
			}
			c+=(font->width-FONT_WIDTH)*4;
		}
		it++;
		x+=FONT_WIDTH;
	}
}

void MRC_Draw_Box(int x, int y, int width, int height, u32 color){
	unsigned char r,g,b,a;
	r=(color>>24)&0xff;
	g=(color>>16)&0xff;
	b=(color>>8)&0xff;
	a=(color)&0xff;
	char *bg;
	for (iy=0; iy<height; iy++){
		bg = (char*)framebufferRGBA + (y+iy)*(vmode->fbWidth * 4) + x*4;
		for (ix=0; ix<width; ix++) {
			png_composite(bg[0], r, a, bg[0]); // r
			png_composite(bg[1], g, a, bg[1]); // g
			png_composite(bg[2], b, a, bg[2]); // b
			bg += 4;
		}
	}
}

void MRC_Center_Texture(MRCtex* tex, int nTiles){
	tex->centerx=(tex->width/nTiles)/2;
	tex->centery=tex->height/2;
}

void MRC_Resize_Texture(MRCtex* tex, int newWidth, int newHeight) {
	int i, j;
	int w=tex->width;
	int h=tex->height;
	if(newWidth==w && newHeight==h)
		return;
	int* pixels=tex->buffer;
	int a, b, c, d, x, y, index ;
	float x_ratio = ((float)(w-1))/newWidth;
	float y_ratio = ((float)(h-1))/newHeight;
	float x_diff, y_diff, red, green, blue;
	//float alpha;
	int offset = 0;

	int* temp=allocate_memory(sizeof(int)*newWidth*newHeight);

	for(i=0;i<newHeight;i++){
		for(j=0;j<newWidth;j++){
			x = (int)(x_ratio * j) ;
			y = (int)(y_ratio * i) ;
			x_diff = (x_ratio * j) - x ;
			y_diff = (y_ratio * i) - y ;
			index = (y*w+x) ;                
			a = pixels[index] ;
			b = pixels[index+1] ;
			c = pixels[index+w] ;
			d = pixels[index+w+1] ;

			// red element
			// Yr = Ar(1-w)(1-h) + Br(w)(1-h) + Cr(h)(1-w) + Dr(wh)
			red = ((a>>24)&0xff)*(1-x_diff)*(1-y_diff) + ((b>>24)&0xff)*(x_diff)*(1-y_diff) +
				((c>>24)&0xff)*(y_diff)*(1-x_diff)   + ((d>>24)&0xff)*(x_diff*y_diff);

			// green element
			// Yg = Ag(1-w)(1-h) + Bg(w)(1-h) + Cg(h)(1-w) + Dg(wh)
			green = ((a>>16)&0xff)*(1-x_diff)*(1-y_diff) + ((b>>16)&0xff)*(x_diff)*(1-y_diff) +
				((c>>16)&0xff)*(y_diff)*(1-x_diff)   + ((d>>16)&0xff)*(x_diff*y_diff);

			// blue element
			// Yb = Ab(1-w)(1-h) + Bb(w)(1-h) + Cb(h)(1-w) + Db(wh)
			blue = ((a>>8)&0xff)*(1-x_diff)*(1-y_diff) + ((b>>8)&0xff)*(x_diff)*(1-y_diff) +
				((c>>8)&0xff)*(y_diff)*(1-x_diff)   + ((d>>8)&0xff)*(x_diff*y_diff);

			// alpha
			//alpha = ((a)&0xff)*(1-x_diff)*(1-y_diff) + ((b)&0xff)*(x_diff)*(1-y_diff) +
			//	((c)&0xff)*(y_diff)*(1-x_diff)   + ((d)&0xff)*(x_diff*y_diff);

			temp[offset++] = 
				((((int)red)<<24)&0xff000000) | 
				((((int)green)<<16)&0xff0000) |
				((((int)blue)<<8)&0xff00) |
			//	((((int)alpha))&0xff);
				0x000000ff ; // hardcode alpha
		}
	}
	free(tex->buffer);
	tex->buffer=temp;
	tex->width=newWidth;
	tex->height=newHeight;
}
void MRC_Center_Texture1(MRCtex* tex, int nTiles){
	tex->centerx=(tex->width/nTiles)/2;
	tex->centery=tex->height/2;
}

void MRC_Resize_Texture1(MRCtex* tex, int newWidth, int newHeight) {
	int i, j;
	int w=tex->width;
	int h=tex->height;
	if(newWidth==w && newHeight==h)
		return;
	int* pixels=tex->buffer;
	int a, b, c, d, x, y, index ;
	float x_ratio = ((float)(w-1))/newWidth;
	float y_ratio = ((float)(h-1))/newHeight;
	float x_diff, y_diff, red, green, blue;
	//float alpha;
	int offset = 0;

	int* temp=allocate_memory(sizeof(int)*newWidth*newHeight);

	for(i=0;i<newHeight;i++){
		for(j=0;j<newWidth;j++){
			x = (int)(x_ratio * j) ;
			y = (int)(y_ratio * i) ;
			x_diff = (x_ratio * j) - x ;
			y_diff = (y_ratio * i) - y ;
			index = (y*w+x) ;                
			a = pixels[index] ;
			b = pixels[index+1] ;
			c = pixels[index+w] ;
			d = pixels[index+w+1] ;

			// red element
			// Yr = Ar(1-w)(1-h) + Br(w)(1-h) + Cr(h)(1-w) + Dr(wh)
			red = ((a>>24)&0xff)*(1-x_diff)*(1-y_diff) + ((b>>24)&0xff)*(x_diff)*(1-y_diff) +
				((c>>24)&0xff)*(y_diff)*(1-x_diff)   + ((d>>24)&0xff)*(x_diff*y_diff);

			// green element
			// Yg = Ag(1-w)(1-h) + Bg(w)(1-h) + Cg(h)(1-w) + Dg(wh)
			green = ((a>>16)&0xff)*(1-x_diff)*(1-y_diff) + ((b>>16)&0xff)*(x_diff)*(1-y_diff) +
				((c>>16)&0xff)*(y_diff)*(1-x_diff)   + ((d>>16)&0xff)*(x_diff*y_diff);

			// blue element
			// Yb = Ab(1-w)(1-h) + Bb(w)(1-h) + Cb(h)(1-w) + Db(wh)
			blue = ((a>>8)&0xff)*(1-x_diff)*(1-y_diff) + ((b>>8)&0xff)*(x_diff)*(1-y_diff) +
				((c>>8)&0xff)*(y_diff)*(1-x_diff)   + ((d>>8)&0xff)*(x_diff*y_diff);

			// alpha
			//alpha = ((a)&0xff)*(1-x_diff)*(1-y_diff) + ((b)&0xff)*(x_diff)*(1-y_diff) +
			//	((c)&0xff)*(y_diff)*(1-x_diff)   + ((d)&0xff)*(x_diff*y_diff);

			temp[offset++] = 
				((((int)red)<<24)&0xff000000) | 
				((((int)green)<<16)&0xff0000) |
				((((int)blue)<<8)&0xff00) |
			//	((((int)alpha))&0xff);
				0x000000ff ; // hardcode alpha
		}
	}
	free(tex->buffer);
	tex->buffer=temp;
	tex->width=newWidth;
	tex->height=newHeight;
}

void MRC_Center_Texture2(MRCtex* tex2, int nTiles2){
	tex2->centerx=(tex2->width/nTiles2)/2;
	tex2->centery=tex2->height/2;
}
void MRC_Resize_Texture2(MRCtex* tex2, int newWidth2, int newHeight2) {
	int i, j;
	int w=tex2->width;
	int h=tex2->height;
	if(newWidth2==w && newHeight2==h)
		return;
	int* pixels=tex2->buffer;
	int a, b, c, d, x, y, index ;
	float x_ratio = ((float)(w-1))/newWidth2;
	float y_ratio = ((float)(h-1))/newHeight2;
	float x_diff, y_diff, red, green, blue;
	//float alpha;
	int offset = 0;

	int* temp=allocate_memory(sizeof(int)*newWidth2*newHeight2);

	for(i=0;i<newHeight2;i++){
		for(j=0;j<newWidth2;j++){
			x = (int)(x_ratio * j) ;
			y = (int)(y_ratio * i) ;
			x_diff = (x_ratio * j) - x ;
			y_diff = (y_ratio * i) - y ;
			index = (y*w+x) ;                
			a = pixels[index] ;
			b = pixels[index+1] ;
			c = pixels[index+w] ;
			d = pixels[index+w+1] ;

			// red element
			// Yr = Ar(1-w)(1-h) + Br(w)(1-h) + Cr(h)(1-w) + Dr(wh)
			red = ((a>>24)&0xff)*(1-x_diff)*(1-y_diff) + ((b>>24)&0xff)*(x_diff)*(1-y_diff) +
				((c>>24)&0xff)*(y_diff)*(1-x_diff)   + ((d>>24)&0xff)*(x_diff*y_diff);

			// green element
			// Yg = Ag(1-w)(1-h) + Bg(w)(1-h) + Cg(h)(1-w) + Dg(wh)
			green = ((a>>16)&0xff)*(1-x_diff)*(1-y_diff) + ((b>>16)&0xff)*(x_diff)*(1-y_diff) +
				((c>>16)&0xff)*(y_diff)*(1-x_diff)   + ((d>>16)&0xff)*(x_diff*y_diff);

			// blue element
			// Yb = Ab(1-w)(1-h) + Bb(w)(1-h) + Cb(h)(1-w) + Db(wh)
			blue = ((a>>8)&0xff)*(1-x_diff)*(1-y_diff) + ((b>>8)&0xff)*(x_diff)*(1-y_diff) +
				((c>>8)&0xff)*(y_diff)*(1-x_diff)   + ((d>>8)&0xff)*(x_diff*y_diff);

			// alpha
			//alpha = ((a)&0xff)*(1-x_diff)*(1-y_diff) + ((b)&0xff)*(x_diff)*(1-y_diff) +
			//	((c)&0xff)*(y_diff)*(1-x_diff)   + ((d)&0xff)*(x_diff*y_diff);

			temp[offset++] = 
				((((int)red)<<24)&0xff000000) | 
				((((int)green)<<16)&0xff0000) |
				((((int)blue)<<8)&0xff00) |
			//	((((int)alpha))&0xff);
				0x000000ff ; // hardcode alpha
		}
	}
	free(tex2->buffer);
	tex2->buffer=temp;
	tex2->width=newWidth2;
	tex2->height=newHeight2;
}


#define MASK_W 32
#define MASK_H 32
static const u8 mask[MASK_H][MASK_W]={
	{  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 208, 208, 208, 208, 208, 208, 208},
	{  0,   0,   0,   0,   0,   0,   0,   0, 128, 128, 208, 208, 208, 208, 208, 208, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
	{  0,   0,   0,   0,   0,   0, 128, 208, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
	{  0,   0,   0,   0, 128, 208, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
	{  0,   0,   0, 128, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
	{  0,   0, 128, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
	{  0,   0, 208, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
	{  0,   0, 208, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
	{  0,   0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
	{  0, 128, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
	{  0, 128, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
	{  0, 208, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
	{  0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
	{  0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
	{  0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
	{  0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
	{  0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
	{128, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
	{128, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
	{128, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
	{128, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
	{208, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
	{208, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
	{208, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
	{208, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
	{208, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
	{208, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
	{208, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
	{208, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
	{208, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
	{208, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255},
	{208, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255}
};


void __MaskBanner(MRCtex* tex){
	char* puntero=tex->buffer;
	for(ix=0; ix<MASK_W; ix++){
		for(iy=0; iy<MASK_H; iy++){
			// top-left corner
			puntero[(iy*tex->width+ix)*4+3]=mask[ix][iy];

			// top-right corner
			puntero[(iy*tex->width+(tex->width-1-ix))*4+3]=mask[ix][iy];

			// bottom-left corner
			puntero[(tex->width*(tex->height-1-iy)+ix)*4+3]=mask[ix][iy];

			// bottom-right corner
			puntero[(tex->width*(tex->height-1-iy)+(tex->width-1-ix))*4+3]=mask[ix][iy];
		}
	}
	tex->alpha=true;
}

MRCtex* __Create_No_Banner( const char* title, int width, int height){
	MRCtex* newTex=allocate_memory(sizeof(MRCtex));
	
	MRC_Draw_Box(0, 0, width, height, 0x303850FF);
	//MRC_Draw_String(10,40, 0xFFFFFFFF, );
	MRC_Draw_String(10,25, 0xFFFFFFFF, title);

	newTex->width=width;
	newTex->height=height;
	newTex->centerx=0;
	newTex->centery=0;
	newTex->buffer=allocate_memory(width*height*4);

	int* temp=allocate_memory(sizeof(int)*width*height);
	for(iy=0; iy<height; iy++){
		for(ix=0; ix<width; ix++){
			int* puntero = (int*)framebufferRGBA;
			temp[iy*newTex->width+ix]=puntero[iy*vmode->fbWidth+ix];
		}
	}
	newTex->buffer=temp;
	return newTex;
}

MRCtex* MRC_Load_Texture(void *img){
	IMGCTX   ctx = NULL;
	PNGUPROP imgProp;
	int ret;
	MRCtex* tex=NULL;
	// Select PNG data
	ctx = PNGU_SelectImageFromBuffer(img);
	if(!ctx){
		ret = -1;
		goto out;
	}

	// Get image properties
	ret=PNGU_GetImageProperties(ctx, &imgProp);
	if(ret!=PNGU_OK){
		ret = -1;
		goto out;
	}
	tex=allocate_memory(sizeof(MRCtex));
	tex->width=imgProp.imgWidth;
	tex->height=imgProp.imgHeight;
	tex->buffer=allocate_memory(imgProp.imgWidth*imgProp.imgHeight*4);
	tex->alpha=(imgProp.imgColorType==PNGU_COLOR_TYPE_RGB_ALPHA);
	tex->centerx=0;
	tex->centery=0;

	// Decode PNG
	PNGU_DECODE_TO_COORDS_RGBA8(ctx, 0, 0, imgProp.imgWidth, imgProp.imgHeight, 255, imgProp.imgWidth, imgProp.imgHeight, tex->buffer);


out:
	// Free memory
	if (ctx)
		PNGU_ReleaseImageContext(ctx);

	if(ret==-1)
		return NULL;
	else
		return tex;
}


void __Render_Lines(int y){
	if(y<0)
		y=0;
	char* bg=framebufferRGBA+y*vmode->fbWidth*4;
	int maxBuffWidth=vmode->xfbHeight*(vmode->fbWidth/2);
	for(ix=y*(vmode->fbWidth/2); ix<maxBuffWidth; ix++){
		((PNGU_u32 *)framebuffer)[ix]=PNGU_RGB8_TO_YCbYCr(bg[0], bg[1], bg[2], bg[4], bg[5], bg[6]);
		bg+=8;
	}
	//VIDEO_Flush();
	//VIDEO_WaitVSync();
}

void MRC_Render_Box(int x, int y){
	if(y<0)
		y=0;
	if(x<0)
		x=0;
	char* bg=framebufferRGBA+(y*vmode->fbWidth)*4;
	int buffWidth=vmode->fbWidth/2;

	x=x/2;
	for(iy=y; iy<vmode->xfbHeight; iy++){
		bg+=x*8;
		for(ix=x; ix<buffWidth; ix++){
			((PNGU_u32 *)framebuffer)[iy*buffWidth+ix]=PNGU_RGB8_TO_YCbYCr(bg[0], bg[1], bg[2], bg[4], bg[5], bg[6]);
			bg+=8;
		}
		//bg+=8*x;
	}

	//VIDEO_Flush();
	//VIDEO_WaitVSync();
}
void MRC_Render_Screen(void){
	/*
	char* ip=framebufferRGBA;
	int buffWidth=vmode->fbWidth/2;
	for(iy=0; iy<vmode->xfbHeight; iy++){
		for(ix=0; ix<buffWidth; ix++){
			((PNGU_u32 *)framebuffer)[iy*buffWidth+ix]=PNGU_RGB8_TO_YCbYCr(ip[0], ip[1], ip[2], ip[4], ip[5], ip[6]);
			ip+=8;
		}
	}
	*/
	char* bg=framebufferRGBA;
	int maxBuffWidth=vmode->xfbHeight*(vmode->fbWidth/2);
	for(ix=0; ix<maxBuffWidth; ix++){
		((PNGU_u32 *)framebuffer)[ix]=PNGU_RGB8_TO_YCbYCr(bg[0], bg[1], bg[2], bg[4], bg[5], bg[6]);
		bg+=8;
	}
	//VIDEO_Flush();
	//VIDEO_WaitVSync();
}

#define CURSOR_WIDTH 36
void MRC_Draw_Cursor(int x, int y, bool grab){
	char *bg, *c;
	c=cursorBuffer->buffer;
	for (iy=0; iy<cursor->height; iy++){
		bg = framebufferRGBA + (y+iy)*(vmode->fbWidth * 4) + (x-cursor->centerx) * 4;
		for (ix=0; ix<CURSOR_WIDTH; ix++) {
			c[0]=bg[0];
			c[1]=bg[1];
			c[2]=bg[2];
			c += 4;
			bg += 4;
		}
	}

	MRC_Draw_Tile(x,y, cursor, 36, grab);
	MRC_Render_Screen();
	MRC_Draw_Texture(x,y, cursorBuffer);

	//VIDEO_Flush();
	//VIDEO_WaitVSync();
}


void MRC_Free_Texture(MRCtex* tex){
	free(tex->buffer);
	free(tex);
}


void MRC_Init(void){
	framebufferRGBA=allocate_memory(vmode->fbWidth*vmode->xfbHeight*4);
	WPAD_SetVRes(WPAD_CHAN_0, vmode->fbWidth, vmode->xfbHeight);

	font=MRC_Load_Texture((void*)font_png);

	cursor=MRC_Load_Texture((void*)cursor_png);
	cursorBuffer=allocate_memory(sizeof(MRCtex));
	cursorBuffer->width=CURSOR_WIDTH;
	cursorBuffer->height=cursor->height;
	cursorBuffer->alpha=false;
	cursorBuffer->buffer=allocate_memory(CURSOR_WIDTH*cursor->height*4);
	cursor->centerx=10;
	cursorBuffer->centerx=10;
	cursorBuffer->centery=0;
}

void MRC_Finish(void){
	MRC_Free_Texture(font);
	MRC_Free_Texture(cursor);
	MRC_Free_Texture(cursorBuffer);
	free(framebufferRGBA);
}




/*void MRC_Capture(void){
	u32 *copiaFrame=NULL;
	IMGCTX pngContext;
	int ret;
	int i;

	copiaFrame=malloc(sizeof(u32)*320*480);
	for(i=0;i<320*480;i++)
		copiaFrame[i]=framebuffer[i];



	pngContext = PNGU_SelectImageFromDevice("fat:/mighty_channels.png");
	if(pngContext){
		ret = PNGU_EncodeFromYCbYCr(pngContext, 640, 480, copiaFrame, 0);
		PNGU_ReleaseImageContext(pngContext);
	}

	for(i=0;i<320*480;i++)
		framebuffer[i]=copiaFrame[i];
	free(copiaFrame);
}*/

