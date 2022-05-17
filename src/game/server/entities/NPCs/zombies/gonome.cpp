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
//=========================================================
// Zombie
//=========================================================

// UNDONE: Don't flinch every time you get hit

#include "cbase.h"
#include "zombie.h"

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define ZOMBIE_AE_ATTACK_GUTS_GRAB 0x03
#define ZOMBIE_AE_ATTACK_GUTS_THROW 4
#define GONOME_AE_ATTACK_BITE_FIRST 19
#define GONOME_AE_ATTACK_BITE_SECOND 20
#define GONOME_AE_ATTACK_BITE_THIRD 21
#define GONOME_AE_ATTACK_BITE_FINISH 22

class COFGonomeGuts : public CBaseEntity
{
public:
	using BaseClass = CBaseEntity;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

	void Spawn() override;

	void Touch(CBaseEntity* pOther) override;

	void EXPORT Animate();

	static void Shoot(entvars_t* pevOwner, Vector vecStart, Vector vecVelocity);

	static COFGonomeGuts* GonomeGutsCreate(const Vector& origin);

	void Launch(entvars_t* pevOwner, Vector vecStart, Vector vecVelocity);

	int m_maxFrame;
};

TYPEDESCRIPTION COFGonomeGuts::m_SaveData[] =
	{
		DEFINE_FIELD(COFGonomeGuts, m_maxFrame, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(COFGonomeGuts, COFGonomeGuts::BaseClass);

LINK_ENTITY_TO_CLASS(gonomeguts, COFGonomeGuts);

void COFGonomeGuts::Spawn()
{
	pev->movetype = MOVETYPE_FLY;
	pev->classname = MAKE_STRING("gonomeguts");

	pev->solid = SOLID_BBOX;
	pev->rendermode = kRenderTransAlpha;
	pev->renderamt = 255;

	//TODO: probably shouldn't be assinging to x every time
	if (g_Language == LANGUAGE_GERMAN)
	{
		SetModel("sprites/bigspit.spr");
		pev->rendercolor.x = 0;
		pev->rendercolor.x = 255;
		pev->rendercolor.x = 0;
	}
	else
	{
		SetModel("sprites/bigspit.spr");
		pev->rendercolor.x = 128;
		pev->rendercolor.x = 32;
		pev->rendercolor.x = 128;
	}

	pev->frame = 0;
	pev->scale = 0.5;

	UTIL_SetSize(pev, g_vecZero, g_vecZero);

	m_maxFrame = static_cast<int>(MODEL_FRAMES(pev->modelindex) - 1);
}

void COFGonomeGuts::Touch(CBaseEntity* pOther)
{
	// splat sound
	const auto iPitch = RANDOM_FLOAT(90, 110);

	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "bullchicken/bc_acid1.wav", 1, ATTN_NORM, 0, iPitch);

	switch (RANDOM_LONG(0, 1))
	{
	case 0:
		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "bullchicken/bc_spithit1.wav", 1, ATTN_NORM, 0, iPitch);
		break;
	case 1:
		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "bullchicken/bc_spithit2.wav", 1, ATTN_NORM, 0, iPitch);
		break;
	}

	if (0 == pOther->pev->takedamage)
	{
		TraceResult tr;
		// make a splat on the wall
		UTIL_TraceLine(pev->origin, pev->origin + pev->velocity * 10, dont_ignore_monsters, ENT(pev), &tr);
		UTIL_BloodDecalTrace(&tr, BLOOD_COLOR_RED);
		UTIL_BloodDrips(tr.vecEndPos, tr.vecPlaneNormal, BLOOD_COLOR_RED, 35);
	}
	else
	{
		pOther->TakeDamage(pev, pev, GetSkillFloat("gonome_dmg_guts"sv), DMG_GENERIC);
	}

	SetThink(&COFGonomeGuts::SUB_Remove);
	pev->nextthink = gpGlobals->time;
}

void COFGonomeGuts::Animate()
{
	pev->nextthink = gpGlobals->time + 0.1;

	if (0 != pev->frame++)
	{
		if (pev->frame > m_maxFrame)
		{
			pev->frame = 0;
		}
	}
}

