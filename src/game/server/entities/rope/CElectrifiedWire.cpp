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
#include "cbase.h"

#include "CRopeSegment.h"

#include "CElectrifiedWire.h"

BEGIN_DATAMAP(CElectrifiedWire)
DEFINE_FIELD(m_bIsActive, FIELD_BOOLEAN),

	DEFINE_FIELD(m_iTipSparkFrequency, FIELD_INTEGER),
	DEFINE_FIELD(m_iBodySparkFrequency, FIELD_INTEGER),
	DEFINE_FIELD(m_iLightningFrequency, FIELD_INTEGER),

	DEFINE_FIELD(m_iXJoltForce, FIELD_INTEGER),
	DEFINE_FIELD(m_iYJoltForce, FIELD_INTEGER),
	DEFINE_FIELD(m_iZJoltForce, FIELD_INTEGER),

	DEFINE_FIELD(m_uiNumUninsulatedSegments, FIELD_INTEGER),
	DEFINE_ARRAY(m_uiUninsulatedSegments, FIELD_INTEGER, CElectrifiedWire::MAX_SEGMENTS),

	// DEFINE_FIELD(m_iLightningSprite, FIELD_INTEGER), //Not restored, reset in Precache.

	DEFINE_FIELD(m_flLastSparkTime, FIELD_TIME),
	END_DATAMAP();

LINK_ENTITY_TO_CLASS(env_electrified_wire, CElectrifiedWire);

bool CElectrifiedWire::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "sparkfrequency"))
	{
		m_iTipSparkFrequency = strtol(pkvd->szValue, nullptr, 10);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "bodysparkfrequency"))
	{
		m_iBodySparkFrequency = strtol(pkvd->szValue, nullptr, 10);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "lightningfrequency"))
	{
		m_iLightningFrequency = strtol(pkvd->szValue, nullptr, 10);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "xforce"))
	{
		m_iXJoltForce = strtol(pkvd->szValue, nullptr, 10);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "yforce"))
	{
		m_iYJoltForce = strtol(pkvd->szValue, nullptr, 10);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "zforce"))
	{
		m_iZJoltForce = strtol(pkvd->szValue, nullptr, 10);
		return true;
	}

	return BaseClass::KeyValue(pkvd);
}

void CElectrifiedWire::Precache()
{
	BaseClass::Precache();

	m_iLightningSprite = PrecacheModel("sprites/lgtning.spr");
}

void CElectrifiedWire::Spawn()
{
	BaseClass::Spawn();

	m_uiNumUninsulatedSegments = 0;
	m_bIsActive = true;

	if (m_iBodySparkFrequency > 0)
	{
		for (size_t uiIndex = 0; uiIndex < GetNumSegments(); ++uiIndex)
		{
			if (IsValidSegmentIndex(uiIndex))
			{
				m_uiUninsulatedSegments[m_uiNumUninsulatedSegments++] = uiIndex;
			}
		}
	}

	if (m_uiNumUninsulatedSegments > 0)
	{
		for (size_t uiIndex = 0; uiIndex < m_uiNumUninsulatedSegments; ++uiIndex)
		{
			GetSegments()[uiIndex]->SetCauseDamageOnTouch(IsActive());
			GetAltSegments()[uiIndex]->SetCauseDamageOnTouch(IsActive());
		}
	}

	if (m_iTipSparkFrequency > 0)
	{
		GetSegments()[GetNumSegments() - 1]->SetCauseDamageOnTouch(IsActive());
		GetAltSegments()[GetNumSegments() - 1]->SetCauseDamageOnTouch(IsActive());
	}

	m_flLastSparkTime = gpGlobals->time;

	SetSoundAllowed(false);
}

void CElectrifiedWire::Think()
{
	if (gpGlobals->time - m_flLastSparkTime > 0.1)
	{
		m_flLastSparkTime = gpGlobals->time;

		if (m_uiNumUninsulatedSegments > 0)
		{
			for (size_t uiIndex = 0; uiIndex < m_uiNumUninsulatedSegments; ++uiIndex)
			{
				if (ShouldDoEffect(m_iBodySparkFrequency))
				{
					DoSpark(m_uiUninsulatedSegments[uiIndex], false);
				}
			}
		}

		if (ShouldDoEffect(m_iTipSparkFrequency))
		{
			DoSpark(GetNumSegments() - 1, true);
		}

		if (ShouldDoEffect(m_iLightningFrequency))
			DoLightning();
	}

	BaseClass::Think();
}

void CElectrifiedWire::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float flValue)
{
	m_bIsActive = !m_bIsActive;

	if (m_uiNumUninsulatedSegments > 0)
	{
		for (size_t uiIndex = 0; uiIndex < m_uiNumUninsulatedSegments; ++uiIndex)
		{
			GetSegments()[m_uiUninsulatedSegments[uiIndex]]->SetCauseDamageOnTouch(IsActive());
			GetAltSegments()[m_uiUninsulatedSegments[uiIndex]]->SetCauseDamageOnTouch(IsActive());
		}
	}

	if (m_iTipSparkFrequency > 0)
	{
		GetSegments()[GetNumSegments() - 1]->SetCauseDamageOnTouch(IsActive());
		GetAltSegments()[GetNumSegments() - 1]->SetCauseDamageOnTouch(IsActive());
	}
}

bool CElectrifiedWire::ShouldDoEffect(const int iFrequency)
{
	if (iFrequency <= 0)
		return false;

	if (!IsActive())
		return false;

	return RANDOM_LONG(1, iFrequency) == 1;
}

void CElectrifiedWire::DoSpark(const size_t uiSegment, const bool bExertForce)
{
	const Vector vecOrigin = GetSegmentAttachmentPoint(uiSegment);

	UTIL_Sparks(vecOrigin);

	if (bExertForce)
	{
		const Vector vecSparkForce(
			RANDOM_FLOAT(-m_iXJoltForce, m_iXJoltForce),
			RANDOM_FLOAT(-m_iYJoltForce, m_iYJoltForce),
			RANDOM_FLOAT(-m_iZJoltForce, m_iZJoltForce));

		ApplyForceToSegment(vecSparkForce, uiSegment);
	}
}

void CElectrifiedWire::DoLightning()
{
	const size_t uiSegment1 = RANDOM_LONG(0, GetNumSegments() - 1);

	size_t uiSegment2;

	size_t uiIndex;

	// Try to get a random segment.
	for (uiIndex = 0; uiIndex < 10; ++uiIndex)
	{
		uiSegment2 = RANDOM_LONG(0, GetNumSegments() - 1);

		if (uiSegment2 != uiSegment1)
			break;
	}

	if (uiIndex >= 10)
		return;

	CRopeSegment* pSegment1;
	CRopeSegment* pSegment2;

	if (GetToggleValue())
	{
		pSegment1 = GetAltSegments()[uiSegment1];
		pSegment2 = GetAltSegments()[uiSegment2];
	}
	else
	{
		pSegment1 = GetSegments()[uiSegment1];
		pSegment2 = GetSegments()[uiSegment2];
	}

	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_BEAMENTS);
	WRITE_SHORT(pSegment1->entindex());
	WRITE_SHORT(pSegment2->entindex());
	WRITE_SHORT(m_iLightningSprite);
	WRITE_BYTE(0);
	WRITE_BYTE(0);
	WRITE_BYTE(1);
	WRITE_BYTE(10);
	WRITE_BYTE(80);
	WRITE_BYTE(255);
	WRITE_BYTE(255);
	WRITE_BYTE(255);
	WRITE_BYTE(255);
	WRITE_BYTE(255);
	MESSAGE_END();
}
