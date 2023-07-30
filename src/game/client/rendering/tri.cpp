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
#include "view.h"
#include "GLRenderer.hpp"

void RenderFog()
{
	if (g_WaterLevel <= WaterLevel::Feet && g_FogDensity >= 0)
	{
		gEngfuncs.pTriAPI->FogParams(g_FogDensity, g_FogSkybox);
		gEngfuncs.pTriAPI->Fog(g_FogColor, g_FogStartDistance, g_FogStopDistance, int(g_RenderFog));
	}
}

/**
 *	@brief Non-transparent triangles-- add them here
 */
void DLLEXPORT HUD_DrawNormalTriangles()
{
	gHUD.m_Spectator.DrawOverview();
}

/**
 *	@brief Render any triangles with transparent rendermode needs here
 */
void DLLEXPORT HUD_DrawTransparentTriangles()
{
	if (g_pParticleMan)
		g_pParticleMan->Update();

	if (GLRenderer::GetInstance()->Okay())
	{
		GLRenderer::GetInstance()->RenderFrame();
	}

	// Handled in V_CalcRefdef.
	// RenderFog();
}
