/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
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

#include "cbase.h"
#include "CItemCTF.h"
#include "CItemRegenerationCTF.h"
#include "ctfplay_gamerules.h"
#include "UserMessages.h"

LINK_ENTITY_TO_CLASS(item_ctfregeneration, CItemRegenerationCTF);

void CItemRegenerationCTF::OnCreate()
{
	CItemCTF::OnCreate();

	pev->model = MAKE_STRING("models/w_health.mdl");
}


void CItemRegenerationCTF::Precache()
{
	g_engfuncs.pfnPrecacheModel(STRING(pev->model));
	g_engfuncs.pfnPrecacheSound("ctf/pow_health_charge.wav");
}

void CItemRegenerationCTF::RemoveEffect(CBasePlayer* pPlayer)
{
	pPlayer->m_flHealthTime += gpGlobals->time - m_flPickupTime;
}

bool CItemRegenerationCTF::MyTouch(CBasePlayer* pPlayer)
{
	if ((pPlayer->m_iItems & CTFItem::Regeneration) == 0)
	{
		if (0 == multipower.value)
		{
			if ((pPlayer->m_iItems & ~(CTFItem::BlackMesaFlag | CTFItem::OpposingForceFlag)) != 0)
				return false;
		}

		if (static_cast<int>(team_no) <= 0 || team_no == pPlayer->m_iTeamNum)
		{
			if (pPlayer->HasSuit())
			{
				pPlayer->m_iItems = static_cast<CTFItem::CTFItem>(pPlayer->m_iItems | CTFItem::Regeneration);
				pPlayer->m_fPlayingHChargeSound = false;

				g_engfuncs.pfnMessageBegin(MSG_ONE, gmsgItemPickup, nullptr, pPlayer->edict());
				g_engfuncs.pfnWriteString(STRING(pev->classname));
				g_engfuncs.pfnMessageEnd();

				EMIT_SOUND_DYN(edict(), CHAN_VOICE, "items/ammopickup1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

				if (pPlayer->pev->health < 100.0)
				{
					pPlayer->TakeHealth(GetSkillFloat("healthkit"sv), DMG_GENERIC);
				}

				return true;
			}
		}
	}

	return false;
}

void CItemRegenerationCTF::Spawn()
{
	g_engfuncs.pfnPrecacheSound("ctf/itemthrow.wav");
	g_engfuncs.pfnPrecacheSound("items/ammopickup1.wav");

	Precache();

	g_engfuncs.pfnSetModel(edict(), STRING(pev->model));

	pev->spawnflags |= SF_NORESPAWN;
	pev->oldorigin = pev->origin;

	CItemCTF::Spawn();

	m_iItemFlag = CTFItem::Regeneration;
	m_pszItemName = "Health";
}

int CItemRegenerationCTF::Classify()
{
	return CLASS_CTFITEM;
}
