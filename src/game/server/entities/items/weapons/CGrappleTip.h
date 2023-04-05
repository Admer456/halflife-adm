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

class CGrappleTip : public CBaseEntity
{
	DECLARE_CLASS(CGrappleTip, CBaseEntity);
	DECLARE_DATAMAP();

public:
	enum class TargetClass
	{
		NOT_A_TARGET = 0,
		SMALL = 1,
		MEDIUM = 2,
		LARGE = 3,
		FIXED = 4,
	};

public:
	void Precache() override;

	void Spawn() override;

	void FlyThink();

	void OffsetThink();

	void TongueTouch(CBaseEntity* pOther);

	TargetClass ClassifyTarget(CBaseEntity* pTarget);

	void SetPosition(const Vector& vecOrigin, const Vector& vecAngles, CBaseEntity* pOwner);

	TargetClass GetGrappleType() const { return m_GrappleType; }

	bool IsStuck() const { return m_bIsStuck; }

	bool HasMissed() const { return m_bMissed; }

	EHANDLE& GetGrappleTarget() { return m_hGrappleTarget; }

	void SetGrappleTarget(CBaseEntity* pTarget)
	{
		m_hGrappleTarget = pTarget;
	}

private:
	TargetClass m_GrappleType;
	bool m_bIsStuck;
	bool m_bMissed;

	EHANDLE m_hGrappleTarget;
	Vector m_vecOriginOffset;
};
