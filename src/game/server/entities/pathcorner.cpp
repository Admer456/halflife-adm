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
#include "trains.h"

class CPathCorner : public CPointEntity
{
public:
	void Spawn() override;
	bool KeyValue(KeyValueData* pkvd) override;
	float GetDelay() override { return m_flWait; }
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

private:
	float m_flWait;
};

LINK_ENTITY_TO_CLASS(path_corner, CPathCorner);

TYPEDESCRIPTION CPathCorner::m_SaveData[] =
	{
		DEFINE_FIELD(CPathCorner, m_flWait, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CPathCorner, CPointEntity);

bool CPathCorner::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "wait"))
	{
		m_flWait = atof(pkvd->szValue);
		return true;
	}

	return CPointEntity::KeyValue(pkvd);
}

void CPathCorner::Spawn()
{
	ASSERTSZ(!FStringNull(pev->targetname), "path_corner without a targetname");
}

TYPEDESCRIPTION CPathTrack::m_SaveData[] =
	{
		DEFINE_FIELD(CPathTrack, m_length, FIELD_FLOAT),
		DEFINE_FIELD(CPathTrack, m_pnext, FIELD_CLASSPTR),
		DEFINE_FIELD(CPathTrack, m_paltpath, FIELD_CLASSPTR),
		DEFINE_FIELD(CPathTrack, m_pprevious, FIELD_CLASSPTR),
		DEFINE_FIELD(CPathTrack, m_altName, FIELD_STRING),
};

IMPLEMENT_SAVERESTORE(CPathTrack, CPointEntity);
LINK_ENTITY_TO_CLASS(path_track, CPathTrack);

bool CPathTrack::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "altpath"))
	{
		m_altName = ALLOC_STRING(pkvd->szValue);
		return true;
	}

	return CPointEntity::KeyValue(pkvd);
}

void CPathTrack::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	bool on;

	// Use toggles between two paths
	if (m_paltpath)
	{
		on = !FBitSet(pev->spawnflags, SF_PATH_ALTERNATE);
		if (ShouldToggle(useType, on))
		{
			if (on)
				SetBits(pev->spawnflags, SF_PATH_ALTERNATE);
			else
				ClearBits(pev->spawnflags, SF_PATH_ALTERNATE);
		}
	}
	else // Use toggles between enabled/disabled
	{
		on = !FBitSet(pev->spawnflags, SF_PATH_DISABLED);

		if (ShouldToggle(useType, on))
		{
			if (on)
				SetBits(pev->spawnflags, SF_PATH_DISABLED);
			else
				ClearBits(pev->spawnflags, SF_PATH_DISABLED);
		}
	}
}

void CPathTrack::Link()
{
	if (!FStringNull(pev->target))
	{
		auto target = UTIL_FindEntityByTargetname(nullptr, STRING(pev->target));
		if (!FNullEnt(target))
		{
			m_pnext = CPathTrack::Instance(target);

			if (m_pnext) // If no next pointer, this is the end of a path
			{
				m_pnext->SetPrevious(this);
			}
		}
		else
			Logger->debug("Dead end link {}", STRING(pev->target));
	}

	// Find "alternate" path
	if (!FStringNull(m_altName))
	{
		auto target = UTIL_FindEntityByTargetname(nullptr, STRING(m_altName));
		if (!FNullEnt(target))
		{
			m_paltpath = CPathTrack::Instance(target);

			if (m_paltpath) // If no next pointer, this is the end of a path
			{
				m_paltpath->SetPrevious(this);
			}
		}
	}
}

void CPathTrack::Spawn()
{
	pev->solid = SOLID_TRIGGER;
	SetSize(Vector(-8, -8, -8), Vector(8, 8, 8));

	m_pnext = nullptr;
	m_pprevious = nullptr;
	// DEBUGGING CODE
#if PATH_SPARKLE_DEBUG
	SetThink(Sparkle);
	pev->nextthink = gpGlobals->time + 0.5;
#endif
}

void CPathTrack::Activate()
{
	if (!FStringNull(pev->targetname)) // Link to next, and back-link
		Link();
}

CPathTrack* CPathTrack::ValidPath(CPathTrack* ppath, bool testFlag)
{
	if (!ppath)
		return nullptr;

	if (testFlag && FBitSet(ppath->pev->spawnflags, SF_PATH_DISABLED))
		return nullptr;

	return ppath;
}

void CPathTrack::Project(CPathTrack* pstart, CPathTrack* pend, Vector* origin, float dist)
{
	if (pstart && pend)
	{
		Vector dir = (pend->pev->origin - pstart->pev->origin);
		dir = dir.Normalize();
		*origin = pend->pev->origin + dir * dist;
	}
}

