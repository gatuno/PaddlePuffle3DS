/*
 * main.c
 * This file is part of Paddle Puffle
 *
 * Copyright (C) 2015 - Félix Arreola Rodríguez
 *
 * Paddle Puffle is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Paddle Puffle is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Paddle Puffle. If not, see <http://www.gnu.org/licenses/>.
 */

#include <3ds.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "normal_bgr.h"
#include "blue_bgra.h"

/* Autómata para un puffle */
enum {
	PUFFLE_NORMAL = 0,
	PUFFLE_FALL,
	PUFFLE_BOUNCE,
	NUM_PUFFLE_MODS
};

enum {
	IMG_BLUE_NORMAL_1 = 0,
	IMG_BLUE_FALL_1,
	IMG_BLUE_FALL_2,
	IMG_BLUE_FALL_3,
	IMG_BLUE_BOUNCE_1,
	IMG_BLUE_BOUNCE_2,
	IMG_BLUE_BOUNCE_3,
	IMG_BLUE_BOUNCE_4
};

static int puffle_frames [17][NUM_PUFFLE_MODS] = {
	{0, 1, 8},
	{2, 1, 8},
	{3, 2, 8},
	{4, 3, 8},
	{5, 4, 8},
	{6, 5, 8},
	{7, 6, 8},
	{4, 7, 8},
	{9, 8, 8},
	{10, 9, 9},
	{11, 10, 10},
	{12, 11, 11},
	{13, 12, 12},
	{14, 13, 13},
	{15, 14, 14},
	{16, 15, 15},
	{0, 16, 16}
};

static int puffle_outputs [17] = {
	IMG_BLUE_NORMAL_1,
	IMG_BLUE_FALL_1,
	IMG_BLUE_FALL_1,
	IMG_BLUE_FALL_1,
	IMG_BLUE_FALL_2,
	IMG_BLUE_FALL_2,
	IMG_BLUE_FALL_3,
	IMG_BLUE_FALL_3,
	IMG_BLUE_BOUNCE_1,
	IMG_BLUE_BOUNCE_1,
	IMG_BLUE_BOUNCE_1,
	IMG_BLUE_BOUNCE_2,
	IMG_BLUE_BOUNCE_2,
	IMG_BLUE_BOUNCE_3,
	IMG_BLUE_BOUNCE_3,
	IMG_BLUE_BOUNCE_4,
	IMG_BLUE_BOUNCE_4
};

/* La estructura principal de un puffle */
typedef struct _Puffle {
	struct _Puffle *next;
	struct _Puffle *prev;
	int x;
	int y;
	s16 x_fixed;
	s16 y_fixed;
	int pop_num;
	int color;
	int frame;
} Puffle;

/* Prototipos de funciones */
void setup (void);
void nuevo_puffle (void);

/* Variables Globales */
Puffle *first_puffle = NULL;
Puffle *last_puffle = NULL;

u8 *puffles_images[8];

void gfxDrawSprite (gfxScreen_t screen, gfx3dSide_t side, u8* spriteData, u16 width, u16 height, s16 x, s16 y) {
	if (!spriteData) return;

	u16 fbWidth, fbHeight;
	u8* fbAdr = gfxGetFramebuffer (screen, side, &fbWidth, &fbHeight);
	//printf ("FB me informa: %i, %i\n", fbWidth, fbHeight);
	if (x + width < 0 || x >= fbWidth) return;
	if (y + height < 0 || y >= fbHeight) return;
	
	u16 xOffset = 0, yOffset = 0;
	u16 widthDrawn = width, heightDrawn = height;
	
	if (x < 0) xOffset = -x;
	if (y < 0) yOffset = -y;
	if (x + width >= fbWidth) widthDrawn = fbWidth - x;
	if (y + height >= fbHeight) heightDrawn = fbHeight - y;
	widthDrawn -= xOffset;
	heightDrawn -= yOffset;
	//printf ("Offset = (%i, %i), Draw: (%i, %i)\n", xOffset, yOffset, widthDrawn, heightDrawn);

	int j;
	for (j = yOffset; j < yOffset + heightDrawn; j++) {
		memcpy(&fbAdr[((x+xOffset)+(y+j)*fbWidth)*3], &spriteData[((xOffset)+(j)*width)*3], widthDrawn*3);
	}
}