void COFGonomeGuts::Shoot(entvars_t* pevOwner, Vector vecStart, Vector vecVelocity)
{
	auto pGuts = GetClassPtr<COFGonomeGuts>(nullptr);
	pGuts->Spawn();

	UTIL_SetOrigin(pGuts->pev, vecStart);
	pGuts->pev->velocity = vecVelocity;
	pGuts->pev->owner = ENT(pevOwner);

	if (pGuts->m_maxFrame > 0)
	{
		pGuts->SetThink(&COFGonomeGuts::Animate);
		pGuts->pev->nextthink = gpGlobals->time + 0.1;
	}
}

COFGonomeGuts* COFGonomeGuts::GonomeGutsCreate(const Vector& origin)
{
	auto pGuts = GetClassPtr<COFGonomeGuts>(nullptr);
	pGuts->Spawn();

	pGuts->pev->origin = origin;

	return pGuts;
}

void COFGonomeGuts::Launch(entvars_t* pevOwner, Vector vecStart, Vector vecVelocity)
{
	UTIL_SetOrigin(pev, vecStart);
	pev->velocity = vecVelocity;
	pev->owner = ENT(pevOwner);

	SetThink(&COFGonomeGuts::Animate);
	pev->nextthink = gpGlobals->time + 0.1;
}

enum
{
	TASK_GONOME_GET_PATH_TO_ENEMY_CORPSE = LAST_COMMON_TASK + 1,
};


class COFGonome : public CZombie
{
public:
	using BaseClass = CZombie;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

	void OnCreate() override;
	void Spawn() override;
	void Precache() override;
	void HandleAnimEvent(MonsterEvent_t* pEvent) override;
	int IgnoreConditions() override;

	void PainSound() override;
	void IdleSound() override;

	static constexpr const char* pIdleSounds[] =
		{
			"gonome/gonome_idle1.wav",
			"gonome/gonome_idle2.wav",
			"gonome/gonome_idle3.wav",
		};

	static constexpr const char* pPainSounds[] =
		{
			"gonome/gonome_pain1.wav",
			"gonome/gonome_pain2.wav",
			"gonome/gonome_pain3.wav",
			"gonome/gonome_pain4.wav",
		};

	bool CheckRangeAttack1(float flDot, float flDist) override;

	bool CheckMeleeAttack1(float flDot, float flDist) override;

	Schedule_t* GetScheduleOfType(int Type) override;

	void Killed(entvars_t* pevAttacker, int iGib) override;

	void StartTask(Task_t* pTask) override;

	void SetActivity(Activity NewActivity) override;

	CUSTOM_SCHEDULES;

	float m_flNextThrowTime = 0;

	//TODO: needs to be EHANDLE, save/restored or a save during a windup will cause problems
	COFGonomeGuts* m_pGonomeGuts = nullptr;
	bool m_fPlayerLocked = false;

protected:
	float GetOneSlashDamage() override { return GetSkillFloat("gonome_dmg_one_slash"sv); }
	float GetBothSlashDamage() override { return 0; } //Not used, so just return 0

	// Take 15% damage from bullets
	virtual float GetBulletDamageFraction() const override { return 0.15f; }
};

TYPEDESCRIPTION COFGonome::m_SaveData[] =
	{
		DEFINE_FIELD(COFGonome, m_flNextThrowTime, FIELD_TIME),
		DEFINE_FIELD(COFGonome, m_fPlayerLocked, FIELD_BOOLEAN),
};

IMPLEMENT_SAVERESTORE(COFGonome, COFGonome::BaseClass);

LINK_ENTITY_TO_CLASS(monster_gonome, COFGonome);

Task_t tlGonomeVictoryDance[] =
	{
		{TASK_STOP_MOVING, 0},
		{TASK_SET_FAIL_SCHEDULE, SCHED_IDLE_STAND},
		{TASK_WAIT, 0.2},
		{TASK_GONOME_GET_PATH_TO_ENEMY_CORPSE, 0},
		{TASK_WALK_PATH, 0},
		{TASK_WAIT_FOR_MOVEMENT, 0},
		{TASK_FACE_ENEMY, 0},
		{TASK_PLAY_SEQUENCE, ACT_VICTORY_DANCE},
		{TASK_PLAY_SEQUENCE, ACT_VICTORY_DANCE},
		{TASK_PLAY_SEQUENCE, ACT_VICTORY_DANCE},
		{TASK_PLAY_SEQUENCE, ACT_VICTORY_DANCE},
		{TASK_PLAY_SEQUENCE, ACT_VICTORY_DANCE},
		{TASK_PLAY_SEQUENCE, ACT_VICTORY_DANCE},
		{TASK_PLAY_SEQUENCE, ACT_VICTORY_DANCE},
};

