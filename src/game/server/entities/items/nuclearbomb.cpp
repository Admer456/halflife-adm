/***
 *
 *	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
 *
 *	This product contains software technology licensed from Id
 *	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
 *	All Rights Reserved.
 *
 *   This source code contains proprietary and confidential information of
 *   Valve LLC and its suppliers.  Access to this code is restricted to
 *   persons who have executed a written SDK license with Valve.  Any access,
 *   use or distribution of this code by or to any unlicensed person is illegal.
 *
 ****/
#include "cbase.h"

class COFNuclearBombButton : public CBaseEntity
{
public:
	int ObjectCaps() override { return FCAP_DONT_SAVE; }

	void OnCreate() override;
	void Precache() override;
	void Spawn() override;

	void SetNuclearBombButton(bool fOn);
};

LINK_ENTITY_TO_CLASS(item_nuclearbombbutton, COFNuclearBombButton);

void COFNuclearBombButton::OnCreate()
{
	CBaseEntity::OnCreate();

	pev->model = MAKE_STRING("models/nuke_button.mdl");
}

void COFNuclearBombButton::Precache()
{
	PrecacheModel(STRING(pev->model));
}

void COFNuclearBombButton::Spawn()
{
	Precache();

	SetModel(STRING(pev->model));

	pev->solid = SOLID_NOT;

	UTIL_SetSize(pev, {-16, -16, 0}, {16, 16, 32});

	pev->movetype = MOVETYPE_NONE;

	UTIL_SetOrigin(pev, pev->origin);

	if (DROP_TO_FLOOR(edict()) == 0)
	{
		CBaseEntity::Logger->error("Nuclear Bomb Button fell out of level at {}", pev->origin);
		UTIL_Remove(this);
	}
	else
	{
		pev->skin = 0;
	}
}

void COFNuclearBombButton::SetNuclearBombButton(bool fOn)
{
	pev->skin = fOn ? 1 : 0;
}

class COFNuclearBombTimer : public CBaseEntity
{
public:
	int ObjectCaps() override { return FCAP_DONT_SAVE; }

	void OnCreate() override;
	void Precache() override;
	void Spawn() override;

	void EXPORT NuclearBombTimerThink();

	void SetNuclearBombTimer(bool fOn);

	bool bPlayBombSound;
	bool bBombSoundPlaying;
};

LINK_ENTITY_TO_CLASS(item_nuclearbombtimer, COFNuclearBombTimer);

void COFNuclearBombTimer::OnCreate()
{
	CBaseEntity::OnCreate();

	pev->model = MAKE_STRING("models/nuke_timer.mdl");
}

void COFNuclearBombTimer::Precache()
{
	PrecacheModel(STRING(pev->model));
	PrecacheSound("common/nuke_ticking.wav");
}

void COFNuclearBombTimer::Spawn()
{
	Precache();

	SetModel(STRING(pev->model));

	pev->solid = SOLID_NOT;

	UTIL_SetSize(pev, {-16, -16, 0}, {16, 16, 32});

	pev->movetype = MOVETYPE_NONE;

	UTIL_SetOrigin(pev, pev->origin);

	if (DROP_TO_FLOOR(edict()) == 0)
	{
		CBaseEntity::Logger->error("Nuclear Bomb Timer fell out of level at {}", pev->origin);
		UTIL_Remove(this);
	}
	else
	{
		pev->skin = 0;
		bPlayBombSound = true;
		bBombSoundPlaying = true;
	}
}

void COFNuclearBombTimer::NuclearBombTimerThink()
{
	if (pev->skin <= 1)
		++pev->skin;
	else
		pev->skin = 0;

	if (bPlayBombSound)
	{
		EMIT_SOUND(edict(), CHAN_BODY, "common/nuke_ticking.wav", 0.75, ATTN_IDLE);
		bBombSoundPlaying = true;
	}

	pev->nextthink = gpGlobals->time + 0.1;
}

void COFNuclearBombTimer::SetNuclearBombTimer(bool fOn)
{
	if (fOn)
	{
		SetThink(&COFNuclearBombTimer::NuclearBombTimerThink);
		pev->nextthink = gpGlobals->time + 0.5;
		bPlayBombSound = true;
	}
	else
	{
		SetThink(nullptr);
		pev->nextthink = gpGlobals->time;

		pev->skin = 3;

		if (bBombSoundPlaying)
		{
			STOP_SOUND(edict(), CHAN_BODY, "common/nuke_ticking.wav");
			bBombSoundPlaying = false;
		}
	}
}

