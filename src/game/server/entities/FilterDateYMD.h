
#pragma once

// ================================
// FilterDateYMD
//
// Filters entity triggering by year/month/day
// ================================
class FilterDateYMD final : public CPointEntity
{
public:
	void Spawn() override;
	bool KeyValue(KeyValueData* pkvd) override;

	void Use(CBaseEntity* activator, CBaseEntity* caller, USE_TYPE useType, float value);
	
	bool Save(CSave& save) override;
	bool Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];

private:
	int m_Year{-1};
	int m_Month{-1};
	int m_Day{-1};
};
