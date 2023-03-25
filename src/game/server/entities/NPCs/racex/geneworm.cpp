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

const int AE_GENEWORM_SPIT_START = 0;
const int AE_GENEWORM_LAUNCH_SPAWN = 2;
const int AE_GENEWORM_SLASH_LEFT_ON = 3;
const int AE_GENEWORM_SLASH_LEFT_OFF = 4;
const int AE_GENEWORM_SLASH_RIGHT_ON = 5;
const int AE_GENEWORM_SLASH_RIGHT_OFF = 6;
const int AE_GENEWORM_SLASH_CENTER_ON = 7;
const int AE_GENEWORM_SLASH_CENTER_OFF = 8;
const int AE_GENEWORM_HIT_WALL = 9;

class COFGeneWormCloud : public CBaseEntity
{
public:
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	int Classify() override { return CLASS_NONE; }

	void Precache() override;
	void Spawn() override;

	void EXPORT GeneWormCloudThink();

	void EXPORT GeneWormCloudTouch(CBaseEntity* pOther);

	void RunGeneWormCloud(float frames);

	void TurnOn();

	static COFGeneWormCloud* GeneWormCloudCreate(const Vector& origin);

	void LaunchCloud(const Vector& origin, const Vector& aim, int nSpeed, edict_t* pOwner, float fadeTime);

	float m_lastTime;
	float m_maxFrame;

	bool m_bLaunched;

	float m_fadeScale;
	float m_fadeRender;
	float m_damageTimer;

	bool m_fSinking;
};

TYPEDESCRIPTION COFGeneWormCloud::m_SaveData[] =
	{
		DEFINE_FIELD(COFGeneWormCloud, m_lastTime, FIELD_FLOAT),
		DEFINE_FIELD(COFGeneWormCloud, m_maxFrame, FIELD_FLOAT),
		DEFINE_FIELD(COFGeneWormCloud, m_bLaunched, FIELD_BOOLEAN),
		DEFINE_FIELD(COFGeneWormCloud, m_fadeScale, FIELD_FLOAT),
		DEFINE_FIELD(COFGeneWormCloud, m_fadeRender, FIELD_FLOAT),
		DEFINE_FIELD(COFGeneWormCloud, m_damageTimer, FIELD_FLOAT),
		DEFINE_FIELD(COFGeneWormCloud, m_fSinking, FIELD_BOOLEAN),
};

IMPLEMENT_SAVERESTORE(COFGeneWormCloud, CBaseEntity);

LINK_ENTITY_TO_CLASS(env_genewormcloud, COFGeneWormCloud);

void COFGeneWormCloud::Precache()
{
	PrecacheModel("sprites/ballsmoke.spr");
}

void COFGeneWormCloud::Spawn()
{
	Precache();

	pev->solid = SOLID_BBOX;
	pev->movetype = MOVETYPE_FLY;
	pev->effects = 0;
	pev->frame = 0;

	SetModel("sprites/ballsmoke.spr");

	UTIL_SetOrigin(pev, pev->origin);

	SetSize(g_vecZero, g_vecZero);

	m_maxFrame = MODEL_FRAMES(pev->modelindex) - 1;

	if (0 != pev->angles.y && 0 == pev->angles.z)
	{
		pev->angles.z = pev->angles.y;
		pev->angles.y = 0;
	}

	pev->scale = 0.01;

	m_fadeScale = 0;

	m_bLaunched = false;
	m_damageTimer = gpGlobals->time;
	m_fSinking = false;
}

void COFGeneWormCloud::GeneWormCloudThink()
{
	RunGeneWormCloud((gpGlobals->time - m_lastTime) * pev->framerate);

	pev->nextthink = gpGlobals->time + 0.1;
	m_lastTime = gpGlobals->time;
}

void COFGeneWormCloud::GeneWormCloudTouch(CBaseEntity* pOther)
{
	if ((!pev->owner || pOther->pev->modelindex != pev->owner->v.modelindex) && pev->modelindex != pOther->pev->modelindex)
	{
		if (pOther->pev->takedamage != DAMAGE_NO)
		{
			pOther->TakeDamage(this, this, GetSkillFloat("geneworm_dmg_spit"sv), DMG_ACID);
		}

		pev->nextthink = gpGlobals->time;
		SetThink(nullptr);
		UTIL_Remove(this);
	}
}

void COFGeneWormCloud::RunGeneWormCloud(float frames)
{
	if (m_bLaunched)
	{
		pev->scale += m_fadeScale;
		pev->renderamt -= m_fadeRender;

		if (pev->scale >= 4.5)
		{
			pev->scale = 0;
			UTIL_Remove(this);
			return;
		}
	}
	else if (pev->scale < 2.0)
	{
		pev->scale += 0.05;
	}

	pev->frame += frames;

	if (pev->frame > m_maxFrame && m_maxFrame > 0)
	{
		// TODO: the original code looks like it may be ignoring the modulo, verify this
		pev->frame = fmod(pev->frame, m_maxFrame);
	}
}

void COFGeneWormCloud::TurnOn()
{
	pev->effects = 0;

	if (pev->framerate != 0 && m_maxFrame > 1.0 || (pev->spawnflags & 2) != 0)
	{
		SetThink(&COFGeneWormCloud::GeneWormCloudThink);
		pev->nextthink = gpGlobals->time;
		m_lastTime = gpGlobals->time;
	}

	pev->frame = 0;
}

COFGeneWormCloud* COFGeneWormCloud::GeneWormCloudCreate(const Vector& origin)
{
	auto pCloud = g_EntityDictionary->Create<COFGeneWormCloud>("env_genewormcloud");

	pCloud->Spawn();

	pCloud->pev->origin = origin;
	pCloud->pev->solid = SOLID_BBOX;
	pCloud->pev->movetype = MOVETYPE_FLY;
	pCloud->pev->effects = 0;

	pCloud->TurnOn();

	pCloud->SetTouch(&COFGeneWormCloud::GeneWormCloudTouch);

	return pCloud;
}

void COFGeneWormCloud::LaunchCloud(const Vector& origin, const Vector& aim, int nSpeed, edict_t* pOwner, float fadeTime)
{
	pev->angles = pOwner->v.angles;

	pev->owner = pOwner;

	pev->velocity = aim * nSpeed;

	m_fadeScale = 2.5 / fadeTime;
	m_fadeRender = (pev->renderamt - 7.0) / fadeTime;

	pev->skin = 0;
	pev->body = 0;
	pev->aiment = nullptr;
	pev->movetype = MOVETYPE_FLY;

	UTIL_SetOrigin(pev, origin);

	SetTouch(&COFGeneWormCloud::GeneWormCloudTouch);
	m_bLaunched = true;
}

