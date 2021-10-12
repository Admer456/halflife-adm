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
//
// flashlight.cpp
//
// implementation of CHudFlashlight class
//

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include <string.h>
#include <stdio.h>


DECLARE_MESSAGE(m_Flash, FlashBat)
DECLARE_MESSAGE(m_Flash, Flashlight)

#define BAT_NAME "sprites/%d_Flashlight.spr"

int CHudFlashlight::Init()
{
	m_fFade = 0;
	m_fOn = 0;

	HOOK_MESSAGE(Flashlight);
	HOOK_MESSAGE(FlashBat);

	m_iFlags |= HUD_ACTIVE;

	gHUD.AddHudElem(this);

	return 1;
};

void CHudFlashlight::Reset()
{
	m_fFade = 0;
	m_fOn = 0;
}

int CHudFlashlight::VidInit()
{
	int HUD_flash_empty = gHUD.GetSpriteIndex("flash_empty");
	int HUD_flash_full = gHUD.GetSpriteIndex("flash_full");
	int HUD_flash_beam = gHUD.GetSpriteIndex("flash_beam");

	m_nvSprite = LoadSprite("sprites/of_nv_b.spr");

	m_hSprite1 = gHUD.GetSprite(HUD_flash_empty);
	m_hSprite2 = gHUD.GetSprite(HUD_flash_full);
	m_hBeam = gHUD.GetSprite(HUD_flash_beam);
	m_prc1 = &gHUD.GetSpriteRect(HUD_flash_empty);
	m_prc2 = &gHUD.GetSpriteRect(HUD_flash_full);
	m_prcBeam = &gHUD.GetSpriteRect(HUD_flash_beam);
	m_iWidth = m_prc2->right - m_prc2->left;

	return 1;
};

int CHudFlashlight::MsgFunc_FlashBat(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);
	int x = READ_BYTE();
	m_iBat = x;
	m_flBat = ((float)x) / 100.0;

	return 1;
}

int CHudFlashlight::MsgFunc_Flashlight(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);
	m_SuitLightType = static_cast<SuitLightType>(READ_BYTE());
	m_fOn = READ_BYTE();
	int x = READ_BYTE();
	m_iBat = x;
	m_flBat = ((float)x) / 100.0;

	//Always update this, so that changing to flashlight type disables NVG effects
	gHUD.SetNightVisionState(m_SuitLightType == SuitLightType::Nightvision && m_fOn);

	return 1;
}

int CHudFlashlight::Draw(float flTime)
{
	if (gHUD.m_iHideHUDDisplay & (HIDEHUD_FLASHLIGHT | HIDEHUD_ALL))
		return 1;

	int x, y;
	wrect_t rc;

	if (!(gHUD.m_iWeaponBits & (1 << (WEAPON_SUIT))))
		return 1;

	const int a = m_fOn ? 225 : MIN_ALPHA;

	const auto& originalColor = m_flBat < 0.20 ? RGB_REDISH : gHUD.m_HudItemColor;

	const auto color = originalColor.Scale(a);

	y = (m_prc1->bottom - m_prc2->top) / 2;
	x = ScreenWidth - m_iWidth - m_iWidth / 2;

	// Draw the flashlight casing
	SPR_Set(m_hSprite1, color);
	SPR_DrawAdditive(0, x, y, m_prc1);

	if (m_fOn)
	{  // draw the flashlight beam
		x = ScreenWidth - m_iWidth / 2;

		SPR_Set(m_hBeam, color);
		SPR_DrawAdditive(0, x, y, m_prcBeam);

		if (m_SuitLightType == SuitLightType::Nightvision)
		{
			DrawNightVision();
		}
	}

	// draw the flashlight energy level
	x = ScreenWidth - m_iWidth - m_iWidth / 2;
	int iOffset = m_iWidth * (1.0 - m_flBat);
	if (iOffset < m_iWidth)
	{
		rc = *m_prc2;
		rc.left += iOffset;

		SPR_Set(m_hSprite2, color);
		SPR_DrawAdditive(0, x + iOffset, y, &rc);
	}


	return 1;
}

void CHudFlashlight::DrawNightVision()
{
	static int lastFrame = 0;

	auto frameIndex = rand() % gEngfuncs.pfnSPR_Frames(m_nvSprite);

	if (frameIndex == lastFrame)
		frameIndex = (frameIndex + 1) % gEngfuncs.pfnSPR_Frames(m_nvSprite);

	lastFrame = frameIndex;

	if (m_nvSprite)
	{
		const auto width = gEngfuncs.pfnSPR_Width(m_nvSprite, 0);
		const auto height = gEngfuncs.pfnSPR_Height(m_nvSprite, 0);

		gEngfuncs.pfnSPR_Set(m_nvSprite, 0, 170, 0);

		wrect_t drawingRect;

		for (auto x = 0; x < gHUD.m_scrinfo.iWidth; x += width)
		{
			drawingRect.left = 0;
			drawingRect.right = x + width >= gHUD.m_scrinfo.iWidth ? gHUD.m_scrinfo.iWidth - x : width;

			for (auto y = 0; y < gHUD.m_scrinfo.iHeight; y += height)
			{
				drawingRect.top = 0;
				drawingRect.bottom = y + height >= gHUD.m_scrinfo.iHeight ? gHUD.m_scrinfo.iHeight - y : height;

				gEngfuncs.pfnSPR_DrawAdditive(frameIndex, x, y, &drawingRect);
			}
		}
	}
}
