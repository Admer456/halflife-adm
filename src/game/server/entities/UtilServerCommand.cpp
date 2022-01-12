
#include "extdll.h"
#include "util.h"
#include "cbase.h"

#include "UtilServerCommand.h"

LINK_ENTITY_TO_CLASS(util_servercommand, UtilServerCommand);

TYPEDESCRIPTION UtilServerCommand::m_SaveData[] =
{
	DEFINE_FIELD(UtilServerCommand, m_Command, FIELD_STRING)
};

IMPLEMENT_SAVERESTORE( UtilServerCommand, CBaseEntity );

// ================================
// UtilServerCommand::Spawn
// ================================
void UtilServerCommand::Spawn()
{
	// Todo: flexible validation methods for less code duplication
	if (!m_Command)
	{
		ALERT(at_console, "util_servercommand at (%4.2f %4.2f %4.2f) does not have a command! Deleting...\n",
			pev->origin.x, pev->origin.y, pev->origin.z);
		
		UTIL_Remove(this);
		return;
	}

	if (!STRING(m_Command)[0])
	{
		ALERT(at_console, "util_servercommand at (%4.2f %4.2f %4.2f) has empty command! Deleting...", 
			pev->origin.x, pev->origin.y, pev->origin.z);

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
