
#include "cbase.h"
#include "FilterDateHMS.h"
#include "utils/CTimeUtils.h"

LINK_ENTITY_TO_CLASS(filter_date_hms, FilterDateHMS);

BEGIN_DATAMAP(FilterDateHMS)
	DEFINE_FIELD(m_Hour, FIELD_INTEGER),
	DEFINE_FIELD(m_Minute, FIELD_INTEGER),
	DEFINE_FIELD(m_Second, FIELD_INTEGER),
END_DATAMAP();

// ================================
// FilterDateHMS::Spawn
// ================================
void FilterDateHMS::Spawn()
{
	CPointEntity::Spawn();

	if (pev->targetname == string_t::Null)
	{
		Logger->warn("filter_date_hms at ({}) does not have a targetname! Removing...", pev->origin);
		
		UTIL_Remove(this);
		return;
	}

	if (pev->target == string_t::Null)
	{
		Logger->warn("filter_date_hms '{}' does not have a target! Removing...", STRING(pev->target));

		UTIL_Remove(this);
		return;
	}

	if (m_Hour == -1 && m_Minute == -1 && m_Second == -1)
	{
		Logger->warn("filter_date_hms '%s' has -1 for all parameters!\n"
					 "It will not filter anything, so consider removing this entity to use fewer edicts.\n", STRING(pev->targetname));
	}
}

// ================================
// FilterDateHMS::KeyValue
// ================================
bool FilterDateHMS::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "hour"))
	{
		m_Hour = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "minute"))
	{
		m_Minute = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "second"))
	{
		m_Second = atoi(pkvd->szValue);
		return true;
	}

	return CPointEntity::KeyValue(pkvd);
}

// ================================
// FilterDateHMS::Use
// ================================
void FilterDateHMS::Use(CBaseEntity* activator, CBaseEntity* caller, USE_TYPE useType, float value)
{
	if (!CTimeUtils::IsTime(m_Hour, m_Minute, m_Second))
	{
		return;
	}

	SUB_UseTargets(activator, useType, value);
}
