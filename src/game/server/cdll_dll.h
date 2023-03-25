/***
 *
 *	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
 *
 *	This product contains software technology licensed from Id
 *	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
 *	All Rights Reserved.
 *
 *   Use, distribution, and modification of this source code and/or resulting
 *   object code is restricted to non-commercial enhancements to products from
 *   Valve LLC.  All other use, distribution, or modification is prohibited
 *   without written permission from Valve LLC.
 *
 ****/
//
//  cdll_dll.h

// this file is included by both the game-dll and the client-dll,

#pragma once

#include <optional>
#include <string_view>

#include "palette.h"

#define MAX_WEAPONS 64 // ???
constexpr int MAX_PLAYERS = 32;

#define MAX_WEAPON_SLOTS 7 // hud item selection slots

#define HIDEHUD_WEAPONS (1 << 0)
#define HIDEHUD_FLASHLIGHT (1 << 1)
#define HIDEHUD_ALL (1 << 2)
#define HIDEHUD_HEALTH (1 << 3)

#define MAX_AMMO_TYPES 32

#define HUD_PRINTNOTIFY 1
#define HUD_PRINTCONSOLE 2
#define HUD_PRINTTALK 3
#define HUD_PRINTCENTER 4

enum WeaponId
{
	WEAPON_NONE = 0,
	WEAPON_CROWBAR,
	WEAPON_GLOCK,
	WEAPON_PYTHON,
	WEAPON_MP5,
	WEAPON_CHAINGUN,
	WEAPON_CROSSBOW,
	WEAPON_SHOTGUN,
	WEAPON_RPG,
	WEAPON_GAUSS,
	WEAPON_EGON,
	WEAPON_HORNETGUN,
	WEAPON_HANDGRENADE,
	WEAPON_TRIPMINE,
	WEAPON_SATCHEL,
	WEAPON_SNARK,
	WEAPON_GRAPPLE,
	WEAPON_EAGLE,
	WEAPON_PIPEWRENCH,
	WEAPON_M249,
	WEAPON_DISPLACER,
	WEAPON_SHOCKRIFLE,
	WEAPON_SPORELAUNCHER,
	WEAPON_SNIPERRIFLE,
	WEAPON_KNIFE,
	WEAPON_PENGUIN,

	WEAPON_SUIT = 31
};

/**
*	@brief Indicates that a weapon does not use magazines.
*/
constexpr int WEAPON_NOCLIP = -1;

enum class SuitLightType
{
	Flashlight = 0,
	Nightvision
};

struct SuitLightTypeInfo
{
	const std::string_view Name;
};

constexpr SuitLightTypeInfo SuitLightTypes[] =
	{
		{"flashlight"},
		{"nightvision"}};

constexpr std::optional<SuitLightType> SuitLightTypeFromString(std::string_view value)
{
	for (int i = 0; const auto& type : SuitLightTypes)
	{
		if (type.Name == value)
		{
			return static_cast<SuitLightType>(i);
		}

		++i;
	}

	return {};
}

// used by suit voice to indicate damage sustained and repaired type to player

// instant damage

#define DMG_GENERIC 0			 // generic damage was done
#define DMG_CRUSH (1 << 0)		 // crushed by falling or moving object
#define DMG_BULLET (1 << 1)		 // shot
#define DMG_SLASH (1 << 2)		 // cut, clawed, stabbed
#define DMG_BURN (1 << 3)		 // heat burned
#define DMG_FREEZE (1 << 4)		 // frozen
#define DMG_FALL (1 << 5)		 // fell too far
#define DMG_BLAST (1 << 6)		 // explosive blast damage
#define DMG_CLUB (1 << 7)		 // crowbar, punch, headbutt
#define DMG_SHOCK (1 << 8)		 // electric shock
#define DMG_SONIC (1 << 9)		 // sound pulse shockwave
#define DMG_ENERGYBEAM (1 << 10) // laser or other high energy beam
#define DMG_NEVERGIB (1 << 12)	 // with this bit OR'd in, no damage type will be able to gib victims upon death
#define DMG_ALWAYSGIB (1 << 13)	 // with this bit OR'd in, any damage type can be made to gib victims upon death.

// time-based damage
// mask off TF-specific stuff too
#define DMG_TIMEBASED (~(0xff003fff)) // mask for time-based damage

#define DMG_DROWN (1 << 14) // Drowning
#define DMG_FIRSTTIMEBASED DMG_DROWN

#define DMG_PARALYZE (1 << 15)	   // slows affected creature down
#define DMG_NERVEGAS (1 << 16)	   // nerve toxins, very bad
#define DMG_POISON (1 << 17)	   // blood poisioning
#define DMG_RADIATION (1 << 18)	   // radiation exposure
#define DMG_DROWNRECOVER (1 << 19) // drowning recovery
#define DMG_ACID (1 << 20)		   // toxic chemicals or acid burns
#define DMG_SLOWBURN (1 << 21)	   // in an oven
#define DMG_SLOWFREEZE (1 << 22)   // in a subzero freezer
#define DMG_MORTAR (1 << 23)	   // Hit by air raid (done to distinguish grenade from mortar)

