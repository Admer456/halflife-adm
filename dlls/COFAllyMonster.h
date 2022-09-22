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

#pragma once

/**
*	@brief Base class for Opposing force allies
*	A copy of CTalkMonster
*/
class COFAllyMonster : public CBaseMonster
{
public:
	void TalkInit();
	CBaseEntity* FindNearestFriend(bool fPlayer);
	float TargetDistance();
	void StopTalking() { SentenceStop(); }

	// Base Monster functions
	void Precache() override;
	bool TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;
	void Touch(CBaseEntity* pOther) override;
	void Killed(entvars_t* pevAttacker, int iGib) override;
	int IRelationship(CBaseEntity* pTarget) override;
	bool CanPlaySentence(bool fDisregardState) override;

protected:
	void PlaySentenceCore(const char* pszSentence, float duration, float volume, float attenuation) override;

public:
	void PlayScriptedSentence(const char* pszSentence, float duration, float volume, float attenuation, bool bConcurrent, CBaseEntity* pListener) override;
	bool KeyValue(KeyValueData* pkvd) override;

	// AI functions
	Schedule_t* GetScheduleOfType(int Type) override;
	void StartTask(Task_t* pTask) override;
	void RunTask(Task_t* pTask) override;
	void HandleAnimEvent(MonsterEvent_t* pEvent) override;
	void PrescheduleThink() override;


	// Conversations / communication
	int GetVoicePitch();
	void IdleRespond();
	bool FIdleSpeak();
	bool FIdleStare();
	bool FIdleHello();
	void IdleHeadTurn(Vector& vecFriend);
	bool FOkToSpeak();
	void TrySmellTalk();
	CBaseEntity* EnumFriends(CBaseEntity* pentPrevious, int listNumber, bool bTrace);
	void AlertFriends();
	void ShutUpFriends();
	bool IsTalking();
	void Talk(float flDuration);
	// For following
	bool CanFollow();
	bool IsFollowing() { return m_hTargetEnt != NULL && m_hTargetEnt->IsPlayer(); }
	void StopFollowing(bool clearSchedule) override;
	void StartFollowing(CBaseEntity* pLeader);
	virtual void DeclineFollowing() {}
	void LimitFollowers(CBaseEntity* pPlayer, int maxFollowers);

	void EXPORT FollowerUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	virtual void SetAnswerQuestion(COFAllyMonster* pSpeaker);
	virtual int FriendNumber(int arrayNumber) { return arrayNumber; }

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;
	static TYPEDESCRIPTION m_SaveData[];


	static const char* m_szFriends[TLK_CFRIENDS]; // array of friend names
	static float g_talkWaitTime;

	int m_bitsSaid;					  // set bits for sentences we don't want repeated
	int m_nSpeak;					  // number of times initiated talking
	int m_voicePitch;				  // pitch of voice for this head
	const char* m_szGrp[TLK_CGROUPS]; // sentence group names
	float m_useTime;				  // Don't allow +USE until this time
	int m_iszUse;					  // Custom +USE sentence group (follow)
	int m_iszUnUse;					  // Custom +USE sentence group (stop following)

	float m_flLastSaidSmelled; // last time we talked about something that stinks
	float m_flStopTalkTime;	   // when in the future that I'll be done saying this sentence.

	EHANDLE m_hTalkTarget; // who to look at while talking
	CUSTOM_SCHEDULES;
};
