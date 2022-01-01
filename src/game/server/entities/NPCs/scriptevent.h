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

#pragma once

#define SCRIPT_EVENT_DEAD					1000		// character is now dead
#define SCRIPT_EVENT_NOINTERRUPT			1001		// does not allow interrupt
#define SCRIPT_EVENT_CANINTERRUPT			1002		// will allow interrupt
#define SCRIPT_EVENT_FIREEVENT				1003		// event now fires
#define SCRIPT_EVENT_SOUND					1004		// Play named wave file (on CHAN_BODY)
#define SCRIPT_EVENT_SENTENCE				1005		// Play named sentence
#define SCRIPT_EVENT_INAIR					1006		// Leave the character in air at the end of the sequence (don't find the floor)
#define SCRIPT_EVENT_ENDANIMATION			1007		// Set the animation by name after the sequence completes
#define SCRIPT_EVENT_SOUND_VOICE			1008		// Play named wave file (on CHAN_VOICE)
#define	SCRIPT_EVENT_SENTENCE_RND1			1009		// Play sentence group 25% of the time
#define SCRIPT_EVENT_NOT_DEAD				1010		// Bring back to life (for life/death sequences)
#define SCRIPT_EVENT_SOUND_VOICE_BODY		1011		// Play named wave file with normal attenuation (on CHAN_BODY)
#define SCRIPT_EVENT_SOUND_VOICE_VOICE		1012		// Play named wave file with normal attenuation (on CHAN_VOICE)
#define SCRIPT_EVENT_SOUND_VOICE_WEAPON		1013		// Play named wave file with normal attenuation (on CHAN_WEAPON)