const auto GENEWORM_SPAWN_BEAM_COUNT = 8;

class COFGeneWormSpawn : public CBaseEntity
{
public:
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	void Precache() override;
	void Spawn() override;

	void EXPORT GeneWormSpawnThink();

	void EXPORT GeneWormSpawnTouch(CBaseEntity* pOther);

	void RunGeneWormSpawn(float frames);

	void TurnOn();

	static COFGeneWormSpawn* GeneWormSpawnCreate(const Vector& origin);

	void LaunchSpawn(const Vector& origin, const Vector& aim, int nSpeed, edict_t* pOwner, float flBirthTime);

	void CreateWarpBeams(int side);

	float m_lastTime;
	float m_maxFrame;
	float m_flBirthTime;
	float m_flWarpTime;

	bool m_bLaunched;
	bool m_bWarping;
	bool m_bTrooperDropped;

	CBeam* m_pBeam[GENEWORM_SPAWN_BEAM_COUNT];

	int m_iBeams;
};

TYPEDESCRIPTION COFGeneWormSpawn::m_SaveData[] =
	{
		DEFINE_FIELD(COFGeneWormSpawn, m_lastTime, FIELD_FLOAT),
		DEFINE_FIELD(COFGeneWormSpawn, m_maxFrame, FIELD_FLOAT),
		DEFINE_FIELD(COFGeneWormSpawn, m_flBirthTime, FIELD_FLOAT),
		DEFINE_FIELD(COFGeneWormSpawn, m_flWarpTime, FIELD_FLOAT),
		DEFINE_FIELD(COFGeneWormSpawn, m_bLaunched, FIELD_BOOLEAN),
		DEFINE_FIELD(COFGeneWormSpawn, m_bWarping, FIELD_BOOLEAN),
		DEFINE_FIELD(COFGeneWormSpawn, m_bTrooperDropped, FIELD_BOOLEAN),
		DEFINE_ARRAY(COFGeneWormSpawn, m_pBeam, FIELD_CLASSPTR, GENEWORM_SPAWN_BEAM_COUNT),
		DEFINE_FIELD(COFGeneWormSpawn, m_iBeams, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(COFGeneWormSpawn, CBaseEntity);

LINK_ENTITY_TO_CLASS(env_genewormspawn, COFGeneWormSpawn);

void COFGeneWormSpawn::Precache()
{
	PrecacheModel("sprites/tele1.spr");
	PrecacheModel("sprites/lgtning.spr");
	PrecacheModel("sprites/boss_glow.spr");
}

void COFGeneWormSpawn::Spawn()
{
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	pev->effects = 0;
	pev->frame = 0;

	Precache();

	SetModel("sprites/boss_glow.spr");

	UTIL_SetOrigin(pev, pev->origin);

	SetSize(g_vecZero, g_vecZero);

	m_maxFrame = MODEL_FRAMES(pev->modelindex) - 1;

	if (pev->angles.y != 0 && pev->angles.z == 0)
	{
		pev->angles.z = pev->angles.y;
		pev->angles.y = 0;
	}

	m_bLaunched = false;
	m_bWarping = false;
	m_bTrooperDropped = false;

	m_flBirthTime = gpGlobals->time;
	m_flWarpTime = gpGlobals->time;

	for (auto& pBeam : m_pBeam)
	{
		pBeam = nullptr;
	}

	m_iBeams = 0;
}

void COFGeneWormSpawn::GeneWormSpawnThink()
{
	RunGeneWormSpawn((gpGlobals->time - m_lastTime) * pev->framerate);

	pev->nextthink = gpGlobals->time + 0.1;
	m_lastTime = gpGlobals->time;
}

void COFGeneWormSpawn::GeneWormSpawnTouch(CBaseEntity* pOther)
{
	// Nothing
}

void COFGeneWormSpawn::RunGeneWormSpawn(float frames)
{
	if (m_bLaunched)
	{
		if (gpGlobals->time > m_flBirthTime)
		{
			if (m_bWarping)
			{
				if (m_bTrooperDropped)
				{
					if (pev->scale < 0.1)
					{
						m_bLaunched = false;
						m_bWarping = false;
						m_bTrooperDropped = false;
						UTIL_Remove(this);
						return;
					}
					pev->scale = pev->scale - 0.075;
				}
				else
				{
					if (m_flWarpTime - 1.0 < gpGlobals->time)
					{
						UTIL_MakeVectors(pev->angles);

						const auto vecStart = pev->origin + Vector(0, 0, 1000);
						const auto vecEnd = pev->origin + Vector(0, 0, -1000);

						TraceResult tr;
						UTIL_TraceLine(vecStart, vecEnd, ignore_monsters, dont_ignore_glass, edict(), &tr);

						m_bTrooperDropped = true;

						CBaseEntity::Create("monster_shocktrooper", tr.vecEndPos + Vector(0, 0, 48), pev->angles);
					}
					else
					{
						::RadiusDamage(pev->origin, this, this, 1000.0, 128.0, CLASS_NONE, DMG_ALWAYSGIB | DMG_SHOCK);
						CreateWarpBeams(1);
						CreateWarpBeams(-1);
					}
				}
			}
			else
			{
				pev->velocity = g_vecZero;

				m_bWarping = true;
				m_flWarpTime = gpGlobals->time + 3.0;

				EmitSound(CHAN_WEAPON, "debris/beamstart2.wav", VOL_NORM, ATTN_NORM);
			}
		}
		else
		{
			pev->scale = pev->scale + 0.1;
		}

		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_ELIGHT);
		WRITE_SHORT(entindex());
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z);
		WRITE_COORD(96.0);
		WRITE_BYTE(207);
		WRITE_BYTE(214);
		WRITE_BYTE(0);
		WRITE_BYTE(1);
		WRITE_COORD(1.0);
		MESSAGE_END();
	}
	else
	{
		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_ELIGHT);
		WRITE_SHORT(entindex());
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z);
		WRITE_COORD(96.0);
		WRITE_BYTE(207);
		WRITE_BYTE(0);
		WRITE_BYTE(214);
		WRITE_BYTE(1);
		WRITE_COORD(1.0);
		MESSAGE_END();
	}

	pev->frame += frames;

	if (pev->frame > m_maxFrame && m_maxFrame > 0)
	{
		pev->frame = fmod(pev->frame, m_maxFrame);
	}
}

void COFGeneWormSpawn::TurnOn()
{
	pev->effects = 0;

	if (pev->framerate != 0 && m_maxFrame > 1.0 || (pev->spawnflags & 2) != 0)
	{
		SetThink(&COFGeneWormSpawn::GeneWormSpawnThink);
		pev->nextthink = gpGlobals->time;
		m_lastTime = gpGlobals->time;
	}

	pev->frame = 0;
}

