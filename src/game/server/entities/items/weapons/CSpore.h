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

enum SporeAnim
{
	SPORE_IDLE = 0,
};

class CSpore : public CGrenade
{
	DECLARE_CLASS(CSpore, CGrenade);
	DECLARE_DATAMAP();

public:
	enum class SporeType
	{
		ROCKET = 1,
		GRENADE = 2
	};

public:
	void Precache() override;

	void Spawn() override;

	void BounceSound() override;

	void EXPORT IgniteThink();

	void EXPORT FlyThink();

	void EXPORT GibThink();

	void EXPORT RocketTouch(CBaseEntity* pOther);

	void EXPORT MyBounceTouch(CBaseEntity* pOther);

	static CSpore* CreateSpore(
		const Vector& vecOrigin, const Vector& vecAngles, CBaseEntity* pOwner,
		SporeType sporeType, bool bIsAI, bool bPuked);

private:
	int m_iBlow;
	int m_iBlowSmall;

	int m_iSpitSprite;
	int m_iTrail;

	SporeType m_SporeType;

	float m_flIgniteTime;
	float m_flSoundDelay;

	bool m_bPuked;
	bool m_bIsAI;

	EHANDLE m_hSprite;
};
