
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

class CShockBeam : public CGrenade
{
	DECLARE_CLASS(CShockBeam, CGrenade);
	DECLARE_DATAMAP();

public:
	void Precache() override;

	void Spawn() override;

	int Classify() override { return CLASS_NONE; }

	void FlyThink();

	void ExplodeThink();

	void WaterExplodeThink();

	void BallTouch(CBaseEntity* pOther);

	static CShockBeam* CreateShockBeam(const Vector& vecOrigin, const Vector& vecAngles, CBaseEntity* pOwner);

private:
	void Explode();

public:
	CBeam* m_pBeam1;
	CBeam* m_pBeam2;

	CSprite* m_pSprite;

	int m_iBeams;
};
