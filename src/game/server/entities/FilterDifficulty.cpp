
#include "cbase.h"
#include "FilterDifficulty.h"

LINK_ENTITY_TO_CLASS(filter_difficulty, FilterDifficulty);

TYPEDESCRIPTION FilterDifficulty::m_SaveData[] =
{
	DEFINE_FIELD(FilterDifficulty, m_SkillFlags, FIELD_INTEGER)
};

IMPLEMENT_SAVERESTORE(FilterDifficulty, CPointEntity);

// ================================
// FilterDifficulty::Spawn
// ================================
void FilterDifficulty::Spawn()
{
	CPointEntity::Spawn();
}

// ================================
// FilterDifficulty::KeyValue
// ================================
bool FilterDifficulty::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "skill"))
	{
		m_SkillFlags = atoi(pkvd->szValue);
		return true;
	}

	return CPointEntity::KeyValue(pkvd);
}

// ================================
// FilterDifficulty::Use
// ================================
void FilterDifficulty::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	// skill 0 is easy, and so on
	int skillLevel = CVAR_GET_FLOAT("skill");

	// In JACK, this is a 'choices' keyvalue, but in TrenchBroom,
	// it's a 'flags' keyvalue. Either way, it is a bitmask
	if (m_SkillFlags & (1 << skillLevel))
	{
		SUB_UseTargets(pActivator, useType, value);		
	}
}
