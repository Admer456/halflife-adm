
#include "extdll.h"
#include "util.h"
#include "cbase.h"

#include "FilterDateYMD.h"

#include "utils/CTimeUtils.h"

LINK_ENTITY_TO_CLASS(filter_date_ymd, FilterDateYMD);

TYPEDESCRIPTION FilterDateYMD::m_SaveData[] =
{
	DEFINE_FIELD(FilterDateYMD, m_Year, FIELD_INTEGER),
	DEFINE_FIELD(FilterDateYMD, m_Month, FIELD_INTEGER),
	DEFINE_FIELD(FilterDateYMD, m_Day, FIELD_INTEGER)
};

IMPLEMENT_SAVERESTORE(FilterDateYMD, CPointEntity);

// ================================
// FilterDateYMD::Spawn
// ================================
void FilterDateYMD::Spawn()
{
	CPointEntity::Spawn();

	if (!pev->targetname)
	{
		ALERT(at_warning, "filter_date_ymd at ( %4.2f %4.2f ) does not have a targetname! Removing...\n",
			pev->origin.x, pev->origin.y, pev->origin.z);

		UTIL_Remove(this);
		return;
	}

	if (!pev->target)
	{
		ALERT(at_warning, "filter_date_ymd '%s' does not have a target! Removing...\n",
			STRING(pev->target));

		UTIL_Remove(this);
		return;
	}

	if (m_Year == -1 && m_Month == -1 && m_Day == -1)
	{
		ALERT(at_warning, "filter_date_ymd '%s' has -1 for all parameters!\n"
			"It will not filter anything, so consider removing this entity to use fewer edicts.\n",
			STRING(pev->targetname));
	}
}

// ================================
// FilterDateYMD::KeyValue
// ================================
bool FilterDateYMD::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "year"))
	{
		m_Year = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "month"))
	{
		m_Month = atoi(pkvd->szValue);
		return true;
	}
	else if (FStrEq(pkvd->szKeyName, "day"))
	{
		m_Day = atoi(pkvd->szValue);
		return true;
	}

	return CPointEntity::KeyValue(pkvd);
}

// ================================
// FilterDateYMD::Use
// ================================
void FilterDateYMD::Use(CBaseEntity* activator, CBaseEntity* caller, USE_TYPE useType, float value)
{
	if (!CTimeUtils::IsDate(m_Year, m_Month, m_Day))
	{
		return;
	}

	SUB_UseTargets(activator, useType, value);
}