CPathTrack* CPathTrack::GetNext()
{
	if (m_paltpath && FBitSet(pev->spawnflags, SF_PATH_ALTERNATE) && !FBitSet(pev->spawnflags, SF_PATH_ALTREVERSE))
		return m_paltpath;

	return m_pnext;
}

CPathTrack* CPathTrack::GetPrevious()
{
	if (m_paltpath && FBitSet(pev->spawnflags, SF_PATH_ALTERNATE) && FBitSet(pev->spawnflags, SF_PATH_ALTREVERSE))
		return m_paltpath;

	return m_pprevious;
}

void CPathTrack::SetPrevious(CPathTrack* pprev)
{
	// Only set previous if this isn't my alternate path
	if (pprev && !FStrEq(STRING(pprev->pev->targetname), STRING(m_altName)))
		m_pprevious = pprev;
}

// Assumes this is ALWAYS enabled
CPathTrack* CPathTrack::LookAhead(Vector* origin, float dist, bool move)
{
	CPathTrack* pcurrent;
	float originalDist = dist;

	pcurrent = this;
	Vector currentPos = *origin;

	if (dist < 0) // Travelling backwards through path
	{
		dist = -dist;
		while (dist > 0)
		{
			Vector dir = pcurrent->pev->origin - currentPos;
			float length = dir.Length();
			if (0 == length)
			{
				if (!ValidPath(pcurrent->GetPrevious(), move)) // If there is no previous node, or it's disabled, return now.
				{
					if (!move)
						Project(pcurrent->GetNext(), pcurrent, origin, dist);
					return nullptr;
				}
				pcurrent = pcurrent->GetPrevious();
			}
			else if (length > dist) // enough left in this path to move
			{
				*origin = currentPos + (dir * (dist / length));
				return pcurrent;
			}
			else
			{
				dist -= length;
				currentPos = pcurrent->pev->origin;
				*origin = currentPos;
				if (!ValidPath(pcurrent->GetPrevious(), move)) // If there is no previous node, or it's disabled, return now.
					return nullptr;

				pcurrent = pcurrent->GetPrevious();
			}
		}
		*origin = currentPos;
		return pcurrent;
	}
	else
	{
		while (dist > 0)
		{
			if (!ValidPath(pcurrent->GetNext(), move)) // If there is no next node, or it's disabled, return now.
			{
				if (!move)
					Project(pcurrent->GetPrevious(), pcurrent, origin, dist);
				return nullptr;
			}
			Vector dir = pcurrent->GetNext()->pev->origin - currentPos;
			float length = dir.Length();
			if (0 == length && !ValidPath(pcurrent->GetNext()->GetNext(), move))
			{
				if (dist == originalDist) // HACK -- up against a dead end
					return nullptr;
				return pcurrent;
			}
			if (length > dist) // enough left in this path to move
			{
				*origin = currentPos + (dir * (dist / length));
				return pcurrent;
			}
			else
			{
				dist -= length;
				currentPos = pcurrent->GetNext()->pev->origin;
				pcurrent = pcurrent->GetNext();
				*origin = currentPos;
			}
		}
		*origin = currentPos;
	}

	return pcurrent;
}

// Assumes this is ALWAYS enabled
CPathTrack* CPathTrack::Nearest(Vector origin)
{
	int deadCount;
	float minDist, dist;
	Vector delta;
	CPathTrack *ppath, *pnearest;


	delta = origin - pev->origin;
	delta.z = 0;
	minDist = delta.Length();
	pnearest = this;
	ppath = GetNext();

	// Hey, I could use the old 2 racing pointers solution to this, but I'm lazy :)
	deadCount = 0;
	while (ppath && ppath != this)
	{
		deadCount++;
		if (deadCount > 9999)
		{
			Logger->error("Bad sequence of path_tracks from {}", STRING(pev->targetname));
			return nullptr;
		}
		delta = origin - ppath->pev->origin;
		delta.z = 0;
		dist = delta.Length();
		if (dist < minDist)
		{
			minDist = dist;
			pnearest = ppath;
		}
		ppath = ppath->GetNext();
	}
	return pnearest;
}

CPathTrack* CPathTrack::Instance(CBaseEntity* pent)
{
	if (pent && pent->ClassnameIs("path_track"))
		return static_cast<CPathTrack*>(pent);
	return nullptr;
}

// DEBUGGING CODE
#if PATH_SPARKLE_DEBUG
void CPathTrack::Sparkle()
{

	pev->nextthink = gpGlobals->time + 0.2;
	if (FBitSet(pev->spawnflags, SF_PATH_DISABLED))
		UTIL_ParticleEffect(pev->origin, Vector(0, 0, 100), 210, 10);
	else
		UTIL_ParticleEffect(pev->origin, Vector(0, 0, 100), 84, 10);
}
#endif
