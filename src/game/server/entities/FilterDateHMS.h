
#pragma once

// ================================
// FilterDateHMS
//
// Filters entity triggering by hour/minute/second
// ================================
class FilterDateHMS final : public CPointEntity
{
	DECLARE_CLASS(FilterDateHMS, CPointEntity);
	DECLARE_DATAMAP();

public:
	void Spawn() override;
	bool KeyValue(KeyValueData* pkvd) override;

	void Use(CBaseEntity* activator, CBaseEntity* caller, USE_TYPE useType, float value);
private:
	int m_Hour{-1};
	int m_Minute{-1};
	int m_Second{-1};
};
