
#pragma once

// ================================
// UtilServerCommand
// 
// A utility entity that executes serverside console commands
// ================================
class UtilServerCommand : public CBaseEntity
{
	DECLARE_CLASS(UtilServerCommand, CPointEntity);
	DECLARE_DATAMAP();

public:
	void	Spawn() override;
	bool	KeyValue( KeyValueData* pkvd ) override;
	void	Use( CBaseEntity* activator, CBaseEntity* caller, USE_TYPE useType, float value ) override;

protected:
	string_t m_Command{};
};
