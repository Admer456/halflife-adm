
#include "extdll.h"
#include "util.h"
#include "cbase.h"

#include "UtilPrint.h"

LINK_ENTITY_TO_CLASS(util_print, UtilPrint);

// ================================
// UtilPrint::Spawn
// ================================
void UtilPrint::Spawn()
{
	if (!pev->targetname)
	{
		ALERT(at_console, "util_print at ( %4.2f %4.2f %4.2f ) doesn't have a name! Removing...\n",
			pev->origin.x, pev->origin.y, pev->origin.z);
		return UTIL_Remove(this);
	}

	if (!pev->netname)
	{
		ALERT(at_console, "util_print '%s' does not have a message! Removing...\n", STRING(pev->targetname));
		return UTIL_Remove(this);
	}
}

// ================================
// UtilPrint::Use
// ================================
void UtilPrint::Use(CBaseEntity* activator, CBaseEntity* caller, USE_TYPE useType, float value)
{
	ALERT(at_console, "%s\n", STRING(pev->netname));
}
