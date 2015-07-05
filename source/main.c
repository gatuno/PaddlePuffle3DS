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

#include "fix16.h"

#include "normal_bgr.h"
#include "new_0_bgr.h"
#include "new_1_bgr.h"
#include "fail_0_bgr.h"
#include "fail_1_bgr.h"

#include "blue_bgra.h"
#include "pink_bgra.h"
#include "black_bgra.h"
#include "green_bgra.h"
#include "purple_bgra.h"
#include "red_bgra.h"
#include "yellow_bgra.h"
#include "white_bgra.h"
#include "orange_bgra.h"
#include "brown_bgra.h"

#include "paddle_bgra.h"

#include "opening_bgr.h"
#include "paddle_fondo_bgra.h"
#include "boton_ui_bgra.h"

#define FPS (268123480/12)

/* Entrada 0 significa normal, 1 nuevo, 2 perdido */
enum {
	BACKGROUND_NORMAL = 0,
	BACKGROUND_NEW,
	BACKGROUND_FAIL
};

/* Autómata para el fondo */
static int background_frames[15][3] = {
	{0, 1, 8},
	{2, 1, 8},
	{3, 1, 8},
	{4, 1, 8},
	{5, 1, 8},
	{6, 1, 8},
	{7, 1, 8},
	{0, 1, 8},
	{9, 1, 8},
	{10, 1, 8},
	{11, 1, 8},
	{12, 1, 8},
	{13, 1, 8},
	{14, 1, 8}
};

enum {
	IMG_BACKGROUND_NORMAL = 0,
	IMG_BACKGROUND_NEW_0,
	IMG_BACKGROUND_NEW_1,
	IMG_BACKGROUND_FAIL_0,
	IMG_BACKGROUND_FAIL_1
};

static int background_outputs[15] = {
	IMG_BACKGROUND_NORMAL,
	IMG_BACKGROUND_NEW_0,
	IMG_BACKGROUND_NEW_0,
	IMG_BACKGROUND_NEW_0,
	IMG_BACKGROUND_NEW_0,
	IMG_BACKGROUND_NEW_1,
	IMG_BACKGROUND_NEW_1,
	IMG_BACKGROUND_NEW_1,
	IMG_BACKGROUND_FAIL_0,
	IMG_BACKGROUND_FAIL_0,
	IMG_BACKGROUND_FAIL_0,
	IMG_BACKGROUND_FAIL_0,
	IMG_BACKGROUND_FAIL_1,
	IMG_BACKGROUND_FAIL_1,
	IMG_BACKGROUND_FAIL_1
};

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

