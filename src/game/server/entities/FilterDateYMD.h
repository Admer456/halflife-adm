
#pragma once

// ================================
// FilterDateYMD
//
// Filters entity triggering by year/month/day
// ================================
class FilterDateYMD final : public CPointEntity
{
	DECLARE_CLASS(FilterDateYMD, CPointEntity);
	DECLARE_DATAMAP();

public:
	void Spawn() override;
	bool KeyValue(KeyValueData* pkvd) override;

	void Use(CBaseEntity* activator, CBaseEntity* caller, USE_TYPE useType, float value);

private:
	int m_Year{-1};
	int m_Month{-1};
	int m_Day{-1};
};
