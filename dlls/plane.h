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

//=========================================================
// Plane
//=========================================================
class CPlane
{
public:
	CPlane();

	//=========================================================
	// InitializePlane - Takes a normal for the plane and a
	// point on the plane and
	//=========================================================
	void InitializePlane(const Vector& vecNormal, const Vector& vecPoint);

	//=========================================================
	// PointInFront - determines whether the given vector is
	// in front of the plane.
	//=========================================================
	bool PointInFront(const Vector& vecPoint);

	Vector m_vecNormal;
	float m_flDist;
	bool m_fInitialized;
};
