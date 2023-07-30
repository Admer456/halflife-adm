
#pragma once

// ================================
// TriggerTimer
//
// A simple trigger timer with optional random timing
// Formula: baseDelay + randomDelay * random(-1.0, 1.0)
// ================================
class TriggerTimer final : public CPointEntity
{	// would probably be more logical to inherit from trigger_relay, but alas
	DECLARE_CLASS(TriggerTimer, CPointEntity);
	DECLARE_DATAMAP();

public:
	static constexpr uint32_t SF_StartOn = 1 << 0;

	void			Spawn() override;
	bool			KeyValue(KeyValueData* pkvd) override;
	
	void			Use(CBaseEntity* activator, CBaseEntity* caller, USE_TYPE useType, float value) override;
	void			Think() override;

private:
	void			Tick(bool useTargets = false);

private:
	// When this entity gets triggered by another, remember the
	// original activator to maintain the trigger chain
	CBaseEntity*	m_Activator{nullptr};
	USE_TYPE		m_UseType{USE_OFF};
	// State management
	bool			m_Enabled{false};

	// Properties set by the mapper
	float			m_BaseDelay{0.0f};
	float			m_RandomDelay{0.0f};
};