COFGeneWormSpawn* COFGeneWormSpawn::GeneWormSpawnCreate(const Vector& origin)
{
	auto pSpawn = g_EntityDictionary->Create<COFGeneWormSpawn>("env_genewormspawn");

	pSpawn->Spawn();

	pSpawn->pev->origin = origin;
	pSpawn->pev->solid = SOLID_NOT;
	pSpawn->pev->movetype = MOVETYPE_NONE;
	pSpawn->pev->effects = 0;

	pSpawn->TurnOn();

	return pSpawn;
}

void COFGeneWormSpawn::LaunchSpawn(const Vector& origin, const Vector& aim, int nSpeed, edict_t* pOwner, float flBirthTime)
{
	pev->movetype = MOVETYPE_FLY;

	pev->angles = pOwner->v.angles;
	pev->owner = pOwner;
	pev->velocity = aim * nSpeed;

	pev->speed = nSpeed;
	pev->skin = 0;
	pev->body = 0;
	pev->aiment = nullptr;
	pev->movetype = MOVETYPE_FLY;

	SetModel("sprites/tele1.spr");

	m_maxFrame = MODEL_FRAMES(pev->modelindex) - 1;

	pev->frame = 0;

	UTIL_SetOrigin(pev, origin);

	SetTouch(&COFGeneWormSpawn::GeneWormSpawnTouch);
	m_bLaunched = true;
	m_flBirthTime = flBirthTime + gpGlobals->time;
}

void COFGeneWormSpawn::CreateWarpBeams(int side)
{
	TraceResult tr;
	float flDist = 1.0;

	if (m_iBeams >= GENEWORM_SPAWN_BEAM_COUNT)
		return;

	UTIL_MakeAimVectors(pev->angles);
	Vector vecSrc = pev->origin + gpGlobals->v_up * 36 + gpGlobals->v_right * side * 16 + gpGlobals->v_forward * 32;

	for (int i = 0; i < 3; i++)
	{
		Vector vecAim = gpGlobals->v_right * side * RANDOM_FLOAT(0, 1) + gpGlobals->v_up * RANDOM_FLOAT(-1, 1);
		TraceResult tr1;
		UTIL_TraceLine(vecSrc, vecSrc + vecAim * 512, dont_ignore_monsters, ENT(pev), &tr1);
		if (flDist > tr1.flFraction)
		{
			tr = tr1;
			flDist = tr.flFraction;
		}
	}

	// Couldn't find anything close enough
	if (flDist == 1.0)
		return;

	m_pBeam[m_iBeams] = CBeam::BeamCreate("sprites/lgtning.spr", 30);
	if (!m_pBeam[m_iBeams])
		return;

	auto pHit = Instance(tr.pHit);

	if (pHit && pHit->pev->takedamage != DAMAGE_NO)
	{
		m_pBeam[m_iBeams]->EntsInit(pHit->entindex(), entindex());
		m_pBeam[m_iBeams]->SetColor(255, 255, 0);
		m_pBeam[m_iBeams]->SetBrightness(255);
		m_pBeam[m_iBeams]->SetNoise(20);
	}
	else
	{
		m_pBeam[m_iBeams]->PointEntInit(tr.vecEndPos, entindex());
		m_pBeam[m_iBeams]->SetColor(255, 255, 0);
		m_pBeam[m_iBeams]->SetBrightness(192);
		m_pBeam[m_iBeams]->SetNoise(80);
	}

	m_pBeam[m_iBeams]->SetThink(&CBeam::SUB_Remove);
	m_pBeam[m_iBeams]->pev->nextthink = gpGlobals->time + 1;

	++m_iBeams;
}

int iGeneWormSpitSprite;

class COFGeneWorm : public CBaseMonster
{
public:
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	int Classify() override { return CLASS_ALIEN_MONSTER; }

	int BloodColor() override { return BLOOD_COLOR_GREEN; }

	// Don't gib ever
	void GibMonster() override {}

	void SetObjectCollisionBox() override
	{
		pev->absmin = pev->origin + Vector(-436.67, -720.49, -331.74);
		pev->absmax = pev->origin + Vector(425.29, 164.85, 355.68);
	}

	void OnCreate() override;
	void Precache() override;
	void Spawn() override;

	bool TakeDamage(CBaseEntity* inflictor, CBaseEntity* attacker, float flDamage, int bitsDamageType) override;

	void TraceAttack(CBaseEntity* attacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType) override;

	void Killed(CBaseEntity* attacker, int iGib) override
	{
		CBaseMonster::Killed(attacker, iGib);
	}

	bool FVisible(CBaseEntity* pEntity) override;

	bool FVisible(const Vector& vecOrigin) override;

	void HandleAnimEvent(MonsterEvent_t* pEvent) override;

	void EXPORT StartupThink();

	void EXPORT HuntThink();

	void EXPORT DyingThink();

	void EXPORT NullThink();

	void EXPORT HitTouch(CBaseEntity* pOther);

	void EXPORT CommandUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	void NextActivity();

	bool ClawAttack();

	void SpewCloud();

	void TrackHead();

	static const char* pIdleSounds[];
	static const char* pSpawnSounds[];

	float m_flNextPainSound;

	Vector m_posTarget;
	float m_flLastSeen;
	float m_flPrevSeen;

	COFGeneWormCloud* m_pCloud;

	int m_iWasHit;
	float m_flTakeHitTime;
	float m_flHitTime;
	float m_flNextMeleeTime;
	float m_flNextRangeTime;

	bool m_fRightEyeHit;
	bool m_fLeftEyeHit;
	bool m_fGetMad;

	bool m_fOrificeHit;
	float m_flOrificeOpenTime;
	COFGeneWormSpawn* m_orificeGlow;

	bool m_fSpawningTrooper;
	float m_flSpawnTrooperTime;

	int m_iHitTimes;
	int m_iMaxHitTimes;

	float m_offsetBeam;
	float m_lenBeam;
	Vector m_posBeam;
	Vector m_vecBeam;
	Vector m_angleBeam;
	float m_flBeamExpireTime;
	float m_flBeamDir;

	bool m_fSpitting;
	float m_flSpitStartTime;

	bool m_fActivated;
	float m_flDeathStart;
	bool m_fHasEntered;

	float m_flMadDelayTime;
};

