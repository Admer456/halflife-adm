
#pragma once

// ================================
// UtilPrint
// 
// A simple utility entity for printing to the console
// ================================
class UtilPrint final : public CPointEntity
{
	DECLARE_CLASS(UtilPrint, CPointEntity);

public:
	void Spawn() override;

	void Use(CBaseEntity* activator, CBaseEntity* caller, USE_TYPE useType, float value) override;

	// pev->netname is used as the message field
	// Todo: probably some type of pev binding
};
