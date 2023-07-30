
#include "cbase.h"
#include "UtilServerCommand.h"

LINK_ENTITY_TO_CLASS(util_servercommand, UtilServerCommand);

BEGIN_DATAMAP(UtilServerCommand)
	DEFINE_FIELD(m_Command, FIELD_STRING),
END_DATAMAP();

// ================================
// UtilServerCommand::Spawn
// ================================
void UtilServerCommand::Spawn()
{
	// Todo: flexible validation methods for less code duplication
	if (m_Command == string_t::Null)
	{
		Logger->warn("util_servercommand at ({}) does not have a command! Deleting...", pev->origin);
		
		UTIL_Remove(this);
		return;
	}

	if (!STRING(m_Command)[0])
	{
		Logger->warn("util_servercommand at ({}) has empty command! Deleting...", pev->origin);

		UTIL_Remove(this);
		return;
	}
}

// ================================
// UtilServerCommand::KeyValue
// ================================
bool UtilServerCommand::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "command"))
	{
		m_Command = ALLOC_STRING(pkvd->szValue);
		return true;
	}

	return CBaseEntity::KeyValue(pkvd);
}

// ================================
// UtilServerCommand::Use
// 
// Assumes that the command is valid, and is filtered out in Spawn
// ================================
void UtilServerCommand::Use(CBaseEntity* activator, CBaseEntity* caller, USE_TYPE useType, float value)
{
	SERVER_COMMAND(STRING(m_Command));
}