class COFNuclearBomb : public CBaseToggle
{
public:
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	int ObjectCaps() override { return CBaseToggle::ObjectCaps() | FCAP_IMPULSE_USE; }

	bool KeyValue(KeyValueData* pkvd) override;
	void OnCreate() override;
	void Precache() override;
	void Spawn() override;

	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

	COFNuclearBombTimer* m_pTimer;
	COFNuclearBombButton* m_pButton;
	bool m_fOn;
	float m_flLastPush;
	int m_iPushCount;
};

TYPEDESCRIPTION COFNuclearBomb::m_SaveData[] =
	{
		DEFINE_FIELD(COFNuclearBomb, m_fOn, FIELD_BOOLEAN),
		DEFINE_FIELD(COFNuclearBomb, m_flLastPush, FIELD_TIME),
		DEFINE_FIELD(COFNuclearBomb, m_iPushCount, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(COFNuclearBomb, CBaseToggle);

LINK_ENTITY_TO_CLASS(item_nuclearbomb, COFNuclearBomb);

bool COFNuclearBomb::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq("initialstate", pkvd->szKeyName))
	{
		m_fOn = atoi(pkvd->szValue) != 0;
		return true;
	}
	else if (FStrEq("wait", pkvd->szKeyName))
	{
		m_flWait = atof(pkvd->szValue);
		return true;
	}

	return CBaseToggle::KeyValue(pkvd);
}

void COFNuclearBomb::OnCreate()
{
	CBaseToggle::OnCreate();

	pev->model = MAKE_STRING("models/nuke_case.mdl");
}

void COFNuclearBomb::Precache()
{
	PrecacheModel(STRING(pev->model));
	UTIL_PrecacheOther("item_nuclearbombtimer");
	UTIL_PrecacheOther("item_nuclearbombbutton");
	PrecacheSound("buttons/button4.wav");
	PrecacheSound("buttons/button6.wav");

	// The other entities are created here since a restore only calls Precache
	// TODO: set the classname members for both entities
	m_pTimer = GetClassPtr<COFNuclearBombTimer>(nullptr);

	m_pTimer->pev->origin = pev->origin;
	m_pTimer->pev->angles = pev->angles;

	m_pTimer->Spawn();

	m_pTimer->SetNuclearBombTimer(m_fOn);

	m_pButton = GetClassPtr<COFNuclearBombButton>(nullptr);

	m_pButton->pev->origin = pev->origin;
	m_pButton->pev->angles = pev->angles;

	m_pButton->Spawn();

	m_pButton->pev->skin = static_cast<int>(m_fOn);
}

void COFNuclearBomb::Spawn()
{
	Precache();

	SetModel(STRING(pev->model));

	pev->solid = SOLID_BBOX;

	UTIL_SetOrigin(pev, pev->origin);
	UTIL_SetSize(pev, {-16, -16, 0}, {16, 16, 32});

	pev->movetype = MOVETYPE_NONE;

	if (DROP_TO_FLOOR(edict()))
	{
		m_iPushCount = 0;
		m_flLastPush = gpGlobals->time;
	}
	else
	{
		CBaseEntity::Logger->error("Nuclear Bomb fell out of level at {}", pev->origin);
		UTIL_Remove(this);
	}
}

void COFNuclearBomb::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if ((m_flWait != -1.0 || m_iPushCount <= 0) && m_flWait <= gpGlobals->time - m_flLastPush && ShouldToggle(useType, m_fOn))
	{
		if (m_fOn)
		{
			m_fOn = false;
			EMIT_SOUND(edict(), CHAN_VOICE, "buttons/button4.wav", VOL_NORM, ATTN_NORM);
		}
		else
		{
			m_fOn = true;
			EMIT_SOUND(edict(), CHAN_VOICE, "buttons/button6.wav", VOL_NORM, ATTN_NORM);
		}

		SUB_UseTargets(pActivator, USE_TOGGLE, 0);

		if (m_pButton)
		{
			m_pButton->SetNuclearBombButton(m_fOn);
		}

		if (m_pTimer)
		{
			m_pTimer->SetNuclearBombTimer(m_fOn);
		}

		if (!m_pTimer || !m_pTimer->bBombSoundPlaying)
		{
			++m_iPushCount;
			m_flLastPush = gpGlobals->time;
		}
	}
}
