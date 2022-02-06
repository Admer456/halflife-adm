//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================

// Triangle rendering, if any

#include "hud.h"

// Triangle rendering apis are in gEngfuncs.pTriAPI

#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "triangleapi.h"
#include "Exports.h"

#include "particleman.h"
#include "tri.h"
extern IParticleMan* g_pParticleMan;

// Since this is just an experiment, I'll do this in a super messy way
// Basically here I'm declaring two lil functions so I can later call them here
namespace Experiment
{
	void InitDiligentEngine();
	void UpdateDiligentEngine(float frameTime);
}

/*
=================
HUD_DrawNormalTriangles

Non-transparent triangles-- add them here
=================
*/
void DLLEXPORT HUD_DrawNormalTriangles()
{
	//	RecClDrawNormalTriangles();

	gHUD.m_Spectator.DrawOverview();

	if (gHUD.m_flTime > 5.0f)
	{
		static bool DiligentInitialised = false;

		if (DiligentInitialised)
		{
			Experiment::UpdateDiligentEngine(float(gHUD.m_flTimeDelta));
		}
		else 
		{
			DiligentInitialised = true;
			Experiment::InitDiligentEngine();
		}
	}
}


/*
=================
HUD_DrawTransparentTriangles

Render any triangles with transparent rendermode needs here
=================
*/
void DLLEXPORT HUD_DrawTransparentTriangles()
{
	//	RecClDrawTransparentTriangles();


	if (g_pParticleMan)
		g_pParticleMan->Update();
}
