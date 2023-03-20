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

#define HC_AE_JUMPATTACK (2)

Task_t tlHCRangeAttack1[] =
	{
		{TASK_STOP_MOVING, (float)0},
		{TASK_FACE_IDEAL, (float)0},
		{TASK_RANGE_ATTACK1, (float)0},
		{TASK_SET_ACTIVITY, (float)ACT_IDLE},
		{TASK_FACE_IDEAL, (float)0},
		{TASK_WAIT_RANDOM, (float)0.5},
};

Schedule_t slHCRangeAttack1[] =
	{
		{tlHCRangeAttack1,
			std::size(tlHCRangeAttack1),
			bits_COND_ENEMY_OCCLUDED |
				bits_COND_NO_AMMO_LOADED,
			0,
			"HCRangeAttack1"},
};

Task_t tlHCRangeAttack1Fast[] =
	{
		{TASK_STOP_MOVING, (float)0},
		{TASK_FACE_IDEAL, (float)0},
		{TASK_RANGE_ATTACK1, (float)0},
		{TASK_SET_ACTIVITY, (float)ACT_IDLE},
};

Schedule_t slHCRangeAttack1Fast[] =
	{
		{tlHCRangeAttack1Fast,
			std::size(tlHCRangeAttack1Fast),
			bits_COND_ENEMY_OCCLUDED |
				bits_COND_NO_AMMO_LOADED,
			0,
			"HCRAFast"},
};

/**
*	@brief tiny, jumpy alien parasite
*/
class CHeadCrab : public CBaseMonster
{
public:
	void OnCreate() override;
	void Spawn() override;
	void Precache() override;
	void RunTask(Task_t* pTask) override;
	void StartTask(Task_t* pTask) override;
	void SetYawSpeed() override;

	/**
	*	@brief this is the headcrab's touch function when it is in the air
	*/
	void EXPORT LeapTouch(CBaseEntity* pOther);

	/**
	*	@brief returns the real center of the headcrab.
	*	The bounding box is much larger than the actual creature so this is needed for targeting
	*/
	Vector Center() override;
	Vector BodyTarget(const Vector& posSrc) override;
	void PainSound() override;
	void DeathSound() override;
	void IdleSound() override;
	void AlertSound() override;
	void PrescheduleThink() override;
	int Classify() override;
	void HandleAnimEvent(MonsterEvent_t* pEvent) override;
	bool CheckRangeAttack1(float flDot, float flDist) override;
	bool CheckRangeAttack2(float flDot, float flDist) override;
	bool TakeDamage(CBaseEntity* inflictor, CBaseEntity* attacker, float flDamage, int bitsDamageType) override;

	virtual float GetDamageAmount() { return GetSkillFloat("headcrab_dmg_bite"sv); }
	virtual int GetVoicePitch() { return 100; }
	virtual float GetSoundVolue() { return 1.0; }
	Schedule_t* GetScheduleOfType(int Type) override;

	CUSTOM_SCHEDULES;

	static const char* pIdleSounds[];
	static const char* pAlertSounds[];
	static const char* pPainSounds[];
	static const char* pAttackSounds[];
	static const char* pDeathSounds[];
	static const char* pBiteSounds[];
};

LINK_ENTITY_TO_CLASS(monster_headcrab, CHeadCrab);

DEFINE_CUSTOM_SCHEDULES(CHeadCrab){
	slHCRangeAttack1,
	slHCRangeAttack1Fast,
};

IMPLEMENT_CUSTOM_SCHEDULES(CHeadCrab, CBaseMonster);

const char* CHeadCrab::pIdleSounds[] =
	{
		"headcrab/hc_idle1.wav",
		"headcrab/hc_idle2.wav",
		"headcrab/hc_idle3.wav",
};
const char* CHeadCrab::pAlertSounds[] =
	{
		"headcrab/hc_alert1.wav",
};
const char* CHeadCrab::pPainSounds[] =
	{
		"headcrab/hc_pain1.wav",
		"headcrab/hc_pain2.wav",
		"headcrab/hc_pain3.wav",
};
const char* CHeadCrab::pAttackSounds[] =
	{
		"headcrab/hc_attack1.wav",
		"headcrab/hc_attack2.wav",
		"headcrab/hc_attack3.wav",
};

const char* CHeadCrab::pDeathSounds[] =
	{
		"headcrab/hc_die1.wav",
		"headcrab/hc_die2.wav",
};

const char* CHeadCrab::pBiteSounds[] =
	{
		"headcrab/hc_headbite.wav",
};

void CHeadCrab::OnCreate()
{
	CBaseMonster::OnCreate();

	pev->health = GetSkillFloat("headcrab_health"sv);
	pev->model = MAKE_STRING("models/headcrab.mdl");
}

int CHeadCrab::Classify()
{
	return CLASS_ALIEN_PREY;
}

Vector CHeadCrab::Center()
{
	return Vector(pev->origin.x, pev->origin.y, pev->origin.z + 6);
}

Vector CHeadCrab::BodyTarget(const Vector& posSrc)
{
	return Center();
}

