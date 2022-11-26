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

#pragma once

//=========================================================
// monster-specific schedule types
//=========================================================
enum
{
	SCHED_VOLTIGORE_THREAT_DISPLAY = LAST_COMMON_SCHEDULE + 1,
};

//=========================================================
// monster-specific tasks
//=========================================================
enum
{
	TASK_VOLTIGORE_GET_PATH_TO_ENEMY_CORPSE = LAST_COMMON_TASK + 1,
};

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define VOLTIGORE_AE_BOLT1 (1)
#define VOLTIGORE_AE_BOLT2 (2)
#define VOLTIGORE_AE_BOLT3 (3)
#define VOLTIGORE_AE_BOLT4 (4)
#define VOLTIGORE_AE_BOLT5 (5)
// some events are set up in the QC file that aren't recognized by the code yet.
#define VOLTIGORE_AE_PUNCH (6)
#define VOLTIGORE_AE_BITE (7)

#define VOLTIGORE_AE_LEFT_PUNCH (12)
#define VOLTIGORE_AE_RIGHT_PUNCH (13)



#define VOLTIGORE_MELEE_DIST 128

constexpr int VOLTIGORE_BEAM_COUNT = 8;

class COFVoltigore : public CSquadMonster
{
public:
	void OnCreate() override;
	void Spawn() override;
	void Precache() override;
	void SetYawSpeed() override;
	int Classify() override;
	int ISoundMask() override;
	void HandleAnimEvent(MonsterEvent_t* pEvent) override;
	void SetObjectCollisionBox() override
	{
		pev->absmin = pev->origin + Vector(-80, -80, 0);
		pev->absmax = pev->origin + Vector(80, 80, 90);
	}

	Schedule_t* GetSchedule() override;
	Schedule_t* GetScheduleOfType(int Type) override;
	bool FCanCheckAttacks() override;
	bool CheckMeleeAttack1(float flDot, float flDist) override;
	bool CheckRangeAttack1(float flDot, float flDist) override;
	void StartTask(Task_t* pTask) override;
	void RunTask(Task_t* pTask) override;
	void AlertSound() override;
	void PainSound() override;
	void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType) override;
	int IRelationship(CBaseEntity* pTarget) override;
	void StopTalking();
	bool ShouldSpeak();

	void ClearBeams();

	void EXPORT CallDeathGibThink();

	virtual void DeathGibThink();

	void GibMonster() override;

	void Killed(entvars_t* pevAttacker, int iGib) override;

	CUSTOM_SCHEDULES;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];

	static constexpr const char* pAttackHitSounds[] =
		{
			"zombie/claw_strike1.wav",
			"zombie/claw_strike2.wav",
			"zombie/claw_strike3.wav",
		};

	static constexpr const char* pAttackMissSounds[] =
		{
			"zombie/claw_miss1.wav",
			"zombie/claw_miss2.wav",
		};

	static constexpr const char* pAttackSounds[] =
		{
			"voltigore/voltigore_attack_melee1.wav",
			"voltigore/voltigore_attack_melee2.wav",
		};

	static constexpr const char* pPainSounds[] =
		{
			"voltigore/voltigore_pain1.wav",
			"voltigore/voltigore_pain2.wav",
			"voltigore/voltigore_pain3.wav",
			"voltigore/voltigore_pain4.wav",
		};

	static constexpr const char* pAlertSounds[] =
		{
			"voltigore/voltigore_alert1.wav",
			"voltigore/voltigore_alert2.wav",
			"voltigore/voltigore_alert3.wav",
		};

	EHANDLE m_pBeam[VOLTIGORE_BEAM_COUNT];
	int m_iBeams;

	float m_flNextBeamAttackCheck;

	float m_flNextPainTime;

	// three hacky fields for speech stuff. These don't really need to be saved.
	float m_flNextSpeakTime;
	float m_flNextWordTime;
	int m_iLastWord;

	EHANDLE m_pChargedBolt;

	int m_iVoltigoreGibs;
	bool m_fDeathCharge;
	float m_flDeathStartTime;

protected:
	virtual bool CanUseRangeAttacks() const { return true; }
	virtual bool BlowsUpOnDeath() const { return true; }

	virtual float GetMeleeDistance() const { return VOLTIGORE_MELEE_DIST; }

	/**
	 *	@brief Spawns the Voltigore
	 */
	void SpawnCore(const Vector& mins, const Vector& maxs);
};
