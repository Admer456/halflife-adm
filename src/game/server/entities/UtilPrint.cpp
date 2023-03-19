
#include "cbase.h"
#include "UtilPrint.h"

LINK_ENTITY_TO_CLASS(util_print, UtilPrint);

// ================================
// UtilPrint::Spawn
// ================================
void UtilPrint::Spawn()
{
	if (pev->targetname == string_t::Null)
	{
		Logger->warn("util_print at ({}) doesn't have a name! Removing...", pev->origin);
		return UTIL_Remove(this);
	}

	if (pev->netname == string_t::Null)
	{
		Logger->warn("util_print '{}' does not have a message! Removing...\n", STRING(pev->targetname));
		return UTIL_Remove(this);
	}
}

// ================================
// UtilPrint::Use
// ================================
void UtilPrint::Use(CBaseEntity* activator, CBaseEntity* caller, USE_TYPE useType, float value)
{
	Logger->info("{}", STRING(pev->netname));
}