void CHeadCrab::SetYawSpeed()
{
	int ys;

	switch (m_Activity)
	{
	case ACT_IDLE:
		ys = 30;
		break;
	case ACT_RUN:
	case ACT_WALK:
		ys = 20;
		break;
	case ACT_TURN_LEFT:
	case ACT_TURN_RIGHT:
		ys = 60;
		break;
	case ACT_RANGE_ATTACK1:
		ys = 30;
		break;
	default:
		ys = 30;
		break;
	}

	pev->yaw_speed = ys;
}

void CHeadCrab::HandleAnimEvent(MonsterEvent_t* pEvent)
{
	switch (pEvent->event)
	{
	case HC_AE_JUMPATTACK:
	{
		ClearBits(pev->flags, FL_ONGROUND);

		UTIL_SetOrigin(pev, pev->origin + Vector(0, 0, 1)); // take him off ground so engine doesn't instantly reset onground
		UTIL_MakeVectors(pev->angles);

		Vector vecJumpDir;
		if (m_hEnemy != nullptr)
		{
			float gravity = g_psv_gravity->value;
			if (gravity <= 1)
				gravity = 1;

			// How fast does the headcrab need to travel to reach that height given gravity?
			float height = (m_hEnemy->pev->origin.z + m_hEnemy->pev->view_ofs.z - pev->origin.z);
			if (height < 16)
				height = 16;
			float speed = sqrt(2 * gravity * height);
			float time = speed / gravity;

			// Scale the sideways velocity to get there at the right time
			vecJumpDir = (m_hEnemy->pev->origin + m_hEnemy->pev->view_ofs - pev->origin);
			vecJumpDir = vecJumpDir * (1.0 / time);

			// Speed to offset gravity at the desired height
			vecJumpDir.z = speed;

			// Don't jump too far/fast
			float distance = vecJumpDir.Length();

			if (distance > 650)
			{
				vecJumpDir = vecJumpDir * (650.0 / distance);
			}
		}
		else
		{
			// jump hop, don't care where
			vecJumpDir = Vector(gpGlobals->v_forward.x, gpGlobals->v_forward.y, gpGlobals->v_up.z) * 350;
		}

		int iSound = RANDOM_LONG(0, 2);
		if (iSound != 0)
			EmitSoundDyn(CHAN_VOICE, pAttackSounds[iSound], GetSoundVolue(), ATTN_IDLE, 0, GetVoicePitch());

		pev->velocity = vecJumpDir;
		m_flNextAttack = gpGlobals->time + 2;
	}
	break;

	default:
		CBaseMonster::HandleAnimEvent(pEvent);
		break;
	}
}

void CHeadCrab::Spawn()
{
	Precache();

	SetModel(STRING(pev->model));
	SetSize(Vector(-12, -12, 0), Vector(12, 12, 24));

	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	m_bloodColor = BLOOD_COLOR_GREEN;
	pev->effects = 0;
	pev->view_ofs = Vector(0, 0, 20); // position of the eyes relative to monster's origin.
	pev->yaw_speed = 5;				  //!!! should we put this in the monster's changeanim function since turn rates may vary with state/anim?
	m_flFieldOfView = 0.5;			  // indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState = MONSTERSTATE_NONE;

	MonsterInit();
}

void CHeadCrab::Precache()
{
	PRECACHE_SOUND_ARRAY(pIdleSounds);
	PRECACHE_SOUND_ARRAY(pAlertSounds);
	PRECACHE_SOUND_ARRAY(pPainSounds);
	PRECACHE_SOUND_ARRAY(pAttackSounds);
	PRECACHE_SOUND_ARRAY(pDeathSounds);
	PRECACHE_SOUND_ARRAY(pBiteSounds);

	PrecacheModel(STRING(pev->model));
}

void CHeadCrab::RunTask(Task_t* pTask)
{
	switch (pTask->iTask)
	{
	case TASK_RANGE_ATTACK1:
	case TASK_RANGE_ATTACK2:
	{
		if (m_fSequenceFinished)
		{
			TaskComplete();
			SetTouch(nullptr);
			m_IdealActivity = ACT_IDLE;
		}
		break;
	}
	default:
	{
		CBaseMonster::RunTask(pTask);
	}
	}
}

void CHeadCrab::LeapTouch(CBaseEntity* pOther)
{
	if (0 == pOther->pev->takedamage)
	{
		return;
	}

	if (pOther->Classify() == Classify())
	{
		return;
	}

	// Don't hit if back on ground
	if (!FBitSet(pev->flags, FL_ONGROUND))
	{
		EmitSoundDyn(CHAN_WEAPON, RANDOM_SOUND_ARRAY(pBiteSounds), GetSoundVolue(), ATTN_IDLE, 0, GetVoicePitch());

		pOther->TakeDamage(this, this, GetDamageAmount(), DMG_SLASH);
	}

	SetTouch(nullptr);
}

void CHeadCrab::PrescheduleThink()
{
	// make the crab coo a little bit in combat state
	if (m_MonsterState == MONSTERSTATE_COMBAT && RANDOM_FLOAT(0, 5) < 0.1)
	{
		IdleSound();
	}
}

