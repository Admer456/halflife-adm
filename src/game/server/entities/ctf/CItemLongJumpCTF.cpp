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
#include "CItemLongJumpCTF.h"
#include "ctfplay_gamerules.h"
#include "UserMessages.h"

LINK_ENTITY_TO_CLASS(item_ctflongjump, CItemLongJumpCTF);

void CItemLongJumpCTF::OnCreate()
{
	CItemCTF::OnCreate();

	pev->model = MAKE_STRING("models/w_jumppack.mdl");
}

void CItemLongJumpCTF::Precache()
{
	g_engfuncs.pfnPrecacheModel(STRING(pev->model));
	g_engfuncs.pfnPrecacheSound("ctf/pow_big_jump.wav");
}

void CItemLongJumpCTF::RemoveEffect(CBasePlayer* pPlayer)
{
	pPlayer->m_fLongJump = false;
	g_engfuncs.pfnSetPhysicsKeyValue(pPlayer->edict(), "jpj", "0");
	pPlayer->m_flJumpTime += gpGlobals->time - m_flPickupTime;
}

bool CItemLongJumpCTF::MyTouch(CBasePlayer* pPlayer)
{
	if ((pPlayer->m_iItems & CTFItem::LongJump) == 0)
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
				pPlayer->m_fLongJump = true;
				g_engfuncs.pfnSetPhysicsKeyValue(pPlayer->edict(), "jpj", "1");

				pPlayer->m_iItems = static_cast<CTFItem::CTFItem>(pPlayer->m_iItems | CTFItem::LongJump);

				g_engfuncs.pfnMessageBegin(MSG_ONE, gmsgItemPickup, nullptr, pPlayer->edict());
				g_engfuncs.pfnWriteString(STRING(pev->classname));
				g_engfuncs.pfnMessageEnd();

				EMIT_SOUND_DYN(edict(), CHAN_VOICE, "items/ammopickup1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

				return true;
			}
		}
	}

	return false;
}

void CItemLongJumpCTF::Spawn()
{
	g_engfuncs.pfnPrecacheSound("ctf/itemthrow.wav");
	g_engfuncs.pfnPrecacheSound("items/ammopickup1.wav");

	Precache();

	g_engfuncs.pfnSetModel(edict(), STRING(pev->model));

	pev->spawnflags |= SF_NORESPAWN;
	pev->oldorigin = pev->origin;
	CItemCTF::Spawn();

	m_iItemFlag = CTFItem::LongJump;
	m_pszItemName = "Jump";
}

int CItemLongJumpCTF::Classify()
{
	return CLASS_CTFITEM;
}
