
#include "cbase.h"
#include "UtilMoveWith.h"

LINK_ENTITY_TO_CLASS(util_movewith, UtilMoveWith);

BEGIN_DATAMAP(UtilMoveWith)
	DEFINE_FIELD(m_ParentTarget, FIELD_STRING),
	DEFINE_FIELD(m_ChildTarget, FIELD_STRING),
	DEFINE_FIELD(m_ParentEntity, FIELD_CLASSPTR),
	DEFINE_FIELD(m_ChildEntity, FIELD_CLASSPTR),
	DEFINE_FIELD(m_Enabled, FIELD_BOOLEAN),
END_DATAMAP();

// ================================
// UtilMoveWith::Spawn
// ================================
void UtilMoveWith::Spawn()
{
	CPointEntity::Spawn();

	if (pev->spawnflags & SF_StartOn)
	{
		m_Enabled = true;
	}
}

// ================================
// UtilMoveWith::Activate
// ================================
void UtilMoveWith::Activate()
{
	m_ParentEntity = UTIL_FindEntityByTargetname(nullptr, STRING(m_ParentTarget));
	m_ChildEntity = UTIL_FindEntityByTargetname(nullptr, STRING(m_ChildTarget));

	if (!Validate())
	{
		UTIL_Remove(this);
		return;
	}
}

// ================================
// UtilMoveWith::KeyValue
// ================================
bool UtilMoveWith::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "parent"))
	{
		m_ParentTarget = ALLOC_STRING(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "child"))
	{
		m_ChildTarget = ALLOC_STRING(pkvd->szValue);
		return true;
	}

	return CPointEntity::KeyValue(pkvd);
}

// ================================
// UtilMoveWith::Use
// ================================
void UtilMoveWith::Use(CBaseEntity* activator, CBaseEntity* caller, USE_TYPE useType, float value)
{
	// It's literally just this
	m_Enabled = !m_Enabled;
}

// ================================
// UtilMoveWith::Think
// ================================
void UtilMoveWith::Think()
{
	if (!m_Enabled)
	{
		return;
	}

	// Edge case: if the parent or child is removed, this will crash
	// We will need some sort of notifier for this
	
	if (~pev->spawnflags & SF_NoPosition)
	{
		m_ChildEntity->pev->origin = m_ParentEntity->pev->origin;
	}

	if (~pev->spawnflags & SF_NoAngles)
	{
		m_ChildEntity->pev->angles = m_ParentEntity->pev->angles;
	}

	pev->nextthink = gpGlobals->time + 0.001f;
}

// ================================
// UtilMoveWith::Validate
// ================================
bool UtilMoveWith::Validate() const
{
	// Gotta let the mapper easily recognise which entity this is
	std::string identifier;
	if (pev->targetname != string_t::Null)
	{
		identifier = std::string("'") + STRING(pev->targetname) + "'";
	}
	else 
	{
		// Should probably write UTIL_VecToString, or even better, write a Vector::ToString()
		char buffer[128];
		snprintf(buffer, 128, "at ( %4.2f %4.2f %4.2f )", pev->origin.x, pev->origin.y, pev->origin.z);
		identifier = buffer;
	}

	const bool parentTargetNull = m_ParentTarget == string_t::Null;
	const bool childTargetNull = m_ChildTarget == string_t::Null;

	if (parentTargetNull && childTargetNull)
	{
		Logger->warn("util_movewith {} does not have 'parent' and 'child' keyvalues! Removing...", identifier);
		return false;
	}

	if (parentTargetNull)
	{
		Logger->warn("util_movewith %s does not have 'parent' keyvalue! Removing...", identifier);
		return false;
	}

	if (childTargetNull)
	{
		Logger->warn("util_movewith %s does not have 'child' keyvalue! Removing...", identifier);
		return false;
	}

	if (nullptr == m_ParentEntity)
	{
		Logger->warn("util_movewith %s: parent entity '%s' does not exist! Did you make a typo? Removing...", identifier, STRING(m_ParentTarget));
		return false;
	}

	if (nullptr == m_ChildEntity)
	{
		Logger->warn("util_movewith %s: child entity '%s' does not exist! Did you make a typo? Removing...", identifier, STRING(m_ChildTarget));
		return false;
	}

	if (m_ParentEntity == m_ChildEntity)
	{
		Logger->warn("util_movewith %s: 'parent' and 'child' are the same! That makes util_movewith needless. Removing...", identifier);
		return false;
	}

	return true;
}
