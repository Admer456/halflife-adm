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
}

#include "physics/Physics.hpp"

/*
=================
HUD_DrawTransparentTriangles

Render any triangles with transparent rendermode needs here
=================
*/
void DLLEXPORT HUD_DrawTransparentTriangles()
{
	//	RecClDrawTransparentTriangles();

	auto& TriApi = gEngfuncs.pTriAPI;

	TriApi->Begin(TRI_LINES);
	TriApi->Vertex3f(0.0f, 128.0f, 64.0f);
	TriApi->Vertex3f(128.0f, 128.0f, 64.0f);
	TriApi->Vertex3f(128.0f, 128.0f, 64.0f);
	TriApi->Vertex3f(128.0f, 0.0f, 64.0f);
	TriApi->Vertex3f(128.0f, 0.0f, 64.0f);
	TriApi->Vertex3f(0.0f, 128.0f, 64.0f);
	TriApi->End();

	gPhysics.Render();

	if (g_pParticleMan)
		g_pParticleMan->Update();
}
