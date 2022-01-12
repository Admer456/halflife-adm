
#pragma once

// ================================
// FilterDifficulty
//
// A filter difficulty that lets a trigger execute 
// depending on the current difficulty
// ================================
class FilterDifficulty final : public CPointEntity
{
public:
	void Spawn() override;
	bool KeyValue( KeyValueData* pkvd ) override;

	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;

	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

private:
	int m_SkillFlags{0};
};