TYPEDESCRIPTION COFGeneWorm::m_SaveData[] =
	{
		DEFINE_FIELD(COFGeneWorm, m_flNextPainSound, FIELD_TIME),
		DEFINE_FIELD(COFGeneWorm, m_posTarget, FIELD_POSITION_VECTOR),
		DEFINE_FIELD(COFGeneWorm, m_flLastSeen, FIELD_TIME),
		DEFINE_FIELD(COFGeneWorm, m_flPrevSeen, FIELD_TIME),
		DEFINE_FIELD(COFGeneWorm, m_pCloud, FIELD_CLASSPTR),
		DEFINE_FIELD(COFGeneWorm, m_iWasHit, FIELD_INTEGER),
		DEFINE_FIELD(COFGeneWorm, m_flTakeHitTime, FIELD_FLOAT),
		DEFINE_FIELD(COFGeneWorm, m_flHitTime, FIELD_FLOAT),
		DEFINE_FIELD(COFGeneWorm, m_flNextMeleeTime, FIELD_FLOAT),
		DEFINE_FIELD(COFGeneWorm, m_flNextRangeTime, FIELD_FLOAT),
		DEFINE_FIELD(COFGeneWorm, m_fRightEyeHit, FIELD_BOOLEAN),
		DEFINE_FIELD(COFGeneWorm, m_fLeftEyeHit, FIELD_BOOLEAN),
		DEFINE_FIELD(COFGeneWorm, m_fGetMad, FIELD_BOOLEAN),
		DEFINE_FIELD(COFGeneWorm, m_fOrificeHit, FIELD_BOOLEAN),
		DEFINE_FIELD(COFGeneWorm, m_flOrificeOpenTime, FIELD_FLOAT),
		DEFINE_FIELD(COFGeneWorm, m_orificeGlow, FIELD_CLASSPTR),
		DEFINE_FIELD(COFGeneWorm, m_fSpawningTrooper, FIELD_BOOLEAN),
		DEFINE_FIELD(COFGeneWorm, m_flSpawnTrooperTime, FIELD_FLOAT),
		DEFINE_FIELD(COFGeneWorm, m_iHitTimes, FIELD_INTEGER),
		DEFINE_FIELD(COFGeneWorm, m_iMaxHitTimes, FIELD_INTEGER),
		DEFINE_FIELD(COFGeneWorm, m_fSpitting, FIELD_BOOLEAN),
		DEFINE_FIELD(COFGeneWorm, m_flSpitStartTime, FIELD_TIME),
		DEFINE_FIELD(COFGeneWorm, m_fActivated, FIELD_BOOLEAN),
		DEFINE_FIELD(COFGeneWorm, m_flDeathStart, FIELD_TIME),
		DEFINE_FIELD(COFGeneWorm, m_fHasEntered, FIELD_BOOLEAN),
		DEFINE_FIELD(COFGeneWorm, m_flMadDelayTime, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(COFGeneWorm, CBaseMonster);

LINK_ENTITY_TO_CLASS(monster_geneworm, COFGeneWorm);

const char* COFGeneWorm::pIdleSounds[] =
	{
		"geneworm/geneworm_idle1.wav",
		"geneworm/geneworm_idle2.wav",
		"geneworm/geneworm_idle3.wav",
		"geneworm/geneworm_idle4.wav"};

const char* COFGeneWorm::pSpawnSounds[] =
	{
		"debris/beamstart7.wav"};

void COFGeneWorm::Precache()
{
	PrecacheModel(STRING(pev->model));
	PrecacheModel("sprites/lgtning.spr");
	PrecacheModel("sprites/tele1.spr");
	PrecacheModel("sprites/bigspit.spr");
	PrecacheModel("sprites/boss_glow.spr");

	iGeneWormSpitSprite = PrecacheModel("sprites/tinyspit.spr");

	PrecacheModel("sprites/xbeam3.spr");

	UTIL_PrecacheOther("monster_shocktrooper");

	PrecacheSound("bullchicken/bc_acid1.wav");
	PrecacheSound("bullchicken/bc_spithit1.wav");
	PrecacheSound("bullchicken/bc_spithit2.wav");

	PRECACHE_SOUND_ARRAY(pIdleSounds);
	PRECACHE_SOUND_ARRAY(pSpawnSounds);

	PrecacheSound("debris/beamstart7.wav");
	PrecacheSound("debris/beamstart2.wav");

	PrecacheModel("sprites/xspark4.spr");
	PrecacheModel("sprites/ballsmoke.spr");

	PrecacheSound("geneworm/geneworm_attack_mounted_gun.wav");
	PrecacheSound("geneworm/geneworm_attack_mounted_rocket.wav");

	PrecacheSound("geneworm/geneworm_beam_attack.wav");
	PrecacheSound("geneworm/geneworm_big_attack_forward.wav");

	PrecacheSound("geneworm/geneworm_death.wav");

	PrecacheSound("geneworm/geneworm_final_pain1.wav");
	PrecacheSound("geneworm/geneworm_final_pain2.wav");
	PrecacheSound("geneworm/geneworm_final_pain3.wav");
	PrecacheSound("geneworm/geneworm_final_pain4.wav");
	PrecacheSound("geneworm/geneworm_shot_in_eye.wav");

	PrecacheSound("geneworm/geneworm_entry.wav");
}

void COFGeneWorm::OnCreate()
{
	CBaseMonster::OnCreate();

	pev->health = GetSkillFloat("geneworm_health"sv);
	pev->model = MAKE_STRING("models/geneworm.mdl");
}

void COFGeneWorm::Spawn()
{
	Precache();

	pev->movetype = MOVETYPE_FLY;
	pev->solid = SOLID_NOT;

	SetModel(STRING(pev->model));

	SetSize({-436.67, -720.49, -331.74}, {425.29, 164.85, 355.68});

	UTIL_SetOrigin(pev, pev->origin);

	pev->flags |= FL_MONSTER;
	pev->takedamage = DAMAGE_AIM;

	pev->effects = 0;

	pev->max_health = pev->health;

	pev->view_ofs = {0, 0, 300};

	m_bloodColor = BLOOD_COLOR_GREEN;
	m_flFieldOfView = 0.5;

	SetThink(&COFGeneWorm::StartupThink);
	pev->nextthink = gpGlobals->time + 0.1;

	m_iWasHit = 0;
	m_fRightEyeHit = false;
	m_fLeftEyeHit = false;

	m_flTakeHitTime = 0;
	m_flHitTime = 0;

	m_fGetMad = false;
	m_fOrificeHit = false;

	m_flOrificeOpenTime = m_flSpawnTrooperTime = gpGlobals->time;

	m_iHitTimes = 0;

	m_MonsterState = MONSTERSTATE_IDLE;

	m_pCloud = nullptr;

	m_iMaxHitTimes = 4;

	pev->deadflag = DEAD_NO;

	m_fSpitting = false;
	m_fActivated = false;

	m_flSpitStartTime = gpGlobals->time;

	UTIL_SetOrigin(pev, pev->origin);

	pev->rendermode = kRenderTransTexture;

	m_fHasEntered = false;

	m_flMadDelayTime = gpGlobals->time;
}

void COFGeneWorm::StartupThink()
{
	Vector vecEyePos, vecEyeAng;

	GetAttachment(0, vecEyePos, vecEyeAng);

	pev->view_ofs = vecEyePos - pev->origin;

	m_flNextMeleeTime = gpGlobals->time;
	m_flNextRangeTime = gpGlobals->time;

	pev->sequence = LookupSequence("entry");

	pev->frame = 0;

	SetThink(&COFGeneWorm::HuntThink);
	SetUse(&COFGeneWorm::CommandUse);

	pev->nextthink = gpGlobals->time + 0.1;

	SetTouch(&COFGeneWorm::HitTouch);
}

void COFGeneWorm::HuntThink()
{
	pev->nextthink = gpGlobals->time + 0.1;

	if (!m_fActivated)
		return;

	DispatchAnimEvents();
	StudioFrameAdvance();

	UpdateShockEffect();

	if (pev->rendermode == kRenderTransTexture)
	{
		if (pev->renderamt < 248.0)
		{
			pev->renderamt += 3.0;
		}
		else
		{
			pev->renderamt = 255;
			pev->rendermode = kRenderNormal;
			pev->renderfx = kRenderFxNone;
		}
	}

	if (m_iWasHit == 2)
	{
		ClearShockEffect();

		SetThink(&COFGeneWorm::DyingThink);
		m_fSequenceFinished = true;
		return;
	}

	if (m_iWasHit != 1)
	{
		if (m_fSequenceFinished)
		{
			const auto oldSequence = pev->sequence;

			if (!m_fHasEntered)
				m_fHasEntered = true;

			NextActivity();

			if (!m_fSequenceLoops || oldSequence != pev->sequence)
			{
				pev->frame = 0;
				ResetSequenceInfo();
			}
		}
	}
	else
	{
		int piDir = 1;

		if (m_pCloud)
		{
			UTIL_Remove(m_pCloud);
			m_pCloud = nullptr;
		}

		if (m_fSpitting)
			m_fSpitting = false;

		if (!m_fRightEyeHit)
		{
			pev->sequence = FindTransition(pev->sequence, LookupSequence("eyepain1"), &piDir);
			EmitSound(CHAN_VOICE, "geneworm/geneworm_shot_in_eye.wav", VOL_NORM, 0.1);
		}
		else if (!m_fLeftEyeHit)
		{
			pev->sequence = FindTransition(pev->sequence, LookupSequence("eyepain2"), &piDir);
			EmitSound(CHAN_VOICE, "geneworm/geneworm_shot_in_eye.wav", VOL_NORM, 0.1);
		}
		else if (m_fOrificeHit)
		{
			pev->sequence = FindTransition(pev->sequence, LookupSequence("bigpain3"), &piDir);
			EmitSound(CHAN_VOICE, "geneworm/geneworm_final_pain3.wav", VOL_NORM, 0.1);
		}
		else
		{
			pev->sequence = FindTransition(pev->sequence, LookupSequence("bigpain1"), &piDir);
			EmitSound(CHAN_VOICE, "geneworm/geneworm_final_pain1.wav", VOL_NORM, 0.1);

			Vector vecOrigin, vecAngles;
			GetAttachment(1, vecOrigin, vecAngles);

			m_orificeGlow = COFGeneWormSpawn::GeneWormSpawnCreate(vecOrigin);

			m_orificeGlow->pev->rendermode = kRenderTransAdd;
			m_orificeGlow->pev->rendercolor.x = 255;
			m_orificeGlow->pev->rendercolor.y = 255;
			m_orificeGlow->pev->rendercolor.z = 255;
			m_orificeGlow->pev->renderamt = 255;
			m_orificeGlow->pev->renderfx = kRenderFxNoDissipation;

			m_orificeGlow->pev->scale = 0.95;
			m_orificeGlow->pev->framerate = 10;

			m_orificeGlow->TurnOn();
		}

		if (piDir <= 0)
			pev->frame = 255;
		else
			pev->frame = 0;

		ResetSequenceInfo();

		m_iWasHit = 0;
	}

	if (!m_fRightEyeHit)
	{
		Vector vecOrigin, vecAngles;
		GetAttachment(2, vecOrigin, vecAngles);

		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_ELIGHT);
		WRITE_SHORT(entindex() | 3);
		WRITE_COORD(vecOrigin.x);
		WRITE_COORD(vecOrigin.y);
		WRITE_COORD(vecOrigin.z);
		WRITE_COORD(48.0);
		WRITE_BYTE(128);
		WRITE_BYTE(255);
		WRITE_BYTE(128);
		WRITE_BYTE(1);
		WRITE_COORD(2);
		MESSAGE_END();
	}

	if (!m_fLeftEyeHit)
	{
		Vector vecOrigin, vecAngles;
		GetAttachment(3, vecOrigin, vecAngles);

		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_ELIGHT);
		WRITE_SHORT(entindex() | 4);
		WRITE_COORD(vecOrigin.x);
		WRITE_COORD(vecOrigin.y);
		WRITE_COORD(vecOrigin.z);
		WRITE_COORD(48.0);
		WRITE_BYTE(128);
		WRITE_BYTE(255);
		WRITE_BYTE(128);
		WRITE_BYTE(1);
		WRITE_COORD(1);
		MESSAGE_END();
	}

	if (m_orificeGlow)
	{
		// Keep the glow in place relative to the orifice
		Vector vecOrigin, vecAngles;
		GetAttachment(1, vecOrigin, vecAngles);
		UTIL_SetOrigin(m_orificeGlow->pev, vecOrigin);
	}

	if (m_hEnemy)
	{
		if (FVisible(m_hEnemy))
		{
			if (gpGlobals->time - 5.0 > m_flLastSeen)
			{
				m_flPrevSeen = gpGlobals->time;
			}

			m_flLastSeen = gpGlobals->time;

			m_posTarget = m_hEnemy->pev->origin;
			m_posTarget.z += 24.0;
		}

		TrackHead();
	}
	else
	{
		// Look forward
		SetBoneController(0, 0);
	}

	if (m_fSpitting)
	{
		if (gpGlobals->time - m_flSpitStartTime <= 2.0)
		{
			Vector vecOrigin, vecAngles;
			GetAttachment(0, vecOrigin, vecAngles);

			m_pCloud = COFGeneWormCloud::GeneWormCloudCreate(vecOrigin);

			if (m_hEnemy)
			{
				// This all looks like sprite code, but the cloud class doesn't inherit from it
				// Could be it originally cast to sprite to use the helper methods
				m_pCloud->pev->rendermode = kRenderGlow;
				m_pCloud->pev->rendercolor.x = 255;
				m_pCloud->pev->rendercolor.y = 255;
				m_pCloud->pev->rendercolor.z = 255;
				m_pCloud->pev->renderamt = 255;
				m_pCloud->pev->renderfx = kRenderFxNoDissipation;

				m_pCloud->pev->rendercolor.x = 0;
				m_pCloud->pev->rendercolor.y = 255;
				m_pCloud->pev->rendercolor.z = 0;

				if (edict())
				{
					m_pCloud->pev->skin = entindex();
					m_pCloud->pev->body = 1;
					m_pCloud->pev->aiment = edict();
					m_pCloud->pev->movetype = MOVETYPE_FOLLOW;
				}

				m_pCloud->pev->scale = 0.5;
				m_pCloud->pev->framerate = 10;

				m_pCloud->TurnOn();

				m_pCloud->LaunchCloud(vecOrigin, (m_hEnemy->pev->origin + m_hEnemy->pev->view_ofs - vecOrigin).Normalize() + Vector(0, 0, RANDOM_FLOAT(-0.01, 0.01)), 1000, edict(), 35);

				m_pCloud = nullptr;
			}
		}
		else
		{
			m_fSpitting = false;
		}
	}
}

