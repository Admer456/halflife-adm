/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#pragma once

// Font stuff

#define NUM_GLYPHS 256
// does not exist: // #include "basetypes.h"

typedef struct
{
	short startoffset;
	short charwidth;
} charinfo;

typedef struct qfont_s
{
	int width, height;
	int rowcount;
	int rowheight;
	charinfo fontinfo[NUM_GLYPHS];
	unsigned char data[4];
} qfont_t;
