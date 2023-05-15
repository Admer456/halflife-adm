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

#include "CSpore.h"

BEGIN_DATAMAP(CSpore)
DEFINE_FIELD(m_SporeType, FIELD_INTEGER),
	DEFINE_FIELD(m_flIgniteTime, FIELD_TIME),
	DEFINE_FIELD(m_bIsAI, FIELD_BOOLEAN),
	DEFINE_FIELD(m_hSprite, FIELD_EHANDLE),
	DEFINE_FUNCTION(IgniteThink),
	DEFINE_FUNCTION(FlyThink),
	DEFINE_FUNCTION(GibThink),
	DEFINE_FUNCTION(RocketTouch),
	DEFINE_FUNCTION(MyBounceTouch),
	END_DATAMAP();

LINK_ENTITY_TO_CLASS(spore, CSpore);

void CSpore::Precache()
{
	PrecacheModel("models/spore.mdl");
	PrecacheModel("sprites/glow01.spr");

	m_iBlow = PrecacheModel("sprites/spore_exp_01.spr");
	m_iBlowSmall = PrecacheModel("sprites/spore_exp_c_01.spr");
	m_iSpitSprite = m_iTrail = PrecacheModel("sprites/tinyspit.spr");

	PrecacheSound("weapons/splauncher_impact.wav");
	PrecacheSound("weapons/splauncher_bounce.wav");
}

void CSpore::Spawn()
{
	Precache();

	if (m_SporeType == SporeType::GRENADE)
		pev->movetype = MOVETYPE_BOUNCE;
	else
		pev->movetype = MOVETYPE_FLY;

	pev->solid = SOLID_BBOX;

	SetModel("models/spore.mdl");

	SetSize(g_vecZero, g_vecZero);

	SetOrigin(pev->origin);

	SetThink(&CSpore::FlyThink);

	if (m_SporeType == SporeType::GRENADE)
	{
		SetTouch(&CSpore::MyBounceTouch);

		if (!m_bPuked)
		{
			pev->angles.x -= RANDOM_LONG(-5, 5) + 30;
		}
	}
	else
	{
		SetTouch(&CSpore::RocketTouch);
	}

	UTIL_MakeVectors(pev->angles);

	if (!m_bIsAI)
	{
		if (m_SporeType != SporeType::GRENADE)
		{
			pev->velocity = gpGlobals->v_forward * 1200;
		}

		pev->gravity = 1;
	}
	else
	{
		pev->gravity = 0.5;
		pev->friction = 0.7;
	}

	pev->dmg = GetSkillFloat("plr_spore"sv);

	m_flIgniteTime = gpGlobals->time;

	pev->nextthink = gpGlobals->time + 0.01;

	auto sprite = CSprite::SpriteCreate("sprites/glow01.spr", pev->origin, false);

	m_hSprite = sprite;

	sprite->SetTransparency(kRenderTransAdd, 180, 180, 40, 100, kRenderFxDistort);
	sprite->SetScale(0.8);
	sprite->SetAttachment(edict(), 0);

	m_fRegisteredSound = false;

	m_flSoundDelay = gpGlobals->time;
}

void CSpore::BounceSound()
{
	// Nothing
}