void COFGeneWorm::DyingThink()
{
	pev->nextthink = gpGlobals->time + 0.1;

	DispatchAnimEvents();
	StudioFrameAdvance();

	if (m_fSequenceFinished && pev->deadflag == DEAD_DYING)
	{
		UTIL_Remove(this);
		return;
	}

	if (pev->deadflag == DEAD_NO)
	{
		pev->deadflag = DEAD_DYING;

		pev->frame = 0;

		// Note: bugged in vanilla, variable is not initialized and causes the ending sequence to break
		int iDir = 0;

		pev->sequence = FindTransition(pev->sequence, LookupSequence("death"), &iDir);

		if (iDir > 0)
			pev->frame = 255;
		else
			pev->frame = 0;

		pev->renderfx = kRenderFxNone;
		pev->rendermode = kRenderTransTexture;

		pev->renderamt = 255;

		ResetSequenceInfo();

		EmitSound(CHAN_VOICE, "geneworm/geneworm_death.wav", VOL_NORM, 0.1);

		m_flDeathStart = gpGlobals->time;

		FireTargets("GeneWormDead", this, this, USE_TOGGLE, 0);

		for (auto pTrooper : UTIL_FindEntitiesByClassname("monster_shocktrooper"))
		{
			pTrooper->SetThink(&CBaseEntity::SUB_FadeOut);
			pTrooper->pev->nextthink = gpGlobals->time + 0.1;
		}

		for (auto pRoach : UTIL_FindEntitiesByClassname("monster_shockroach"))
		{
			pRoach->SetThink(&CBaseEntity::SUB_FadeOut);
			pRoach->pev->nextthink = gpGlobals->time + 0.1;
		}
	}

	if (gpGlobals->time - m_flDeathStart >= 15)
	{
		auto pPlayer = UTIL_GetLocalPlayer();

		if (pPlayer)
		{
			// Teleport the player to the end script
			// TODO: this really shouldn't be hardcoded
			for (auto pTeleport : UTIL_FindEntitiesByTargetname("GeneWormTeleport"))
			{
				pTeleport->Touch(pPlayer);
				AILogger->debug("Touching Target GeneWormTeleport");
			}

			FireTargets("GeneWormTeleport", pPlayer, pPlayer, USE_ON, 1);
		}

		m_flDeathStart = gpGlobals->time + 999;
	}

	if (pev->deadflag == DEAD_DYING)
	{
		pev->renderamt -= 1;
	}

	if (m_pCloud)
	{
		UTIL_Remove(m_pCloud);
		m_pCloud = nullptr;
	}

	if (m_orificeGlow)
	{
		UTIL_Remove(m_orificeGlow);
		m_orificeGlow = nullptr;
	}
}