Schedule_t slGonomeVictoryDance[] =
	{
		{
			tlGonomeVictoryDance,
			std::size(tlGonomeVictoryDance),
			bits_COND_NEW_ENEMY |
				bits_COND_LIGHT_DAMAGE |
				bits_COND_HEAVY_DAMAGE,
			bits_SOUND_NONE,
			"BabyVoltigoreVictoryDance" //Yup, it's a copy
		},
};

DEFINE_CUSTOM_SCHEDULES(COFGonome){
	slGonomeVictoryDance,
};

//TODO: need to use CZombie instead of CBaseMonster
IMPLEMENT_CUSTOM_SCHEDULES(COFGonome, CBaseMonster);

void COFGonome::OnCreate()
{
	CZombie::OnCreate();

	pev->health = GetSkillFloat("gonome_health"sv);
	pev->model = MAKE_STRING("models/gonome.mdl");
}

void COFGonome::PainSound()
{
	int pitch = 95 + RANDOM_LONG(0, 9);

	if (RANDOM_LONG(0, 5) < 2)
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pPainSounds), 1.0, ATTN_NORM, 0, pitch);
}

void COFGonome::IdleSound()
{
	int pitch = 100 + RANDOM_LONG(-5, 5);

	// Play a random idle sound
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pIdleSounds), 1.0, ATTN_NORM, 0, pitch);
}