void CHeadCrab::StartTask(Task_t* pTask)
{
	m_iTaskStatus = TASKSTATUS_RUNNING;

	switch (pTask->iTask)
	{
	case TASK_RANGE_ATTACK1:
	{
		EmitSoundDyn(CHAN_WEAPON, pAttackSounds[0], GetSoundVolue(), ATTN_IDLE, 0, GetVoicePitch());
		m_IdealActivity = ACT_RANGE_ATTACK1;
		SetTouch(&CHeadCrab::LeapTouch);
		break;
	}
	default:
	{
		CBaseMonster::StartTask(pTask);
	}
	}
}

bool CHeadCrab::CheckRangeAttack1(float flDot, float flDist)
{
	if (FBitSet(pev->flags, FL_ONGROUND) && flDist <= 256 && flDot >= 0.65)
	{
		return true;
	}
	return false;
}

bool CHeadCrab::CheckRangeAttack2(float flDot, float flDist)
{
	return false;
	// BUGBUG: Why is this code here?  There is no ACT_RANGE_ATTACK2 animation.  I've disabled it for now.
#if 0
	if (FBitSet(pev->flags, FL_ONGROUND) && flDist > 64 && flDist <= 256 && flDot >= 0.5)
	{
		return true;
	}
	return false;
#endif
}

bool CHeadCrab::TakeDamage(CBaseEntity* inflictor, CBaseEntity* attacker, float flDamage, int bitsDamageType)
{
	// Don't take any acid damage -- BigMomma's mortar is acid
	if ((bitsDamageType & DMG_ACID) != 0)
		flDamage = 0;

	return CBaseMonster::TakeDamage(inflictor, attacker, flDamage, bitsDamageType);
}

#define CRAB_ATTN_IDLE (float)1.5
void CHeadCrab::IdleSound()
{
	EmitSoundDyn(CHAN_VOICE, RANDOM_SOUND_ARRAY(pIdleSounds), GetSoundVolue(), ATTN_IDLE, 0, GetVoicePitch());
}

void CHeadCrab::AlertSound()
{
	EmitSoundDyn(CHAN_VOICE, RANDOM_SOUND_ARRAY(pAlertSounds), GetSoundVolue(), ATTN_IDLE, 0, GetVoicePitch());
}

void CHeadCrab::PainSound()
{
	EmitSoundDyn(CHAN_VOICE, RANDOM_SOUND_ARRAY(pPainSounds), GetSoundVolue(), ATTN_IDLE, 0, GetVoicePitch());
}

void CHeadCrab::DeathSound()
{
	EmitSoundDyn(CHAN_VOICE, RANDOM_SOUND_ARRAY(pDeathSounds), GetSoundVolue(), ATTN_IDLE, 0, GetVoicePitch());
}

Schedule_t* CHeadCrab::GetScheduleOfType(int Type)
{
	switch (Type)
	{
	case SCHED_RANGE_ATTACK1:
	{
		return &slHCRangeAttack1[0];
	}
	break;
	}

	return CBaseMonster::GetScheduleOfType(Type);
}

class CBabyCrab : public CHeadCrab
{
public:
	void OnCreate() override;
	void Spawn() override;
	void SetYawSpeed() override;
	float GetDamageAmount() override { return GetSkillFloat("headcrab_dmg_bite"sv) * 0.3; }
	bool CheckRangeAttack1(float flDot, float flDist) override;
	Schedule_t* GetScheduleOfType(int Type) override;
	int GetVoicePitch() override { return PITCH_NORM + RANDOM_LONG(40, 50); }
	float GetSoundVolue() override { return 0.8; }
};

LINK_ENTITY_TO_CLASS(monster_babycrab, CBabyCrab);

void CBabyCrab::OnCreate()
{
	CHeadCrab::OnCreate();

	pev->health = GetSkillFloat("headcrab_health"sv) * 0.25; // less health than full grown
	pev->model = MAKE_STRING("models/baby_headcrab.mdl");
}

void CBabyCrab::Spawn()
{
	CHeadCrab::Spawn();
	pev->rendermode = kRenderTransTexture;
	pev->renderamt = 192;
	SetSize(Vector(-12, -12, 0), Vector(12, 12, 24));
}

void CBabyCrab::SetYawSpeed()
{
	pev->yaw_speed = 120;
}

bool CBabyCrab::CheckRangeAttack1(float flDot, float flDist)
{
	if ((pev->flags & FL_ONGROUND) != 0)
	{
		if (pev->groundentity && (pev->groundentity->v.flags & (FL_CLIENT | FL_MONSTER)) != 0)
			return true;

		// A little less accurate, but jump from closer
		if (flDist <= 180 && flDot >= 0.55)
			return true;
	}

	return false;
}

Schedule_t* CBabyCrab::GetScheduleOfType(int Type)
{
	switch (Type)
	{
	case SCHED_FAIL: // If you fail, try to jump!
		if (m_hEnemy != nullptr)
			return slHCRangeAttack1Fast;
		break;

	case SCHED_RANGE_ATTACK1:
	{
		return slHCRangeAttack1Fast;
	}
	break;
	}

	return CHeadCrab::GetScheduleOfType(Type);
}