enum {
	IMG_PADDLE_1 = 0,
	IMG_PADDLE_2,
	IMG_PADDLE_3,
	IMG_PADDLE_4
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

/* Autómata para el paddle */
enum {
	PADDLE_NORMAL = 0,
	PADDLE_BOUNCE,
	NUM_PADDLE_MODS
};

static int paddle_frames [5][NUM_PADDLE_MODS] = {
	{0, 1},
	{2, 1},
	{3, 2},
	{4, 3},
	{0, 4}
};

static int paddle_outputs [5] = {
	IMG_PADDLE_1,
	IMG_PADDLE_2,
	IMG_PADDLE_2,
	IMG_PADDLE_3,
	IMG_PADDLE_4
};

/* Codigos de salida */
enum {
	GAME_NONE = 0, /* No usado */
	GAME_CONTINUE,
	GAME_QUIT
};

/* La estructura principal de un puffle */
typedef struct _Puffle {
	struct _Puffle *next;
	struct _Puffle *prev;
	int x;
	int y;
	fix16_t x_fixed;
	fix16_t y_fixed;
	int pop_num;
	int color;
	int frame;
} Puffle;

/* Prototipos de funciones */
void setup (void);
void nuevo_puffle (void);
void eliminar_puffle (Puffle *p);

/* Variables Globales */
Puffle *first_puffle = NULL;
Puffle *last_puffle = NULL;

u8 *puffles_images[10][8];
u8 *paddle_images[4];
u8 *background_images[5];

int background_frame = 0;

void gfxDrawSprite (gfxScreen_t screen, gfx3dSide_t side, const u8* spriteData, u16 width, u16 height, s16 x, s16 y) {
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

void gfxDrawTransSprite (gfxScreen_t screen, gfx3dSide_t side, const u8* spriteData, u16 width, u16 height, s16 x, s16 y) {
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
		const u8* data = &spriteData[(xOffset) * 4]; /* La imagen es ABGR */
		for (i = xOffset; i < xOffset + widthDrawn; i++, fbd += 3, data += 4) {
			alpha = data[3];
			if (alpha == 0) continue; /* 255 = Opaco, 0 = Trans */
			fbd[0] = ((data[0] * alpha) + (fbd[0] * (255 - alpha))) / 256;
			fbd[1] = ((data[1] * alpha) + (fbd[1] * (255 - alpha))) / 256;
			fbd[2] = ((data[2] * alpha) + (fbd[2] * (255 - alpha))) / 256;
		}
		fbAdr += fbWidth * 3;
		spriteData += width * 4;
	}
}

void gfxDrawPixel (gfxScreen_t screen, gfx3dSide_t side, s16 x, s16 y, u8 r, u8 g, u8 b) {
	u16 fbWidth, fbHeight;
	u8* fbAdr = gfxGetFramebuffer (screen, side, &fbWidth, &fbHeight);

	if (x < 0 || x >= fbWidth) return;
	if (y < 0 || y >= fbHeight) return;
	
	fbAdr += y * fbWidth * 3;
	
	u8 *fbd = &fbAdr[x * 3];
	
	fbd[0] = b;
	fbd[1] = g;
	fbd[2] = r;
}

int game_intro (void) {
	int done = 0;
	
	u32 keys;
	touchPosition touch;
	
	while (aptMainLoop () && !done) {
		hidScanInput ();
		keys = hidKeysDown ();
		
		if (keys & KEY_START) {
			done = GAME_QUIT;
		}
		
		hidTouchRead(&touch);
		if (keys & KEY_TOUCH && touch.px > 83 && touch.px < 247 && touch.py > 179 && touch.py < 227) {
			done = GAME_CONTINUE;
		}
		
		/* Dibujar la parte superior */
		gfxDrawSprite (GFX_TOP, GFX_LEFT, normal_bgr, 240, 400, 0, 0);
		
		/* Dibujar la parte inferior de la pantalla */
		gfxDrawSprite (GFX_BOTTOM, GFX_LEFT, opening_bgr, 240, 320, 0, 0);
		
		/* Dibujar el botón de jugar */
		gfxDrawTransSprite (GFX_BOTTOM, GFX_LEFT, boton_ui_bgra, 48, 164, 18, 78);
		
		//Wait for VBlank
		gspWaitForVBlank();
		
		gfxFlushBuffers();
		gfxSwapBuffers();
	}
	
	return done;
}

int game_loop (void) {
	int done = 0;
	Puffle *thispuffle;
	
	int fuerzax, fuerzay; /* Calculos de fuerza al golpear el puffle */
	int g, h;
	
	fix16_t poder;
	fix16_t speed = 0xA0000; /* 10 = (10 << 16) */
	fix16_t balance = 0x40000; /* 4 = (4 << 16) */
	int wind = 1, wind_countdown = 240; /* Para evitar puffles estancados verticalmente */
	int n_puffles = 1, most_puffles = 1, dropped_puffles = 0; /* Llevar la cantidad de puffles */
	int count = 0, goal = 20, default_goal = 20; /* Para control de la generación de próximos puffles */
	int bounces = 0, role = 0; /* Bounces, golpes totales. Role, el mayor número de golpes */
	int paddle_x, paddle_y, paddle_frame = 0;
	int paddle_x2, paddle_x1, paddle_y2, paddle_y1; /* Para calcular los desplazamientos del paddle */
	int tickets = 0;
	u64 last_time, now_time;
	
	u32 keys;
	touchPosition touch, touch1;
	nuevo_puffle ();
	background_frame = 0;
	
	paddle_x = paddle_x2 = paddle_x1 = 200;
	paddle_y = paddle_y2 = paddle_y1 = 120;
	
	while (aptMainLoop () && !done) {
		last_time = svcGetSystemTick ();
		hidScanInput ();
		keys = hidKeysDown ();
		
		if (keys & KEY_START) {
			done = GAME_QUIT;
		}
		
		if (count >= goal) {
			count = 0;
			n_puffles++;
	
			if (n_puffles > 4) {
				goal = default_goal;
		
				if (default_goal > 5) {
					default_goal--;
				}
		
				if (dropped_puffles > 49) {
					default_goal += 20;
					dropped_puffles = 0;
				}
			} else if (n_puffles >= most_puffles) {
				goal = n_puffles * 20;
			} else {
				goal = n_puffles * 10;
			}
	
			if (most_puffles < n_puffles) {
				most_puffles = n_puffles;
			}
	
			nuevo_puffle ();
		}
		
		paddle_y2 = paddle_y1;
		paddle_y1 = paddle_y;
		
		paddle_x2 = paddle_x1;
		paddle_x1 = paddle_x;
		
		touch1 = touch;
		
		hidTouchRead(&touch);
		if (touch.px != 0 && touch.py != 0) {
			if (keys & KEY_TOUCH) {
				touch1 = touch;
			}
			
			paddle_x += (touch.px - touch1.px);
			paddle_y += (touch.py - touch1.py);
		}
		
		if (paddle_x < 0) paddle_x = 0;
		if (paddle_y < -30) paddle_y = -30;
		if (paddle_x > 400) paddle_x = 400;
		if (paddle_y > 220) paddle_y = 220;
		
		fuerzay = paddle_y2 - paddle_y;
		
		if (fuerzay > 0) {
			poder = fix16_div (fuerzay << 16, (6 << 16));
		} else {
			poder = 0;
		}
	
		fuerzax = paddle_x2 - paddle_x;
		//printf ("Touch: %i, %i\n", touch.px, touch.py);
		//printf ("Diferencial: %i, %i\n", (handposx1 - handposx2), (handposy1 - handposy2));
		
		thispuffle = first_puffle;
		do {
			if (thispuffle->y > 290) {
				/* Este puffle está perdido */
				n_puffles--;
				dropped_puffles++;
				
				if (n_puffles > 4) {
					goal = default_goal;
				} else if (n_puffles >= most_puffles) {
					goal = n_puffles * 20;
				} else {
					goal = n_puffles * 10;
				}
				
				if (thispuffle->prev != NULL) {
					thispuffle = thispuffle->prev;
					eliminar_puffle (thispuffle->next);
				} else {
					eliminar_puffle (thispuffle);
					thispuffle = first_puffle;
				}
			}
			if (thispuffle != NULL) thispuffle = thispuffle->next;
		} while (thispuffle != NULL);
		
		if (first_puffle == NULL) {
			done = GAME_CONTINUE;
			/*tickets = bounces + most_puffles * role;
			*ret_tickets = tickets;
			*ret_bounces = bounces;
			*ret_most = most_puffles;
			*ret_role = role;*/
			break;
		}
		
		/* Dibujar la parte inferior de la pantalla */
		gfxDrawSprite (GFX_BOTTOM, GFX_LEFT, opening_bgr, 240, 320, 0, 0);
		gfxDrawTransSprite (GFX_BOTTOM, GFX_LEFT, paddle_fondo_bgra, 172, 194, 26, 63);
		
		background_frame = background_frames [background_frame][BACKGROUND_NORMAL];
		gfxDrawSprite (GFX_TOP, GFX_LEFT, background_images[background_outputs[background_frame]], 240, 400, 0, 0);
		
		thispuffle = first_puffle;
		do {
			/* Avanzar de posición los puffles */
			g = fix16_to_int (thispuffle->x_fixed);
			h = fix16_to_int (thispuffle->y_fixed);
			thispuffle->x = thispuffle->x + g;
			thispuffle->y = thispuffle->y + h;
			
			if (thispuffle->x >= 360 && g >= 0) {
				thispuffle->x_fixed = fix16_mul (thispuffle->x_fixed, 0xFFFF0000); /* -1 */
			} else if (thispuffle->x <= 40 && g < 0) {
				thispuffle->x_fixed = fix16_mul (thispuffle->x_fixed, 0xFFFF0000); /* -1 */
			}
			
			if (h < -10) {
				thispuffle->y_fixed = fix16_add (thispuffle->y_fixed, 58983); /* 0.9 aprox */
			} else {
				thispuffle->y_fixed = fix16_add (thispuffle->y_fixed, fix16_one); /* 1 de "Gravity" */
			}
			
			g = fix16_to_int (thispuffle->x_fixed);
			if (g > 30 || g < -30) {
				thispuffle->x_fixed = fix16_mul (thispuffle->x_fixed, 62259); /* 0.95 */
			}
			
			/* Si tiene un score (alias bounces) mayor a 50, aplicar un poco de viento */
			if (bounces > 50) { /* Bounces ajustado a 25, valor original 50 */
				if (wind) thispuffle->x_fixed = fix16_add (thispuffle->x_fixed, 6554); /* 0.1 aprox */
				else thispuffle->x_fixed = fix16_sub (thispuffle->x_fixed, 6554); /* 0.1 aprox */
				
				if (wind_countdown > 0) wind_countdown--;
			}
			
			if (wind_countdown == 0) {
				if (wind) wind = 0;
				else wind = 1;
				wind_countdown = 240;
			}
			
			h = fix16_to_int (thispuffle->y_fixed);
			if (thispuffle->y > -99 && h >= 0) {
				if ((thispuffle->x > paddle_x - 39 && thispuffle->x < paddle_x + 39) && ((thispuffle->y + 30 > paddle_y && thispuffle->y + 30 < paddle_y + 55) || (thispuffle->y > paddle_y && thispuffle->y < paddle_y2))) {
					/* Bounce the puffle */
					/* sonido = SND_SQUEAK1 + (int) (2.0 * rand () / (RAND_MAX + 1.0));
					
					if (fuerzax > 300 || fuerzax < -300 || poder > 30) {
						sonido = SND_SQUEAK3;
					}*/
					g = thispuffle->x - (paddle_x + fuerzax);
					thispuffle->x_fixed = fix16_div (g << 16, balance);
					thispuffle->y_fixed = fix16_mul (4294914867u, fix16_add (speed, poder)); /* -0.8 */
					
					bounces++; count++;
					
					if (fix16_to_int (speed) < 40) speed = fix16_add (speed, 13107); /* 0.2 */
					else if (fix16_to_int (balance) > 2) balance = fix16_sub (balance, 13107);
					
					/*pop_num = ++thispuffle->pop_num;
					pop_timer = 0;
					pop_x = thispuffle->x;
					pop_y = thispuffle->y;
					
					if (thispuffle->pop_num > role) role = thispuffle->pop_num;
					*/
					thispuffle->frame = puffle_frames [thispuffle->frame][PUFFLE_BOUNCE];
					paddle_frame = paddle_frames [paddle_frame][PADDLE_BOUNCE];
					
					//if (use_sound) Mix_PlayChannel (-1, sounds[sonido], 0);
					
					tickets = bounces + most_puffles * role;
				} else if ((thispuffle->y + 30 > paddle_y && thispuffle->y + 30 < paddle_y + 55) && ((thispuffle->x > paddle_x && thispuffle->x < paddle_x2) || (thispuffle->x < paddle_x && thispuffle->x > paddle_x2))) {
					/* Bounce the puffle */
					/*sonido = SND_SQUEAK1 + (int) (2.0 * rand () / (RAND_MAX + 1.0));
					
					if (fuerzax > 300 || fuerzax < -300 || poder > 30) {
						sonido = SND_SQUEAK3;
					}*/
					
					g = thispuffle->x - (paddle_x + fuerzax);
					thispuffle->x_fixed = fix16_div (g << 16, balance);
					thispuffle->y_fixed = fix16_mul (4294914867u, fix16_add (speed, poder)); /* -0.8 */
					
					bounces++; count++;
					
					if (fix16_to_int (speed) < 40) speed = fix16_add (speed, 13107); /* 0.2 */
					else if (fix16_to_int (balance) > 2) balance = fix16_sub (balance, 13107);
					
					/*pop_num = ++thispuffle->pop_num;
					pop_timer = 0;
					pop_x = thispuffle->x;
					pop_y = thispuffle->y;
					
					if (thispuffle->pop_num > role) role = thispuffle->pop_num;*/
					
					thispuffle->frame = puffle_frames [thispuffle->frame][PUFFLE_BOUNCE];
					paddle_frame = paddle_frames [paddle_frame][PADDLE_BOUNCE];
					
					//if (use_sound) Mix_PlayChannel (-1, sounds[sonido], 0);
					
					tickets = bounces + most_puffles * role;
				}
			}
			
			h = fix16_to_int (thispuffle->y_fixed);
			if (h > 10) {
				thispuffle->frame = puffle_frames [thispuffle->frame][PUFFLE_FALL];
			}
			
			thispuffle->frame = puffle_frames [thispuffle->frame][PUFFLE_NORMAL];
			if (thispuffle != NULL) thispuffle = thispuffle->next;
		} while (thispuffle != NULL);
		
		/* TODO: Dibujar aquí la cantidad de Tickets */
		paddle_frame = paddle_frames[paddle_frame][PADDLE_NORMAL];
		
		gfxDrawTransSprite (GFX_TOP, GFX_LEFT, (u8 *) paddle_images[paddle_outputs [paddle_frame]], 141, 64, 240 - (paddle_y - 39) - 141, paddle_x - 32);
		
		thispuffle = first_puffle;
		do {
			if (thispuffle->y > -100) {
				/* Blit this puffle */
				gfxDrawTransSprite (GFX_TOP, GFX_LEFT, (u8*) puffles_images [thispuffle->color][puffle_outputs [thispuffle->frame]], 94, 98, 240 - (thispuffle->y - 60) - 94, thispuffle->x - 48);
			}
			
			if (thispuffle != NULL) thispuffle = thispuffle->next;
		} while (thispuffle != NULL);
		
		//Wait for VBlank
		gspWaitForVBlank();
		
		gfxFlushBuffers();
		gfxSwapBuffers();
		
		now_time = svcGetSystemTick ();
		if (now_time < last_time + FPS) svcSleepThread (last_time + FPS - now_time);
	}
	
	return done;
}

int main (void) {
	
	setup ();
	
	do {
		if (game_intro () == GAME_QUIT) break;
		if (game_loop () == GAME_QUIT) break;
	} while (1 == 0);
	
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
	gfxSetDoubleBuffering (GFX_BOTTOM, 1);
	
	//consoleInit(GFX_BOTTOM, NULL);
	
	/* Primero para el puffle azul */
	for (g = 0; g < 8; g++) {
		puffles_images[0][g] = (u8 *) blue_bgra + 36848 * g;
	}
	
	for (g = 0; g < 8; g++) {
		puffles_images[1][g] = (u8 *) pink_bgra + 36848 * g;
	}
	
	for (g = 0; g < 8; g++) {
		puffles_images[2][g] = (u8 *) black_bgra + 36848 * g;
	}
	
	for (g = 0; g < 8; g++) {
		puffles_images[3][g] = (u8 *) green_bgra + 36848 * g;
	}
	
	for (g = 0; g < 8; g++) {
		puffles_images[4][g] = (u8 *) purple_bgra + 36848 * g;
	}
	
	for (g = 0; g < 8; g++) {
		puffles_images[5][g] = (u8 *) red_bgra + 36848 * g;
	}
	
	for (g = 0; g < 8; g++) {
		puffles_images[6][g] = (u8 *) yellow_bgra + 36848 * g;
	}
	
	for (g = 0; g < 8; g++) {
		puffles_images[7][g] = (u8 *) white_bgra + 36848 * g;
	}
	
	for (g = 0; g < 8; g++) {
		puffles_images[8][g] = (u8 *) orange_bgra + 36848 * g;
	}
	
	for (g = 0; g < 8; g++) {
		puffles_images[9][g] = (u8 *) brown_bgra + 36848 * g;
	}
	
	/* Para el paddle */
	for (g = 0; g < 4; g++) {
		paddle_images[g] = (u8 *) paddle_bgra + 36096 * g;
	}
	
	background_images[0] = (u8 *) normal_bgr;
	background_images[1] = (u8 *) new_0_bgr;
	background_images[2] = (u8 *) new_1_bgr;
	background_images[3] = (u8 *) fail_0_bgr;
	background_images[4] = (u8 *) fail_1_bgr;
}

void nuevo_puffle (void) {
	Puffle *new;
	static int color = 0;
	new = (Puffle *) malloc (sizeof (Puffle));
	
	/* Inicializar el Puffle */
	new->color = color++;
	if (color >= 10) color = 0;
	new->frame = 0;
	new->x_fixed = new->y_fixed = new->pop_num = 0;
	
	new->y = -40;
	new->x = 20 + (int) (360.0 * rand () / (RAND_MAX + 1.0));
	
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
	background_frame = background_frames [background_frame][BACKGROUND_NEW];
	//whole_flip = 1;
}

void eliminar_puffle (Puffle *p) {
	if (p == NULL) return;
	
	if (p->prev == NULL) { /* El primero de la lista */
		first_puffle = p->next;
	} else {
		p->prev->next = p->next;
	}
	
	if (p->next == NULL) {
		last_puffle = p->prev;
	} else {
		p->next->prev = p->prev;
	}
	
	free (p);
	
	/* Background, dame un "miss" */
	background_frame = background_frames [background_frame][BACKGROUND_FAIL];
	//whole_flip = 1;
}