void gfxDrawTransSprite (gfxScreen_t screen, gfx3dSide_t side, u8* spriteData, u16 width, u16 height, s16 x, s16 y) {
	if (!spriteData) return;

	u16 fbWidth, fbHeight;
	u8* fbAdr = gfxGetFramebuffer (screen, side, &fbWidth, &fbHeight);

	if (x + width < 0 || x >= fbWidth) return;
	if (y + height < 0 || y >= fbHeight) return;

	u16 xOffset=0, yOffset=0;
	u16 widthDrawn=width, heightDrawn=height;

	if (x < 0) xOffset = -x;
	if (y < 0) yOffset = -y;
	if (x+width >= fbWidth) widthDrawn = fbWidth - x;
	if (y+height >= fbHeight) heightDrawn = fbHeight - y;
	widthDrawn -= xOffset;
	heightDrawn -= yOffset;

	//TODO : optimize
	fbAdr += (y + yOffset) * fbWidth * 3;
	spriteData += yOffset * width * 4; /* La imagen es ABGR */
	int j, i;
	u8 alpha;
	for (j = yOffset; j < yOffset + heightDrawn; j++) {
		u8* fbd = &fbAdr[(x+xOffset) * 3];
		u8* data = &spriteData[(xOffset) * 4]; /* La imagen es ABGR */
		for (i = xOffset; i < xOffset + widthDrawn; i++) {
			alpha = data[3];
			fbd[0] = ((data[0] * alpha) + (fbd[0] * (255 - alpha))) / 256;
			fbd[1] = ((data[1] * alpha) + (fbd[1] * (255 - alpha))) / 256;
			fbd[2] = ((data[2] * alpha) + (fbd[2] * (255 - alpha))) / 256;
			fbd += 3;
			data += 4; /* La imagen tiene alpha */
		}
		fbAdr += fbWidth * 3;
		spriteData += width * 4;
	}
}

void game_loop (void) {
	int done = 0;
	Puffle *thispuffle;
	
	u32 keys;
	
	nuevo_puffle ();
	while (aptMainLoop () && !done) {
		hidScanInput ();
		keys = hidKeysDown ();
		
		if (keys & KEY_START) {
			done = 1;
		}
		
		if (keys & KEY_A) {
			first_puffle->frame = puffle_frames [first_puffle->frame][PUFFLE_BOUNCE];
			first_puffle->y_fixed = 0;
		} else if (keys & KEY_B) {
			first_puffle->y_fixed = (12 << 8);
		}
		
		gfxDrawSprite (GFX_TOP, GFX_LEFT, (u8*)normal_bgr, 240, 400, 0, 0);
		
		thispuffle = first_puffle;
		do {
			/* Avanzar de posición los puffles
			thispuffle->x = thispuffle->x + thispuffle->x_fixed;
			thispuffle->y = thispuffle->y + thispuffle->y_fixed;
			 */
			
			if (thispuffle->y_fixed > (10 << 8)) {
				printf ("El puffle está cayendo\n");
				thispuffle->frame = puffle_frames [thispuffle->frame][PUFFLE_FALL];
			}
			
			thispuffle->frame = puffle_frames [thispuffle->frame][PUFFLE_NORMAL];
			if (thispuffle != NULL) thispuffle = thispuffle->next;
		} while (thispuffle != NULL);
		
		thispuffle = first_puffle;
		do {
			//if (thispuffle->y > -100) {
				/* Blit this puffle */
				//puf_pos.x = thispuffle->x - 48; /* Constante temporal */
				//puf_pos.y = thispuffle->y - 60; /* Constante temporal */
				gfxDrawTransSprite (GFX_TOP, GFX_LEFT, (u8*) puffles_images [puffle_outputs [thispuffle->frame]], 94, 98, 0, 100);
				//add_rect (&puf_pos);
			//}
			
			if (thispuffle != NULL) thispuffle = thispuffle->next;
		} while (thispuffle != NULL);
		
		gfxFlushBuffers();
		gfxSwapBuffers();

		//gspWaitForEvent(GSPEVENT_VBlank0, 0);
		//Wait for VBlank
		gspWaitForVBlank();
	}
}

int main (void) {
	
	setup ();
	
	game_loop ();
	
	gfxExit();
	hidExit();
	aptExit();
	srvExit();
	return 0;
}

void setup (void) {
	int g;
	/* Inicializar 3DS */
	srvInit();
	aptInit();
	hidInit(NULL);
	gfxInitDefault();
	gfxSet3D(0);
	
	consoleInit(GFX_BOTTOM, NULL);
	
	/* Primero para el puffle azul */
	for (g = 0; g < 8; g++) {
		puffles_images[g] = (u8 *) blue_bgra + 36848 * g;
	}
}

void nuevo_puffle (void) {
	Puffle *new;
	static int color = 0;
	new = (Puffle *) malloc (sizeof (Puffle));
	
	/* Inicializar el Puffle */
	new->color = color++;
	if (color >= 10) color = 0;
	new->frame = 0;
	//new->x_virtual = new->y_virtual = new->pop_num = 0;
	
	new->y = -40;
	new->x = 20 + (int) (720.0 * rand () / (RAND_MAX + 1.0));
	
	/* Ahora sus campos para lista doble ligada */
	new->next = NULL;
	new->prev = last_puffle;
	
	if (last_puffle == NULL) {
		first_puffle = last_puffle = new;
	} else {
		last_puffle->next = new;
		last_puffle = new;
	}
	
	/* Background, dame un "more" */
	//background_frame = background_frames [background_frame][BACKGROUND_NEW];
	//whole_flip = 1;
}