//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void COFGonome::HandleAnimEvent(MonsterEvent_t* pEvent)
{
	switch (pEvent->event)
	{
	case ZOMBIE_AE_ATTACK_RIGHT:
		ZombieSlashAttack(GetOneSlashDamage(), {5, 0, -9}, -(gpGlobals->v_right * 25), false);
		break;

	case ZOMBIE_AE_ATTACK_LEFT:
		ZombieSlashAttack(GetOneSlashDamage(), {5, 0, 9}, gpGlobals->v_right * 25, false);
		break;

	case ZOMBIE_AE_ATTACK_GUTS_GRAB:
	{
		//Only if we still have an enemy at this point
		if (m_hEnemy)
		{
			Vector vecGutsPos, vecGutsAngles;
			GetAttachment(0, vecGutsPos, vecGutsAngles);

			if (!m_pGonomeGuts)
			{
				m_pGonomeGuts = COFGonomeGuts::GonomeGutsCreate(vecGutsPos);
			}

			//Attach to hand for throwing
			m_pGonomeGuts->pev->skin = entindex();
			m_pGonomeGuts->pev->body = 1;
			m_pGonomeGuts->pev->aiment = edict();
			m_pGonomeGuts->pev->movetype = MOVETYPE_FOLLOW;

			auto direction = (m_hEnemy->pev->origin + m_hEnemy->pev->view_ofs - vecGutsPos).Normalize();

			direction = direction + Vector(
										RANDOM_FLOAT(-0.05, 0.05),
										RANDOM_FLOAT(-0.05, 0.05),
										RANDOM_FLOAT(-0.05, 0));

			UTIL_BloodDrips(vecGutsPos, direction, BLOOD_COLOR_RED, 35);
		}
	}
	break;

	case ZOMBIE_AE_ATTACK_GUTS_THROW:
	{
		//Note: this check wasn't in the original. If an enemy dies during gut throw windup, this can be null and crash
		if (m_hEnemy)
		{
			Vector vecGutsPos, vecGutsAngles;
			GetAttachment(0, vecGutsPos, vecGutsAngles);

			UTIL_MakeVectors(pev->angles);

			if (!m_pGonomeGuts)
			{
				m_pGonomeGuts = COFGonomeGuts::GonomeGutsCreate(vecGutsPos);
			}

			auto direction = (m_hEnemy->pev->origin + m_hEnemy->pev->view_ofs - vecGutsPos).Normalize();

			direction = direction + Vector(
										RANDOM_FLOAT(-0.05, 0.05),
										RANDOM_FLOAT(-0.05, 0.05),
										RANDOM_FLOAT(-0.05, 0));

			UTIL_BloodDrips(vecGutsPos, direction, BLOOD_COLOR_RED, 35);

			//Detach from owner
			m_pGonomeGuts->pev->skin = 0;
			m_pGonomeGuts->pev->body = 0;
			m_pGonomeGuts->pev->aiment = nullptr;
			m_pGonomeGuts->pev->movetype = MOVETYPE_FLY;

			m_pGonomeGuts->Launch(pev, vecGutsPos, direction * 900);
		}
		else
		{
			UTIL_Remove(m_pGonomeGuts);
		}

		m_pGonomeGuts = nullptr;
	}
	break;

	case GONOME_AE_ATTACK_BITE_FIRST:
	case GONOME_AE_ATTACK_BITE_SECOND:
	case GONOME_AE_ATTACK_BITE_THIRD:
	{
		//TODO: this doesn't check if the enemy is the player, can cause bugs
		if ((pev->origin - m_hEnemy->pev->origin).Length() < 48)
		{
			//TODO: not suited for multiplayer
			auto pPlayer = static_cast<CBasePlayer*>(UTIL_FindEntityByClassname(nullptr, "player"));

			if (pPlayer && pPlayer->IsAlive())
				pPlayer->EnableControl(false);

			m_fPlayerLocked = true;
		}

		// do stuff for this event.
		//		ALERT( at_console, "Slash left!\n" );
		CBaseEntity* pHurt = CheckTraceHullAttack(70, GetSkillFloat("gonome_dmg_one_bite"sv), DMG_SLASH);
		if (pHurt)
		{
			if ((pHurt->pev->flags & (FL_MONSTER | FL_CLIENT)) != 0)
			{
				pHurt->pev->punchangle.x = 9;
				pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_forward * 25;
			}
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pAttackHitSounds), 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));
		}
		else
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pAttackMissSounds), 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));
	}
	break;

	case GONOME_AE_ATTACK_BITE_FINISH:
	{
		auto pPlayer = static_cast<CBasePlayer*>(UTIL_FindEntityByClassname(nullptr, "player"));

		if (pPlayer && pPlayer->IsAlive())
		{
			pPlayer->EnableControl(true);
		}

		m_fPlayerLocked = false;

		// do stuff for this event.
		//		ALERT( at_console, "Slash left!\n" );
		CBaseEntity* pHurt = CheckTraceHullAttack(70, GetSkillFloat("gonome_dmg_one_bite"sv), DMG_SLASH);
		if (pHurt)
		{
			if ((pHurt->pev->flags & (FL_MONSTER | FL_CLIENT)) != 0)
			{
				pHurt->pev->punchangle.x = 9;
				pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_forward * 25;
			}
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pAttackHitSounds), 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));
		}
		else
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pAttackMissSounds), 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));
	}
	break;

	default:
		CBaseMonster::HandleAnimEvent(pEvent);
		break;
	}
}

