
#include "hud.h"
#include "pm_defs.h"
#include "pmtrace.h"
#include "MossWorld.hpp"
#include "MossRenderer.hpp"

MossWorld gMoss;

void MossWorld::Init()
{
	renderer = MossRenderer::GetInstance();
	renderer->Init();

	gEngfuncs.pfnAddCommand("moss_spawn", []() 
		{ 
			const Vector trStart = gHUD.m_vecOrigin;
			const Vector trEnd = trStart - Vector(0.0f, 0.0f, 100.0f);

			const pmtrace_s* trace = gEngfuncs.PM_TraceLine(trStart, trEnd, PM_TRACELINE_PHYSENTSONLY, 2, -1);
			if (nullptr == trace)
			{
				return;
			}

			gMoss.AddBlob(trace->endpos, trace->plane.normal, RANDOM_FLOAT(0.0f, 360.0f));
		});
}

void MossWorld::Shutdown()
{
	renderer->Shutdown( "Shutting down all subsystems" );
}

void MossWorld::Render()
{
	if (renderer->Okay())
	{
		renderer->RenderFrame(blobList);
	}
}

void MossWorld::Update()
{
	const float& deltaTime = gHUD.m_flTimeDelta;

	updateTimer -= deltaTime;
	if (updateTimer > 0.0f)
	{
		return;
	}

	updateTimer = 0.1f;

	for (auto& blob : blobList)
	{
		blob.Update( deltaTime );
	}
}

void MossWorld::AddBlob(const Vector& position, const Vector& normal, const float& angle)
{
	blobList.emplace_back(position, normal, 0.5f, angle);
	watchList.push_back(&blobList.back());
}
