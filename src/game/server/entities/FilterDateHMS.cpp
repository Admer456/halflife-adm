
#include "extdll.h"
#include "util.h"
#include "cbase.h"

#include "FilterDateHMS.h"

#include "utils/CTimeUtils.h"

LINK_ENTITY_TO_CLASS(filter_date_hms, FilterDateHMS);

TYPEDESCRIPTION FilterDateHMS::m_SaveData[] =
{
	DEFINE_FIELD(FilterDateHMS, m_Hour, FIELD_INTEGER),
	DEFINE_FIELD(FilterDateHMS, m_Minute, FIELD_INTEGER),
	DEFINE_FIELD(FilterDateHMS, m_Second, FIELD_INTEGER)
};

IMPLEMENT_SAVERESTORE(FilterDateHMS, CPointEntity);

// ================================
// FilterDateHMS::Spawn
// ================================
void FilterDateHMS::Spawn()
{
	CPointEntity::Spawn();

	if (!pev->targetname)
	{
		ALERT(at_warning, "filter_date_hms at ( %4.2f %4.2f ) does not have a targetname! Removing...\n",
			pev->origin.x, pev->origin.y, pev->origin.z);

		UTIL_Remove(this);
		return;
	}

	if (!pev->target)
	{
		ALERT(at_warning, "filter_date_hms '%s' does not have a target! Removing...\n",
			STRING(pev->target));

		UTIL_Remove(this);
		return;
	}

	if (m_Hour == -1 && m_Minute == -1 && m_Second == -1)
	{
		ALERT(at_warning, "filter_date_hms '%s' has -1 for all parameters!\n"
						  "It will not filter anything, so consider removing this entity to use fewer edicts.\n",
			STRING(pev->targetname));
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
