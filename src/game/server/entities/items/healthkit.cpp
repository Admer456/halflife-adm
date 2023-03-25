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
#include "items.h"
#include "UserMessages.h"

class CHealthKit : public CItem
{
public:
	static constexpr float RefillHealthAmount = -1;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

	void OnCreate() override
	{
		CItem::OnCreate();
		m_HealthAmount = GetSkillFloat("healthkit"sv);
		pev->model = MAKE_STRING("models/w_medkit.mdl");
	}

	void Precache() override
	{
		CItem::Precache();
		PrecacheSound("items/smallmedkit1.wav");
	}

	bool KeyValue(KeyValueData* pkvd) override
	{
		if (FStrEq(pkvd->szKeyName, "health_amount"))
		{
			m_HealthAmount = std::max(RefillHealthAmount, static_cast<float>(atof(pkvd->szValue)));
			return true;
		}

		return CItem::KeyValue(pkvd);
	}

	bool AddItem(CBasePlayer* player) override
	{
		if (player->pev->deadflag != DEAD_NO)
		{
			return false;
		}

		float amount = m_HealthAmount;

		if (amount == RefillHealthAmount)
		{
			amount = player->pev->max_health;
		}

		if (player->TakeHealth(amount, DMG_GENERIC))
		{
			MESSAGE_BEGIN(MSG_ONE, gmsgItemPickup, nullptr, player->pev);
			WRITE_STRING(STRING(pev->classname));
			MESSAGE_END();

			if (m_PlayPickupSound)
			{
				player->EmitSound(CHAN_ITEM, "items/smallmedkit1.wav", 1, ATTN_NORM);
			}

			return true;
		}

		return false;
	}

protected:
	float m_HealthAmount = 0;
};

LINK_ENTITY_TO_CLASS(item_healthkit, CHealthKit);

TYPEDESCRIPTION CHealthKit::m_SaveData[] =
	{
		DEFINE_FIELD(CHealthKit, m_HealthAmount, FIELD_FLOAT)};

IMPLEMENT_SAVERESTORE(CHealthKit, CItem);

/**
*	@brief Wall mounted health kit
*/
class CWallHealth : public CBaseToggle
{
public:
	void Spawn() override;
	void Precache() override;
	void EXPORT Off();
	void EXPORT Recharge();
	bool KeyValue(KeyValueData* pkvd) override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	int ObjectCaps() override { return (CBaseToggle::ObjectCaps() | FCAP_CONTINUOUS_USE) & ~FCAP_ACROSS_TRANSITION; }
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

	float m_flNextCharge;
	int m_iReactivate; // DeathMatch Delay until reactvated
	int m_iJuice;
	int m_iOn; // 0 = off, 1 = startup, 2 = going
	float m_flSoundTime;
};

TYPEDESCRIPTION CWallHealth::m_SaveData[] =
	{
		DEFINE_FIELD(CWallHealth, m_flNextCharge, FIELD_TIME),
		DEFINE_FIELD(CWallHealth, m_iReactivate, FIELD_INTEGER),
		DEFINE_FIELD(CWallHealth, m_iJuice, FIELD_INTEGER),
		DEFINE_FIELD(CWallHealth, m_iOn, FIELD_INTEGER),
		DEFINE_FIELD(CWallHealth, m_flSoundTime, FIELD_TIME),
};

IMPLEMENT_SAVERESTORE(CWallHealth, CBaseToggle);

LINK_ENTITY_TO_CLASS(func_healthcharger, CWallHealth);

bool CWallHealth::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "style") ||
		FStrEq(pkvd->szKeyName, "height") ||
		FStrEq(pkvd->szKeyName, "value1") ||
		FStrEq(pkvd->szKeyName, "value2") ||
		FStrEq(pkvd->szKeyName, "value3"))
	{
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "dmdelay"))
	{
		m_iReactivate = atoi(pkvd->szValue);
		return true;
	}

	return CBaseToggle::KeyValue(pkvd);
}

void CWallHealth::Spawn()
{
	Precache();

	pev->solid = SOLID_BSP;
	pev->movetype = MOVETYPE_PUSH;

	UTIL_SetOrigin(pev, pev->origin); // set size and link into world
	SetSize(pev->mins, pev->maxs);
	SetModel(STRING(pev->model));
	m_iJuice = GetSkillFloat("healthcharger"sv);
	pev->frame = 0;
}

void CWallHealth::Precache()
{
	PrecacheSound("items/medshot4.wav");
	PrecacheSound("items/medshotno1.wav");
	PrecacheSound("items/medcharge4.wav");
}

void CWallHealth::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	// Make sure that we have a caller
	if (!pActivator)
		return;
	// if it's not a player, ignore
	if (!pActivator->IsPlayer())
		return;

	auto player = static_cast<CBasePlayer*>(pActivator);

	// if there is no juice left, turn it off
	if (m_iJuice <= 0)
	{
		pev->frame = 1;
		Off();
	}

	// if the player doesn't have the suit, or there is no juice left, make the deny noise
	if ((m_iJuice <= 0) || !player->HasSuit())
	{
		if (m_flSoundTime <= gpGlobals->time)
		{
			m_flSoundTime = gpGlobals->time + 0.62;
			EmitSound(CHAN_ITEM, "items/medshotno1.wav", 1.0, ATTN_NORM);
		}
		return;
	}

	pev->nextthink = pev->ltime + 0.25;
	SetThink(&CWallHealth::Off);

	// Time to recharge yet?

	if (m_flNextCharge >= gpGlobals->time)
		return;

	// Play the on sound or the looping charging sound
	if (0 == m_iOn)
	{
		m_iOn++;
		EmitSound(CHAN_ITEM, "items/medshot4.wav", 1.0, ATTN_NORM);
		m_flSoundTime = 0.56 + gpGlobals->time;
	}
	if ((m_iOn == 1) && (m_flSoundTime <= gpGlobals->time))
	{
		m_iOn++;
		EmitSound(CHAN_STATIC, "items/medcharge4.wav", 1.0, ATTN_NORM);
	}


	// charge the player
	if (player->TakeHealth(1, DMG_GENERIC))
	{
		m_iJuice--;
	}

	// govern the rate of charge
	m_flNextCharge = gpGlobals->time + 0.1;
}

void CWallHealth::Recharge()
{
	EmitSound(CHAN_ITEM, "items/medshot4.wav", 1.0, ATTN_NORM);
	m_iJuice = GetSkillFloat("healthcharger"sv);
	pev->frame = 0;
	SetThink(&CWallHealth::SUB_DoNothing);
}

void CWallHealth::Off()
{
	// Stop looping sound.
	if (m_iOn > 1)
		StopSound(CHAN_STATIC, "items/medcharge4.wav");

	m_iOn = 0;

	if ((0 == m_iJuice) && ((m_iReactivate = g_pGameRules->FlHealthChargerRechargeTime()) > 0))
	{
		pev->nextthink = pev->ltime + m_iReactivate;
		SetThink(&CWallHealth::Recharge);
	}
	else
		SetThink(&CWallHealth::SUB_DoNothing);
}
