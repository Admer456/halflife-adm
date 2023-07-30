
#pragma once

// ================================
// FilterDifficulty
//
// A filter difficulty that lets a trigger execute 
// depending on the current difficulty
// ================================
class FilterDifficulty final : public CPointEntity
{
	DECLARE_CLASS(FilterDifficulty, CPointEntity);
	DECLARE_DATAMAP();

public:
	void Spawn() override;
	bool KeyValue( KeyValueData* pkvd ) override;

	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

private:
	int m_SkillFlags{0};
};
