
#pragma once

// ================================
// UtilServerCommand
// 
// A utility entity that executes serverside console commands
// ================================
class UtilServerCommand : public CBaseEntity
{
public:
	void	Spawn() override;
	bool	KeyValue( KeyValueData* pkvd ) override;
	void	Use( CBaseEntity* activator, CBaseEntity* caller, USE_TYPE useType, float value ) override;

	bool	Save( CSave& save ) override;
	bool	Restore( CRestore& restore ) override;

	static	TYPEDESCRIPTION m_SaveData[];

protected:
	string_t m_Command{iStringNull};
};
