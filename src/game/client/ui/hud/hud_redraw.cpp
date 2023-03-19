/***
 *
 *	Copyright (c) 1999, Valve LLC. All rights reserved.
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
//
// hud_redraw.cpp
//
#include "hud.h"

#include "vgui_TeamFortressViewport.h"
#include "vgui_StatsMenuPanel.h"

#define MAX_LOGO_FRAMES 56

int grgLogoFrame[MAX_LOGO_FRAMES] =
	{
		1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 13, 13, 13, 13, 13, 12, 11, 10, 9, 8, 14, 15,
		16, 17, 18, 19, 20, 20, 20, 20, 20, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
		29, 29, 29, 29, 29, 28, 27, 26, 25, 24, 30, 31};


extern bool g_iVisibleMouse;

float HUD_GetFOV();

extern cvar_t* sensitivity;

// Think
void CHud::Think()
{
	m_scrinfo.iSize = sizeof(m_scrinfo);
	GetScreenInfo(&m_scrinfo);

	int newfov;

	for (auto hudElement : m_HudList)
	{
		if ((hudElement->m_iFlags & HUD_ACTIVE) != 0)
		{
			hudElement->Think();
		}
	}

	newfov = HUD_GetFOV();
	if (newfov == 0)
	{
		m_iFOV = default_fov->value;
	}
	else
	{
		m_iFOV = newfov;
	}

	// the clients fov is actually set in the client data update section of the hud

	// Set a new sensitivity
	if (m_iFOV == default_fov->value)
	{
		// reset to saved sensitivity
		m_flMouseSensitivity = 0;
	}
	else
	{
		// set a new sensitivity that is proportional to the change from the FOV default
		m_flMouseSensitivity = sensitivity->value * ((float)newfov / (float)default_fov->value) * CVAR_GET_FLOAT("zoom_sensitivity_ratio");
	}

	// think about default fov
	if (m_iFOV == 0)
	{ // only let players adjust up in fov,  and only if they are not overriden by something else
		m_iFOV = V_max(default_fov->value, 90);
	}

	if (0 != gEngfuncs.IsSpectateOnly())
	{
		m_iFOV = gHUD.m_Spectator.GetFOV(); // default_fov->value;
	}
}

// Redraw
// step through the local data,  placing the appropriate graphics & text as appropriate
// returns 1 if they've changed, 0 otherwise
bool CHud::Redraw(float flTime, bool intermission)
{
	m_fOldTime = m_flTime; // save time of previous redraw
	m_flTime = flTime;
	m_flTimeDelta = (double)m_flTime - m_fOldTime;
	static float m_flShotTime = 0;

	// Clock was reset, reset delta
	if (m_flTimeDelta < 0)
		m_flTimeDelta = 0;

	// Bring up the scoreboard during intermission
	if (gViewPort)
	{
		if (m_iIntermission && !intermission)
		{
			// Have to do this here so the scoreboard goes away
			m_iIntermission = intermission;
			gViewPort->HideCommandMenu();
			gViewPort->HideScoreBoard();
			if (gViewPort->m_pStatsMenu && gViewPort->m_pStatsMenu->isVisible())
			{
				gViewPort->m_pStatsMenu->setVisible(false);
			}
			gViewPort->UpdateSpectatorPanel();
		}
		// Disabled in Opposing Force.
		/*
		else if (!m_iIntermission && intermission)
		{
			m_iIntermission = intermission;
			gViewPort->HideCommandMenu();
			gViewPort->HideVGUIMenu();
			gViewPort->ShowScoreBoard();
			gViewPort->UpdateSpectatorPanel();

			// Take a screenshot if the client's got the cvar set
			if (CVAR_GET_FLOAT("hud_takesshots") != 0)
				m_flShotTime = flTime + 1.0; // Take a screenshot in a second
		}
		*/
	}

	if (0 != m_flShotTime && m_flShotTime < flTime)
	{
		gEngfuncs.pfnClientCmd("snapshot\n");
		m_flShotTime = 0;
	}

	m_iIntermission = intermission;

	// if no redrawing is necessary
	// return 0;

	// draw all registered HUD elements
	if (0 != m_pCvarDraw->value)
	{
		for (auto hudElement : m_HudList)
		{
			if (!intermission)
			{
				if ((hudElement->m_iFlags & HUD_ACTIVE) != 0 && (m_iHideHUDDisplay & HIDEHUD_ALL) == 0)
					hudElement->Draw(flTime);
			}
			else
			{ // it's an intermission,  so only draw hud elements that are set to draw during intermissions
				if ((hudElement->m_iFlags & HUD_INTERMISSION) != 0)
					hudElement->Draw(flTime);
			}
		}
	}

	// are we in demo mode? do we need to draw the logo in the top corner?
	if (m_ShowLogo)
	{
		int x, y, i;

		if (m_hsprLogo == 0)
			m_hsprLogo = LoadSprite("sprites/%d_logo.spr");

		SPR_Set(m_hsprLogo, {250, 250, 250});

		x = SPR_Width(m_hsprLogo, 0);
		x = ScreenWidth - x;
		y = SPR_Height(m_hsprLogo, 0) / 2;

		// Draw the logo at 20 fps
		int iFrame = (int)(flTime * 20) % MAX_LOGO_FRAMES;
		i = grgLogoFrame[iFrame] - 1;

		SPR_DrawAdditive(i, x, y, nullptr);
	}

	/*
	if ( g_iVisibleMouse )
	{
		void IN_GetMousePos( int *mx, int *my );
		int mx, my;

		IN_GetMousePos( &mx, &my );

		if (m_hsprCursor == 0)
		{
			char sz[256];
			sprintf( sz, "sprites/cursor.spr" );
			m_hsprCursor = SPR_Load( sz );
		}

		SPR_Set(m_hsprCursor, 250, 250, 250 );

		// Draw the logo at 20 fps
		SPR_DrawAdditive( 0, mx, my, nullptr );
	}
	*/

	return true;
}