//=========================================================
// Spawn
//=========================================================
void COFGonome::Spawn()
{
	m_flNextThrowTime = gpGlobals->time;
	m_pGonomeGuts = nullptr;
	m_fPlayerLocked = false;

	CZombie::Spawn();
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void COFGonome::Precache()
{
	//Don't call CZombie::Spawn here!
	PrecacheModel(STRING(pev->model));
	PrecacheModel("sprites/bigspit.spr");

	PRECACHE_SOUND_ARRAY(pAttackHitSounds);
	PRECACHE_SOUND_ARRAY(pAttackMissSounds);
	PRECACHE_SOUND_ARRAY(pIdleSounds);
	PRECACHE_SOUND_ARRAY(pAlertSounds);
	PRECACHE_SOUND_ARRAY(pPainSounds);

	PRECACHE_SOUND("gonome/gonome_death2.wav");
	PRECACHE_SOUND("gonome/gonome_death3.wav");
	PRECACHE_SOUND("gonome/gonome_death4.wav");

	PRECACHE_SOUND("gonome/gonome_jumpattack.wav");

	PRECACHE_SOUND("gonome/gonome_melee1.wav");
	PRECACHE_SOUND("gonome/gonome_melee2.wav");

	PRECACHE_SOUND("gonome/gonome_run.wav");
	PRECACHE_SOUND("gonome/gonome_eat.wav");

	PRECACHE_SOUND("bullchicken/bc_acid1.wav");
	PRECACHE_SOUND("bullchicken/bc_spithit1.wav");
	PRECACHE_SOUND("bullchicken/bc_spithit2.wav");
}

//=========================================================
// AI Schedules Specific to this monster
//=========================================================



int COFGonome::IgnoreConditions()
{
	int iIgnore = CBaseMonster::IgnoreConditions();

	if (m_Activity == ACT_RANGE_ATTACK1)
	{
		iIgnore |= bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE | bits_COND_ENEMY_TOOFAR | bits_COND_ENEMY_OCCLUDED;
	}
	else if (m_Activity == ACT_MELEE_ATTACK1)
	{
#if 0
		if (pev->health < 20)
			iIgnore |= (bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE);
		else
#endif
		if (m_flNextFlinch >= gpGlobals->time)
			iIgnore |= (bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE);
	}

	if ((m_Activity == ACT_SMALL_FLINCH) || (m_Activity == ACT_BIG_FLINCH))
	{
		if (m_flNextFlinch < gpGlobals->time)
			m_flNextFlinch = gpGlobals->time + ZOMBIE_FLINCH_DELAY;
	}

	return iIgnore;
}

bool COFGonome::CheckMeleeAttack1(float flDot, float flDist)
{
	if (flDist <= 64.0 && flDot >= 0.7 && m_hEnemy)
	{
		return (m_hEnemy->pev->flags & FL_ONGROUND) != 0;
	}

	return false;
}

bool COFGonome::CheckRangeAttack1(float flDot, float flDist)
{
	if (flDist < 256.0)
		return false;

	if (IsMoving() && flDist >= 512.0)
	{
		return false;
	}

	if (flDist > 64.0 && flDist <= 784.0 && flDot >= 0.5 && gpGlobals->time >= m_flNextThrowTime)
	{
		if (!m_hEnemy || (fabs(pev->origin.z - m_hEnemy->pev->origin.z) <= 256.0))
		{
			if (IsMoving())
			{
				m_flNextThrowTime = gpGlobals->time + 5.0;
			}
			else
			{
				m_flNextThrowTime = gpGlobals->time + 0.5;
			}

			return true;
		}
	}

	return false;
}

Schedule_t* COFGonome::GetScheduleOfType(int Type)
{
	if (Type == SCHED_VICTORY_DANCE)
		return slGonomeVictoryDance;
	else
		return CBaseMonster::GetScheduleOfType(Type);
}

void COFGonome::Killed(entvars_t* pevAttacker, int iGib)
{
	if (m_pGonomeGuts)
	{
		UTIL_Remove(m_pGonomeGuts);
		m_pGonomeGuts = nullptr;
	}

	if (m_fPlayerLocked)
	{
		//TODO: not suited for multiplayer
		auto pPlayer = static_cast<CBasePlayer*>(UTIL_FindEntityByClassname(nullptr, "player"));

		if (pPlayer && pPlayer->IsAlive())
			pPlayer->EnableControl(true);

		m_fPlayerLocked = false;
	}

	CBaseMonster::Killed(pevAttacker, iGib);
}

void COFGonome::StartTask(Task_t* pTask)
{
	switch (pTask->iTask)
	{
	case TASK_GONOME_GET_PATH_TO_ENEMY_CORPSE:
	{
		if (m_pGonomeGuts)
		{
			UTIL_Remove(m_pGonomeGuts);
			m_pGonomeGuts = nullptr;
		}

		UTIL_MakeVectors(pev->angles);

		if (BuildRoute(m_vecEnemyLKP - 64 * gpGlobals->v_forward, 64, nullptr))
		{
			TaskComplete();
		}
		else
		{
			ALERT(at_aiconsole, "GonomeGetPathToEnemyCorpse failed!!\n");
			TaskFail();
		}
	}
	break;

	default:
		CBaseMonster::StartTask(pTask);
		break;
	}
}

void COFGonome::SetActivity(Activity NewActivity)
{
	int iSequence = ACTIVITY_NOT_AVAILABLE;
	void* pmodel = GET_MODEL_PTR(ENT(pev));

	if (NewActivity != ACT_RANGE_ATTACK1 && m_pGonomeGuts)
	{
		UTIL_Remove(m_pGonomeGuts);
		m_pGonomeGuts = nullptr;
	}

	if (m_fPlayerLocked)
	{
		if (NewActivity != ACT_MELEE_ATTACK1)
		{
			auto pPlayer = static_cast<CBasePlayer*>(UTIL_FindEntityByClassname(nullptr, "player"));

			if (pPlayer && pPlayer->IsAlive())
				pPlayer->EnableControl(true);

			m_fPlayerLocked = false;
		}
	}

	switch (NewActivity)
	{
	case ACT_MELEE_ATTACK1:
		if (m_hEnemy)
		{
			if ((pev->origin - m_hEnemy->pev->origin).Length() >= 48)
			{
				iSequence = LookupSequence("attack1");
			}
			else
			{
				iSequence = LookupSequence("attack2");
			}
		}
		else
		{
			iSequence = LookupActivity(ACT_MELEE_ATTACK1);
		}
		break;
	case ACT_RUN:
		if (m_hEnemy)
		{
			if ((pev->origin - m_hEnemy->pev->origin).Length() <= 512)
			{
				iSequence = LookupSequence("runshort");
			}
			else
			{
				iSequence = LookupSequence("runlong");
			}
		}
		else
		{
			iSequence = LookupActivity(ACT_RUN);
		}
		break;
	default:
		iSequence = LookupActivity(NewActivity);
		break;
	}

	// Set to the desired anim, or default anim if the desired is not present
	if (iSequence > ACTIVITY_NOT_AVAILABLE)
	{
		if (pev->sequence != iSequence || !m_fSequenceLoops)
		{
			pev->frame = 0;
		}

		pev->sequence = iSequence; // Set to the reset anim (if it's there)
		ResetSequenceInfo();
		SetYawSpeed();
	}
	else
	{
		// Not available try to get default anim
		ALERT(at_console, "%s has no sequence for act:%d\n", STRING(pev->classname), NewActivity);
		pev->sequence = 0; // Set to the reset anim (if it's there)
	}

	m_Activity = NewActivity; // Go ahead and set this so it doesn't keep trying when the anim is not present
	m_IdealActivity = NewActivity;
}

//=========================================================
// DEAD GONOME PROP
//=========================================================
class CDeadGonome : public CBaseMonster
{
public:
	void OnCreate() override;
	void Spawn() override;
	int Classify() override { return CLASS_ALIEN_PASSIVE; }

	bool KeyValue(KeyValueData* pkvd) override;

	int m_iPose; // which sequence to display	-- temporary, don't need to save
	static const char* m_szPoses[3];
};

const char* CDeadGonome::m_szPoses[] = {"dead_on_stomach1", "dead_on_back", "dead_on_side"};

void CDeadGonome::OnCreate()
{
	CBaseMonster::OnCreate();

	// Corpses have less health
	pev->health = 8;
	pev->model = MAKE_STRING("models/gonome.mdl");
}

bool CDeadGonome::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "pose"))
	{
		m_iPose = atoi(pkvd->szValue);
		return true;
	}

	return CBaseMonster::KeyValue(pkvd);
}

LINK_ENTITY_TO_CLASS(monster_gonome_dead, CDeadGonome);

//=========================================================
// ********** DeadGonome SPAWN **********
//=========================================================
void CDeadGonome::Spawn()
{
	PrecacheModel(STRING(pev->model));
	SetModel(STRING(pev->model));

	pev->effects = 0;
	pev->sequence = 0;
	m_bloodColor = BLOOD_COLOR_GREEN;

	pev->sequence = LookupSequence(m_szPoses[m_iPose]);

	if (pev->sequence == -1)
	{
		ALERT(at_console, "Dead gonome with bad pose\n");
	}

	MonsterInitDead();
}
