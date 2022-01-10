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
// CSquadMonster - for any monster that forms squads.
//=========================================================
class COFSquadTalkMonster : public CTalkMonster
{
public:
	// squad leader info
	EHANDLE	m_hSquadLeader;		// who is my leader
	EHANDLE	m_hSquadMember[MAX_SQUAD_MEMBERS - 1];	// valid only for leader
	int		m_afSquadSlots;
	float	m_flLastEnemySightTime; // last time anyone in the squad saw the enemy
	bool	m_fEnemyEluded;

	EHANDLE m_hWaitMedic;
	float m_flMedicWaitTime;
	float m_flLastHitByPlayer;
	int m_iPlayerHits;

	// squad member info
	int		m_iMySlot;// this is the behaviour slot that the monster currently holds in the squad. 

	bool CheckEnemy(CBaseEntity* pEnemy) override;
	void StartMonster() override;
	void VacateSlot();
	void ScheduleChange() override;
	void Killed(entvars_t* pevAttacker, int iGib) override;
	bool OccupySlot(int iDesiredSlot);
	bool NoFriendlyFire();

	// squad functions still left in base class
	COFSquadTalkMonster* MySquadLeader()
	{
		COFSquadTalkMonster* pSquadLeader = (COFSquadTalkMonster*)((CBaseEntity*)m_hSquadLeader);
		if (pSquadLeader != NULL)
			return pSquadLeader;
		return this;
	}
	COFSquadTalkMonster* MySquadMember(int i)
	{
		if (i >= MAX_SQUAD_MEMBERS - 1)
			return this;
		else
			return (COFSquadTalkMonster*)((CBaseEntity*)m_hSquadMember[i]);
	}
	bool InSquad() { return m_hSquadLeader != NULL; }
	bool IsLeader() { return m_hSquadLeader == this; }
	int SquadJoin(int searchRadius);
	int SquadRecruit(int searchRadius, int maxMembers);
	int	SquadCount();
	void SquadRemove(COFSquadTalkMonster* pRemove);
	void SquadUnlink();
	bool SquadAdd(COFSquadTalkMonster* pAdd);
	void SquadDisband();
	void SquadAddConditions(int iConditions);
	void SquadMakeEnemy(CBaseEntity* pEnemy);
	void SquadPasteEnemyInfo();
	void SquadCopyEnemyInfo();
	bool SquadEnemySplit();
	bool SquadMemberInRange(const Vector& vecLocation, float flDist);

	COFSquadTalkMonster* MySquadTalkMonsterPointer() override { return this; }

	static TYPEDESCRIPTION m_SaveData[];

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	bool FValidateCover(const Vector& vecCoverLocation) override;

	MONSTERSTATE GetIdealState() override;
	Schedule_t* GetScheduleOfType(int iType) override;

	void EXPORT FollowerUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	COFSquadTalkMonster* MySquadMedic();

	COFSquadTalkMonster* FindSquadMedic(int searchRadius);

	virtual bool HealMe(COFSquadTalkMonster* pTarget);

	bool TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;
};