void ScaleColors(int& r, int& g, int& b, int a)
{
	float x = (float)a / 255;
	r = (int)(r * x);
	g = (int)(g * x);
	b = (int)(b * x);
}

int CHud::DrawHudString(int xpos, int ypos, int iMaxX, const char* szIt, const RGB24& color)
{
	return xpos + gEngfuncs.pfnDrawString(xpos, ypos, szIt, color.Red, color.Green, color.Blue);
}

int CHud::DrawHudNumberString(int xpos, int ypos, int iMinX, int iNumber, const RGB24& color)
{
	char szString[32];
	sprintf(szString, "%d", iNumber);
	return DrawHudStringReverse(xpos, ypos, iMinX, szString, color);
}

// draws a string from right to left (right-aligned)
int CHud::DrawHudStringReverse(int xpos, int ypos, int iMinX, const char* szString, const RGB24& color)
{
	/*
	return xpos - gEngfuncs.pfnDrawStringReverse( xpos, ypos, szString, r, g, b);
	*/

	// Op4 uses custom reverse drawing to fix an issue with the letter k overlapping the letter i in the string "kills"

	if ('\0' == *szString)
	{
		return xpos;
	}

	const char* i;

	for (i = szString; '\0' != *i; ++i)
	{
	}

	--i;

	int x = xpos - gHUD.m_scrinfo.charWidths[*i];

	if (iMinX > x)
	{
		return xpos;
	}

	while (true)
	{
		gEngfuncs.pfnDrawCharacter(x, ypos, *i, color.Red, color.Green, color.Blue);

		if (i == szString)
			break;

		--i;

		const int width = gHUD.m_scrinfo.charWidths[*i];

		if (x - width < iMinX)
			break;

		x -= width;
	}

	return x;
}