void CSpore::IgniteThink()
{
	SetThink(nullptr);
	SetTouch(nullptr);

	if (m_hSprite)
	{
		UTIL_Remove(m_hSprite);
		m_hSprite = nullptr;
	}

	EmitSound(CHAN_WEAPON, "weapons/splauncher_impact.wav", VOL_NORM, ATTN_NORM);

	const auto vecDir = pev->velocity.Normalize();

	TraceResult tr;

	UTIL_TraceLine(
		pev->origin, pev->origin + vecDir * (m_SporeType == SporeType::GRENADE ? 64 : 32),
		dont_ignore_monsters, edict(), &tr);

	UTIL_DecalTrace(&tr, DECAL_SPR_SPLT1 + RANDOM_LONG(0, 2));

	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
	WRITE_BYTE(TE_SPRITE_SPRAY);
	WRITE_COORD_VECTOR(pev->origin);
	WRITE_COORD_VECTOR(tr.vecPlaneNormal);
	WRITE_SHORT(m_iSpitSprite);
	WRITE_BYTE(100);
	WRITE_BYTE(40);
	WRITE_BYTE(180);
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
	WRITE_BYTE(TE_DLIGHT);
	WRITE_COORD_VECTOR(pev->origin);
	WRITE_BYTE(10);
	WRITE_BYTE(15);
	WRITE_BYTE(220);
	WRITE_BYTE(40);
	WRITE_BYTE(5);
	WRITE_BYTE(10);
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
	WRITE_BYTE(TE_SPRITE);
	WRITE_COORD_VECTOR(pev->origin);
	WRITE_SHORT(RANDOM_LONG(0, 1) ? m_iBlow : m_iBlowSmall);
	WRITE_BYTE(20);
	WRITE_BYTE(128);
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
	WRITE_BYTE(TE_SPRITE_SPRAY);
	WRITE_COORD_VECTOR(pev->origin);
	WRITE_COORD(RANDOM_FLOAT(-1, 1));
	WRITE_COORD(1);
	WRITE_COORD(RANDOM_FLOAT(-1, 1));
	WRITE_SHORT(m_iTrail);
	WRITE_BYTE(2);
	WRITE_BYTE(20);
	WRITE_BYTE(80);
	MESSAGE_END();

	::RadiusDamage(pev->origin, this, GetOwner(), pev->dmg, 200, DMG_ALWAYSGIB | DMG_BLAST);

	SetThink(&CSpore::SUB_Remove);

	pev->nextthink = gpGlobals->time;
}

void CSpore::FlyThink()
{
	const float flDelay = m_bIsAI ? 4.0 : 2.0;

	if (m_SporeType != SporeType::GRENADE || (gpGlobals->time <= m_flIgniteTime + flDelay))
	{
		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_SPRITE_SPRAY);
		WRITE_COORD_VECTOR(pev->origin);
		WRITE_COORD_VECTOR(pev->velocity.Normalize());
		WRITE_SHORT(m_iTrail);
		WRITE_BYTE(2);
		WRITE_BYTE(20);
		WRITE_BYTE(80);
		MESSAGE_END();
	}
	else
	{
		SetThink(&CSpore::IgniteThink);
	}

	pev->nextthink = gpGlobals->time + 0.03;
}

void CSpore::GibThink()
{
	// Nothing
}

void CSpore::RocketTouch(CBaseEntity* pOther)
{
	if (pOther->pev->takedamage != DAMAGE_NO)
	{
		pOther->TakeDamage(this, GetOwner(), GetSkillFloat("plr_spore"sv), DMG_GENERIC);
	}

	IgniteThink();
}

void CSpore::MyBounceTouch(CBaseEntity* pOther)
{
	if (pOther->pev->takedamage == DAMAGE_NO)
	{
		if (pOther->edict() != pev->owner)
		{
			if (gpGlobals->time > m_flSoundDelay)
			{
				CSoundEnt::InsertSound(bits_SOUND_DANGER, pev->origin, static_cast<int>(pev->dmg / 0.4), 0.3);

				m_flSoundDelay = gpGlobals->time + 1.0;
			}

			if ((pev->flags & FL_ONGROUND) != 0)
			{
				pev->velocity = pev->velocity * 0.5;
			}
			else
			{
				EmitSound(CHAN_VOICE, "weapons/splauncher_bounce.wav", 0.25, ATTN_NORM);
			}
		}
	}
	else
	{
		pOther->TakeDamage(this, GetOwner(), GetSkillFloat("plr_spore"sv), DMG_GENERIC);

		IgniteThink();
	}
}

CSpore* CSpore::CreateSpore(
	const Vector& vecOrigin, const Vector& vecAngles, CBaseEntity* pOwner,
	SporeType sporeType, bool bIsAI, bool bPuked)
{
	auto pSpore = g_EntityDictionary->Create<CSpore>("spore");

	pSpore->SetOrigin(vecOrigin);

	pSpore->m_SporeType = sporeType;

	if (bIsAI)
	{
		pSpore->pev->velocity = vecAngles;

		pSpore->pev->angles = UTIL_VecToAngles(vecAngles);
	}
	else
	{
		pSpore->pev->angles = vecAngles;
	}

	pSpore->m_bIsAI = bIsAI;

	pSpore->m_bPuked = bPuked;

	pSpore->Spawn();

	pSpore->pev->owner = pOwner->edict();

	return pSpore;
}
