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

/**
*	@brief Base class for charger entities.
*/
class CBaseCharger : public CBaseToggle
{
public:
	enum class State
	{
		Off,
		Starting,
		Going
	};

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

	int ObjectCaps() override { return (CBaseToggle::ObjectCaps() | FCAP_CONTINUOUS_USE) & ~FCAP_ACROSS_TRANSITION; }

	bool KeyValue(KeyValueData* pkvd) override;
	void Precache() override;
	void Spawn() override;
	
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

	void EXPORT Off();
	void EXPORT Recharge();

protected:
	virtual bool TryCharge(CBasePlayer* player) = 0;

protected:
	State m_State = State::Off;
	int m_Juice = 0;
	float m_NextCharge = 0;
	float m_SoundTime = 0;

	// Initialized by derived class.
	int m_TotalJuice = 0;
	int m_RechargeDelay = 0; // How long until charger recharges its juice.
	float m_SoundVolume = VOL_NORM;
	const char* m_StartupSound = nullptr;
	const char* m_DenySound = nullptr;
	const char* m_DispensingSound = nullptr;
};

TYPEDESCRIPTION CBaseCharger::m_SaveData[] =
	{
		DEFINE_FIELD(CBaseCharger, m_State, FIELD_INTEGER),
		DEFINE_FIELD(CBaseCharger, m_Juice, FIELD_INTEGER),
		DEFINE_FIELD(CBaseCharger, m_NextCharge, FIELD_TIME),
		DEFINE_FIELD(CBaseCharger, m_SoundTime, FIELD_TIME),
		DEFINE_FIELD(CBaseCharger, m_TotalJuice, FIELD_INTEGER),
		DEFINE_FIELD(CBaseCharger, m_RechargeDelay, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CBaseCharger, CBaseToggle);

bool CBaseCharger::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "recharge_delay"))
	{
		m_RechargeDelay = std::max(ChargerRechargeDelayNever, atoi(pkvd->szValue));
		return true;
	}

	return CBaseToggle::KeyValue(pkvd);
}

void CBaseCharger::Precache()
{
	assert(m_StartupSound);
	assert(m_DenySound);
	assert(m_DispensingSound);

	PrecacheSound(m_StartupSound);
	PrecacheSound(m_DenySound);
	PrecacheSound(m_DispensingSound);
}

void CBaseCharger::Spawn()
{
	Precache();

	pev->solid = SOLID_BSP;
	pev->movetype = MOVETYPE_PUSH;

	UTIL_SetOrigin(pev, pev->origin); // set size and link into world
	SetSize(pev->mins, pev->maxs);
	SetModel(STRING(pev->model));
	m_Juice = m_TotalJuice;
	pev->frame = 0;
}

void CBaseCharger::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	// if it's not a player, ignore
	auto player = ToBasePlayer(pActivator);

	if (!player)
	{
		return;
	}

	// if there is no juice left, turn it off
	if (m_Juice <= 0)
	{
		pev->frame = 1;
		Off();
	}

	// if the player doesn't have the suit, or there is no juice left, make the deny noise
	if ((m_Juice <= 0) || !player->HasSuit())
	{
		if (m_SoundTime <= gpGlobals->time)
		{
			m_SoundTime = gpGlobals->time + 0.62;
			EmitSound(CHAN_ITEM, m_DenySound, m_SoundVolume, ATTN_NORM);
		}
		return;
	}

	pev->nextthink = pev->ltime + 0.25;
	SetThink(&CBaseCharger::Off);

	// Time to recharge yet?
	if (m_NextCharge >= gpGlobals->time)
		return;

	// Play the on sound or the looping charging sound
	if (State::Off == m_State)
	{
		m_State = State::Starting;
		EmitSound(CHAN_ITEM, m_StartupSound, m_SoundVolume, ATTN_NORM);
		m_SoundTime = 0.56 + gpGlobals->time;
	}

	if ((m_State == State::Starting) && (m_SoundTime <= gpGlobals->time))
	{
		m_State = State::Going;
		EmitSound(CHAN_STATIC, m_DispensingSound, m_SoundVolume, ATTN_NORM);
	}

	// charge the player
	if (TryCharge(player))
	{
		--m_Juice;
	}

	// govern the rate of charge
	m_NextCharge = gpGlobals->time + 0.1;
}

void CBaseCharger::Off()
{
	// Stop looping sound.
	if (m_State > State::Starting)
		StopSound(CHAN_STATIC, m_DispensingSound);

	m_State = State::Off;

	if (0 == m_Juice && m_RechargeDelay != ChargerRechargeDelayNever)
	{
		pev->nextthink = pev->ltime + m_RechargeDelay;
		SetThink(&CBaseCharger::Recharge);
	}
	else
		SetThink(&CBaseCharger::SUB_DoNothing);
}

void CBaseCharger::Recharge()
{
	EmitSound(CHAN_ITEM, m_StartupSound, m_SoundVolume, ATTN_NORM);
	m_Juice = m_TotalJuice;
	pev->frame = 0;
	SetThink(&CBaseCharger::SUB_DoNothing);
}

/**
 *	@brief Wall mounted health kit
 */
class CWallHealth : public CBaseCharger
{
public:
	void OnCreate() override
	{
		CBaseCharger::OnCreate();
		m_TotalJuice = GetSkillFloat("healthcharger"sv);
		m_RechargeDelay = g_pGameRules->HealthChargerRechargeTime();
		m_SoundVolume = VOL_NORM;
		m_StartupSound = "items/medshot4.wav";
		m_DenySound = "items/medshotno1.wav";
		m_DispensingSound = "items/medcharge4.wav";
	}

protected:
	bool TryCharge(CBasePlayer* player) override
	{
		return player->TakeHealth(1, DMG_GENERIC);
	}
};

LINK_ENTITY_TO_CLASS(func_healthcharger, CWallHealth);

class CRecharge : public CBaseCharger
{
public:
	void OnCreate() override
	{
		CBaseCharger::OnCreate();
		m_TotalJuice = GetSkillFloat("suitcharger"sv);
		m_RechargeDelay = g_pGameRules->HEVChargerRechargeTime();
		m_SoundVolume = 0.85f;
		m_StartupSound = "items/suitchargeok1.wav";
		m_DenySound = "items/suitchargeno1.wav";
		m_DispensingSound = "items/suitcharge1.wav";
	}

protected:
	bool TryCharge(CBasePlayer* player) override
	{
		if (player->pev->armorvalue >= MAX_NORMAL_BATTERY)
		{
			return false;
		}

		player->pev->armorvalue += 1;

		if (player->pev->armorvalue > MAX_NORMAL_BATTERY)
			player->pev->armorvalue = MAX_NORMAL_BATTERY;

		return true;
	}
};

LINK_ENTITY_TO_CLASS(func_recharge, CRecharge);
