
#include "cbase.h"
#include "TriggerTimer.h"

LINK_ENTITY_TO_CLASS(trigger_timer, TriggerTimer);

BEGIN_DATAMAP(TriggerTimer)
	DEFINE_FIELD(m_Activator, FIELD_CLASSPTR),
	DEFINE_FIELD(m_Enabled, FIELD_BOOLEAN),
	DEFINE_FIELD(m_UseType, FIELD_INTEGER),
	DEFINE_FIELD(m_BaseDelay, FIELD_FLOAT),
	DEFINE_FIELD(m_RandomDelay, FIELD_FLOAT),
END_DATAMAP();

// ================================
// TriggerTimer::Spawn
// ================================
void TriggerTimer::Spawn()
{
	CPointEntity::Spawn();

	if (pev->spawnflags & SF_StartOn)
	{
		Use(this, this, USE_ON, 0.0f);
	}
}

// ================================
// TriggerTimer::KeyValue
// ================================
bool TriggerTimer::KeyValue(KeyValueData* pkvd)
{
	// Small experiment here to see if we can compare these more easier, without FStrEq
	using namespace std::string_view_literals;
	
	if (pkvd->szKeyName == "baseDelay"sv)
	{
		m_BaseDelay = atof(pkvd->szValue);
		return true;
	}
	else if (pkvd->szKeyName == "randomDelay"sv)
	{
		m_RandomDelay = atof(pkvd->szValue);
		return true;
	}

	return CPointEntity::KeyValue(pkvd);
}

// ================================
// TriggerTimer::Use
// ================================
void TriggerTimer::Use(CBaseEntity* activator, CBaseEntity* caller, USE_TYPE useType, float value)
{
	m_Enabled = !m_Enabled;
	m_Activator = activator;
	m_UseType = useType;

	if (m_Enabled)
	{
		Tick();
	}
}

// ================================
// TriggerTimer::Think
// ================================
void TriggerTimer::Think()
{
	if (!m_Enabled)
	{
		return;
	}

	Tick(true);
}

// ================================
// TriggerTimer::Tick
// 
// Triggers targets and thinks
// ================================
void TriggerTimer::Tick(bool useTargets)
{
	if (useTargets)
	{
		SUB_UseTargets(m_Activator, m_UseType, 0);
	}

	pev->nextthink = gpGlobals->time + m_BaseDelay + RANDOM_FLOAT(-m_RandomDelay, m_RandomDelay);
}