void COFGeneWorm::NullThink()
{
	StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.5;
}

void COFGeneWorm::HitTouch(CBaseEntity* pOther)
{
	auto tr = UTIL_GetGlobalTrace();

	if (pOther->pev->modelindex != pev->modelindex && m_flHitTime <= gpGlobals->time && tr.pHit && pev->modelindex == tr.pHit->v.modelindex)
	{
		m_flHitTime = gpGlobals->time + 0.5;

		// Apply damage to to the toucher based on what was hit
		switch (tr.iHitgroup)
		{
		case 1:
			pOther->TakeDamage(this, this, 10, DMG_CRUSH | DMG_SLASH);
			break;
		case 2:
			pOther->TakeDamage(this, this, 15, DMG_CRUSH | DMG_SLASH);
			break;
		case 3:
			pOther->TakeDamage(this, this, 20, DMG_CRUSH | DMG_SLASH);
			break;

		default:
			pOther->TakeDamage(this, this, pOther->pev->health, DMG_CRUSH | DMG_SLASH);
			break;
		}

		pOther->pev->punchangle.z = 15;

		// TODO: maybe determine direction of velocity to apply?
		pOther->pev->velocity = pOther->pev->velocity + Vector{0, 0, 200};

		pOther->pev->flags &= ~FL_ONGROUND;
	}
}

void COFGeneWorm::CommandUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (useType == USE_TOGGLE && !m_fActivated)
	{
		pev->sequence = LookupSequence("entry");

		pev->frame = 0;

		ResetSequenceInfo();

		pev->renderamt = 0;

		pev->rendermode = kRenderTransTexture;
		pev->renderfx = kRenderFxNone;

		m_fActivated = true;

		pev->solid = SOLID_BBOX;

		UTIL_SetOrigin(pev, pev->origin);
		EmitSound(CHAN_VOICE, "geneworm/geneworm_entry.wav", VOL_NORM, 0.1);
	}
}