// TF ADDITIONS
#define DMG_IGNITE (1 << 24)	   // Players hit by this begin to burn
#define DMG_RADIUS_MAX (1 << 25)   // Radius damage with this flag doesn't decrease over distance
#define DMG_RADIUS_QUAKE (1 << 26) // Radius damage is done like Quake. 1/2 damage at 1/2 radius.
#define DMG_IGNOREARMOR (1 << 27)  // Damage ignores target's armor
#define DMG_AIMED (1 << 28)		   // Does Hit location damage
#define DMG_WALLPIERCING (1 << 29) // Blast Damages ents through walls

#define DMG_CALTROP (1 << 30)
#define DMG_HALLUC (1 << 31)

// TF Healing Additions for TakeHealth
#define DMG_IGNORE_MAXHEALTH DMG_IGNITE
// TF Redefines since we never use the originals
#define DMG_NAIL DMG_SLASH
#define DMG_NOT_SELF DMG_FREEZE


#define DMG_TRANQ DMG_MORTAR
#define DMG_CONCUSS DMG_SONIC

// these are the damage types that are allowed to gib corpses
#define DMG_GIB_CORPSE (DMG_CRUSH | DMG_FALL | DMG_BLAST | DMG_SONIC | DMG_CLUB)

// these are the damage types that have client hud art
#define DMG_SHOWNHUD (DMG_POISON | DMG_ACID | DMG_FREEZE | DMG_SLOWFREEZE | DMG_DROWN | DMG_BURN | DMG_SLOWBURN | DMG_NERVEGAS | DMG_RADIATION | DMG_SHOCK)

// NOTE: tweak these values based on gameplay feedback:

#define PARALYZE_DURATION 2 // number of 2 second intervals to take damage
#define PARALYZE_DAMAGE 1.0 // damage to take each 2 second interval

#define NERVEGAS_DURATION 2
#define NERVEGAS_DAMAGE 5.0

#define POISON_DURATION 5
#define POISON_DAMAGE 2.0

#define RADIATION_DURATION 2
#define RADIATION_DAMAGE 1.0

#define ACID_DURATION 2
#define ACID_DAMAGE 5.0

#define SLOWBURN_DURATION 2
#define SLOWBURN_DAMAGE 1.0

#define SLOWFREEZE_DURATION 2
#define SLOWFREEZE_DAMAGE 1.0


#define itbd_Paralyze 0
#define itbd_NerveGas 1
#define itbd_Poison 2
#define itbd_Radiation 3
#define itbd_DrownRecover 4
#define itbd_Acid 5
#define itbd_SlowBurn 6
#define itbd_SlowFreeze 7
#define CDMG_TIMEBASED 8

constexpr Vector VEC_HULL_MIN(-16, -16, -36);
constexpr Vector VEC_HULL_MAX(16, 16, 36);
constexpr Vector VEC_HUMAN_HULL_MIN(-16, -16, 0);
constexpr Vector VEC_HUMAN_HULL_MAX(16, 16, 72);
constexpr Vector VEC_HUMAN_HULL_DUCK(16, 16, 36);

constexpr Vector VEC_VIEW(0, 0, 28);

constexpr Vector VEC_DUCK_HULL_MIN(-16, -16, -18);
constexpr Vector VEC_DUCK_HULL_MAX(16, 16, 18);
constexpr Vector VEC_DUCK_VIEW(0, 0, 12);

constexpr Vector VEC_DEAD_VIEW(0, 0, -8);

#define MENU_NONE 0
#define MENU_DEFAULT 1
#define MENU_TEAM 2
#define MENU_CLASS 3
#define MENU_MAPBRIEFING 4
#define MENU_INTRO 5
#define MENU_CLASSHELP 6
#define MENU_CLASSHELP2 7
#define MENU_REPEATHELP 8
// #define MENU_SPECHELP				9
#define MENU_STATSMENU 9
#define MENU_SCOREBOARD 10

constexpr RGB24 RGB_WHITE{255, 255, 255};
constexpr RGB24 RGB_YELLOWISH{255, 160, 0};
constexpr RGB24 RGB_REDISH{255, 16, 16};
constexpr RGB24 RGB_GREENISH{0, 160, 0};
constexpr RGB24 RGB_BLUEISH{95, 95, 255};

constexpr RGB24 RGB_HUD_COLOR{RGB_YELLOWISH};

/**
 *	@brief Amount of time, in seconds, between entity info update checks.
 */
constexpr float EntityInfoUpdateInterval = 0.2f;

/**
 *	@brief Amount of time, in seconds, to draw entity info before disappearing.
 */
constexpr float EntityInfoDrawTime = 2.f;