int CHud::DrawHudNumber(int x, int y, int iFlags, int iNumber, const RGB24& color)
{
	int iWidth = GetSpriteRect(m_HUD_number_0).right - GetSpriteRect(m_HUD_number_0).left;
	int k;

	if (iNumber > 0)
	{
		// SPR_Draw 100's
		if (iNumber >= 100)
		{
			k = iNumber / 100;
			SPR_Set(GetSprite(m_HudNumbers[k]), color);
			SPR_DrawAdditive(0, x, y, &GetSpriteRect(m_HudNumbers[k]));
			x += iWidth;
		}
		else if ((iFlags & DHN_3DIGITS) != 0)
		{
			// SPR_DrawAdditive( 0, x, y, &rc );
			x += iWidth;
		}

		// SPR_Draw 10's
		if (iNumber >= 10)
		{
			k = (iNumber % 100) / 10;
			SPR_Set(GetSprite(m_HudNumbers[k]), color);
			SPR_DrawAdditive(0, x, y, &GetSpriteRect(m_HudNumbers[k]));
			x += iWidth;
		}
		else if ((iFlags & (DHN_3DIGITS | DHN_2DIGITS)) != 0)
		{
			// SPR_DrawAdditive( 0, x, y, &rc );
			x += iWidth;
		}

		// SPR_Draw ones
		k = iNumber % 10;
		SPR_Set(GetSprite(m_HudNumbers[k]), color);
		SPR_DrawAdditive(0, x, y, &GetSpriteRect(m_HudNumbers[k]));
		x += iWidth;
	}
	else if ((iFlags & DHN_DRAWZERO) != 0)
	{
		SPR_Set(GetSprite(m_HUD_number_0), color);

		// SPR_Draw 100's
		if ((iFlags & DHN_3DIGITS) != 0)
		{
			// SPR_DrawAdditive( 0, x, y, &rc );
			x += iWidth;
		}

		if ((iFlags & (DHN_3DIGITS | DHN_2DIGITS)) != 0)
		{
			// SPR_DrawAdditive( 0, x, y, &rc );
			x += iWidth;
		}

		// SPR_Draw ones

		SPR_DrawAdditive(0, x, y, &GetSpriteRect(m_HUD_number_0));
		x += iWidth;
	}

	return x;
}


int CHud::GetNumWidth(int iNumber, int iFlags)
{
	if ((iFlags & DHN_3DIGITS) != 0)
		return 3;

	if ((iFlags & DHN_2DIGITS) != 0)
		return 2;

	if (iNumber <= 0)
	{
		if ((iFlags & DHN_DRAWZERO) != 0)
			return 1;
		else
			return 0;
	}

	if (iNumber < 10)
		return 1;

	if (iNumber < 100)
		return 2;

	return 3;
}

int CHud::GetHudNumberWidth(int number, int width, int flags)
{
	const int digitWidth = GetSpriteRect(m_HUD_number_0).right - GetSpriteRect(m_HUD_number_0).left;

	int totalDigits = 0;

	if (number > 0)
	{
		totalDigits = static_cast<int>(log10(number)) + 1;
	}
	else if ((flags & DHN_DRAWZERO) != 0)
	{
		totalDigits = 1;
	}

	totalDigits = V_max(totalDigits, width);

	return totalDigits * digitWidth;
}

int CHud::DrawHudNumberReverse(int x, int y, int number, int flags, const RGB24& color)
{
	if (number > 0 || (flags & DHN_DRAWZERO) != 0)
	{
		const int digitWidth = GetSpriteRect(m_HUD_number_0).right - GetSpriteRect(m_HUD_number_0).left;

		int remainder = number;

		do
		{
			const int digit = remainder % 10;
			const int digitSpriteIndex = m_HudNumbers[digit];

			// This has to happen *before* drawing because we're drawing in reverse
			x -= digitWidth;

			SPR_Set(GetSprite(digitSpriteIndex), color);
			SPR_DrawAdditive(0, x, y, &GetSpriteRect(digitSpriteIndex));

			remainder /= 10;
		} while (remainder > 0);
	}

	return x;
}