void COFGeneWorm::NextActivity()
{
	UTIL_MakeAimVectors(pev->angles);

	if (m_hEnemy)
	{
		if (!m_hEnemy->IsAlive())
			m_hEnemy = nullptr;
	}

	if (gpGlobals->time > m_flLastSeen + 15.0 && nullptr != m_hEnemy && (pev->origin - m_hEnemy->pev->origin).Length2D() > 700.0)
	{
		m_hEnemy = nullptr;
	}

	if (!m_hEnemy)
	{
		Look(4096);
		m_hEnemy = BestVisibleEnemy();
	}

	if (m_fGetMad)
	{
		pev->sequence = LookupSequence("mad");
		m_fGetMad = false;
		return;
	}

	if (m_fRightEyeHit && m_fLeftEyeHit)
	{
		if (gpGlobals->time <= m_flOrificeOpenTime && !m_fOrificeHit)
		{
			pev->sequence = LookupSequence("bigpain2");
			EmitSound(CHAN_VOICE, "geneworm/geneworm_final_pain2.wav", VOL_NORM, 0.1);
			return;
		}

		m_flOrificeOpenTime = gpGlobals->time;

		if (!m_fSpawningTrooper)
		{
			pev->sequence = LookupSequence("bigpain4");
			EmitSound(CHAN_VOICE, "geneworm/geneworm_final_pain4.wav", VOL_NORM, 0.1);
			m_fSpawningTrooper = true;
			return;
		}

		m_fLeftEyeHit = false;
		m_fRightEyeHit = false;
		m_fOrificeHit = false;
		pev->skin = 0;
		m_fSpawningTrooper = false;
	}

	if (m_hEnemy)
	{
		if (ClawAttack())
			return;

		if (m_iHitTimes > 1 && gpGlobals->time > m_flMadDelayTime && !RANDOM_LONG(0, m_iMaxHitTimes - m_iHitTimes) && m_hEnemy && FVisible(m_hEnemy))
		{
			pev->sequence = LookupSequence("mad");
			m_flMadDelayTime = gpGlobals->time + 15.0;
			return;
		}
	}

	switch (m_iHitTimes)
	{
	case 0:
		pev->sequence = LookupSequence("idlepain");
		break;
	case 1:
		pev->sequence = LookupSequence("idlepain2");
		break;
	case 2:
		pev->sequence = LookupSequence("idlepain3");
		break;
	default:
		break;
	}

	EmitSoundDyn(CHAN_BODY, pIdleSounds[RANDOM_LONG(0, std::size(pIdleSounds) - 1)], VOL_NORM, 0.1, 0, RANDOM_LONG(-5, 5) + 100);
}

bool COFGeneWorm::ClawAttack()
{
	auto pEnemy = m_hEnemy.Entity<CBaseEntity>();

	if (pEnemy)
	{
		if (m_flNextMeleeTime <= gpGlobals->time)
		{
			m_posTarget = pEnemy->pev->origin;

			if (FVisible(m_posTarget))
			{
				const auto targetAngle = UTIL_VecToAngles((m_posTarget - pev->origin).Normalize());

				const auto yawDelta = UTIL_AngleDiff(targetAngle.y, pev->angles.y);

				if (gpGlobals->time > m_flNextRangeTime)
				{
					// TODO: never used?
					Vector vecMouthPos, vecMouthAngle;
					GetAttachment(0, vecMouthPos, vecMouthAngle);

					if (yawDelta >= 10.0)
						pev->sequence = LookupSequence("dattack2");
					else if (yawDelta > -10.0)
						pev->sequence = LookupSequence("dattack1");
					else
						pev->sequence = LookupSequence("dattack3");

					EmitSound(CHAN_VOICE, "geneworm/geneworm_beam_attack.wav", VOL_NORM, 0.1);

					m_flNextRangeTime = RANDOM_FLOAT(10, 15) + gpGlobals->time;

					m_flNextMeleeTime = RANDOM_FLOAT(3, 5) + gpGlobals->time;
					return true;
				}

				if ((pev->origin - pEnemy->pev->origin).Length2D() < 1280)
				{
					if (m_posTarget.z <= pev->origin.z)
					{
						pev->sequence = LookupSequence("melee3");
					}
					else if (yawDelta >= 10.0)
					{
						pev->sequence = LookupSequence("melee1");
						EmitSound(CHAN_VOICE, "geneworm/geneworm_attack_mounted_rocket.wav", VOL_NORM, 0.1);
					}
					else if (yawDelta > -2.0)
					{
						pev->sequence = LookupSequence("melee3");
						EmitSound(CHAN_VOICE, "geneworm/geneworm_big_attack_forward.wav", VOL_NORM, 0.1);
					}
					else
					{
						pev->sequence = LookupSequence("melee2");
						EmitSound(CHAN_VOICE, "geneworm/geneworm_attack_mounted_gun.wav", VOL_NORM, 0.1);
					}

					m_flNextMeleeTime = RANDOM_FLOAT(3, 5) + gpGlobals->time;
					return true;
				}
			}
		}
	}

	return false;
}

bool COFGeneWorm::TakeDamage(CBaseEntity* inflictor, CBaseEntity* attacker, float flDamage, int bitsDamageType)
{
	// Never actually die
	if (flDamage >= pev->health)
	{
		pev->health = 1;
	}
	else
	{
		pev->health -= flDamage;
	}

	return true;
}

void COFGeneWorm::TraceAttack(CBaseEntity* attacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType)
{
	const auto isLaser = 0 == strcmp("env_laser", STRING(attacker->pev->classname));

	if (ptr->iHitgroup != 4 && ptr->iHitgroup != 5 && ptr->iHitgroup != 6)
	{
		if (pev->dmgtime != gpGlobals->time || RANDOM_LONG(0, 10) <= 0)
		{
			if (isLaser)
			{
				UTIL_Sparks(ptr->vecEndPos);
			}
			else if ((bitsDamageType & DMG_BULLET) != 0)
			{
				UTIL_Ricochet(ptr->vecEndPos, RANDOM_FLOAT(1, 2));
			}

			pev->dmgtime = gpGlobals->time;
		}

		return;
	}

	if (gpGlobals->time <= m_flTakeHitTime)
		return;

	auto skipChecks = false;

	if (!isLaser)
	{
		if (ptr->iHitgroup != 6)
		{
			if (gpGlobals->time != pev->dmgtime || RANDOM_LONG(0, 10) <= 0)
			{
				if ((bitsDamageType & DMG_BULLET) != 0)
				{
					UTIL_Ricochet(ptr->vecEndPos, RANDOM_FLOAT(1, 2));
				}
				pev->dmgtime = gpGlobals->time;
			}

			return;
		}

		skipChecks = true;
	}

	if (!skipChecks)
	{
		if (!m_fHasEntered)
		{
			UTIL_Sparks(ptr->vecEndPos);
			return;
		}

		if (m_fLeftEyeHit && m_fRightEyeHit || m_fGetMad)
			return;
	}

	switch (ptr->iHitgroup)
	{
	case 4:
	{
		if (!m_fLeftEyeHit)
		{
			if (0 == strcmp("left_eye_laser", STRING(attacker->pev->targetname)))
			{
				m_fLeftEyeHit = true;

				if (m_fRightEyeHit)
				{
					pev->skin = 3;
					m_flOrificeOpenTime = gpGlobals->time + 20.0;
				}
				else
				{
					pev->skin = 1;
					m_fGetMad = true;
				}

				m_iWasHit = 1;

				if (m_bloodColor != DONT_BLEED)
				{
					SpawnBlood(ptr->vecEndPos - vecDir * 4, m_bloodColor, 300.0);
					TraceBleed(flDamage, vecDir, ptr, bitsDamageType);
				}
				break;
			}
		}

		UTIL_Sparks(ptr->vecEndPos);
		break;
	}

	case 5:
	{
		if (!m_fRightEyeHit)
		{
			if (0 == strcmp("right_eye_laser", STRING(attacker->pev->targetname)))
			{
				m_fRightEyeHit = true;

				if (m_fLeftEyeHit)
				{
					pev->skin = 3;
					m_flOrificeOpenTime = gpGlobals->time + 20.0;
				}
				else
				{
					m_fGetMad = true;
					pev->skin = 2;
				}

				m_iWasHit = 1;

				if (m_bloodColor != DONT_BLEED)
				{
					SpawnBlood(ptr->vecEndPos - vecDir * 4, m_bloodColor, 300.0);
					TraceBleed(flDamage, vecDir, ptr, bitsDamageType);
				}
				break;
			}
		}
		UTIL_Sparks(ptr->vecEndPos);
		break;
	}

	case 6:
	{
		if (m_flOrificeOpenTime > gpGlobals->time && !m_fOrificeHit)
		{
			pev->health -= flDamage;

			if (pev->health <= 0)
			{
				m_fOrificeHit = true;

				UTIL_BloodDecalTrace(ptr, m_bloodColor);

				++m_iHitTimes;

				if (m_iHitTimes >= m_iMaxHitTimes)
				{
					m_iWasHit = 2;
				}
				else
				{
					m_iWasHit = 1;
				}

				pev->health = pev->max_health;
			}
		}
		break;
	}

	default:
		break;
	}
}

bool COFGeneWorm::FVisible(CBaseEntity* pEntity)
{
	if ((pEntity->pev->flags & FL_NOTARGET) == 0)
	{
		if ((pev->waterlevel != WaterLevel::Head && pEntity->pev->waterlevel != WaterLevel::Head) || WaterLevel::Dry != pEntity->pev->waterlevel)
		{
			return FVisible(pEntity->EyePosition());
		}
	}

	return false;
}

bool COFGeneWorm::FVisible(const Vector& vecOrigin)
{
	Vector vecLookerOrigin, vecLookerAngle;
	GetAttachment(0, vecLookerOrigin, vecLookerAngle);

	TraceResult tr;
	UTIL_TraceLine(vecLookerOrigin, vecOrigin, ignore_monsters, ignore_glass, edict(), &tr);

	return tr.flFraction == 1.0;
}

void FireHurtTargets(const char* targetName, CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!targetName)
		return;

	CBaseMonster::AILogger->debug("Firing: ({})", targetName);

	for (auto target : UTIL_FindEntitiesByTargetname(targetName))
	{
		// Fire only those targets that were toggled by the last hurt event
		if (target && !(useType == USE_OFF && target->pev->solid == SOLID_NOT) && !(useType == USE_ON && target->pev->solid == SOLID_TRIGGER) && (target->pev->flags & FL_KILLME) == 0) // Don't use dying ents
		{
			CBaseMonster::AILogger->debug("Found: {}, firing ({})", STRING(target->pev->classname), targetName);
			target->Use(pActivator, pCaller, useType, value);
		}
	}
}

void COFGeneWorm::HandleAnimEvent(MonsterEvent_t* pEvent)
{
	switch (pEvent->event)
	{
	case AE_GENEWORM_SPIT_START:
		m_fSpitting = true;
		m_flSpitStartTime = gpGlobals->time;
		break;

	case AE_GENEWORM_LAUNCH_SPAWN:
		if (m_orificeGlow)
		{
			Vector vecPos, vecAng;
			GetAttachment(1, vecPos, vecAng);
			UTIL_MakeVectors(pev->angles);

			EmitSoundDyn(CHAN_WEAPON, pSpawnSounds[RANDOM_LONG(0, std::size(pSpawnSounds) - 1)], VOL_NORM, 0.1, 0, RANDOM_LONG(-5, 5) + 100);

			m_orificeGlow->LaunchSpawn(vecPos, gpGlobals->v_forward, RANDOM_LONG(0, 50) + 300, edict(), 2);

			m_orificeGlow = nullptr;
		}
		break;

	case AE_GENEWORM_SLASH_LEFT_ON:
		FireHurtTargets("GeneWormLeftSlash", this, this, USE_ON, 0);
		break;

	case AE_GENEWORM_SLASH_LEFT_OFF:
		FireHurtTargets("GeneWormLeftSlash", this, this, USE_OFF, 0);
		break;

	case AE_GENEWORM_SLASH_RIGHT_ON:
		FireHurtTargets("GeneWormRightSlash", this, this, USE_ON, 0);
		break;

	case AE_GENEWORM_SLASH_RIGHT_OFF:
		FireHurtTargets("GeneWormRightSlash", this, this, USE_OFF, 0);
		break;

	case AE_GENEWORM_SLASH_CENTER_ON:
		FireHurtTargets("GeneWormCenterSlash", this, this, USE_ON, 0);
		break;

	case AE_GENEWORM_SLASH_CENTER_OFF:
		FireHurtTargets("GeneWormCenterSlash", this, this, USE_OFF, 0);
		break;

	case AE_GENEWORM_HIT_WALL:
		FireTargets("GeneWormWallHit", this, this, USE_TOGGLE, 0);
		UTIL_ScreenShake(pev->origin, 24, 3, 5, 2048);
		break;

	default:
		break;
	}
}

void COFGeneWorm::TrackHead()
{
	Vector vecMouthPos, vecMouthAngle;
	GetAttachment(0, vecMouthPos, vecMouthAngle);

	const auto angles = UTIL_VecToAngles((m_posTarget - vecMouthPos).Normalize());

	const auto yawDelta = UTIL_AngleDiff(angles.y, pev->angles.y);

	const auto yawAngle = std::clamp(yawDelta, -30.f, 30.f);

	SetBoneController(0, yawAngle);
}

void COFGeneWorm::SpewCloud()
{
	// Not much to do here, probably never finished
	UTIL_MakeVectors(pev->angles);
}
