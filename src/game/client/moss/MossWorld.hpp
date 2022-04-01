
#pragma once

#include "MossBlob.hpp"
#include "IMossRenderer.hpp"

class MossWorld final
{
public:
	void Init();
	void Shutdown();

	void Render();
	void Update();
	void AddBlob(const Vector& position, const Vector& normal, const float& angle);

	IMossRenderer* GetRenderer()
	{
		return renderer;
	}

private:
	float updateTimer{0.0f};

	// Future optimisation consideration: split blob clusters up across BSP leaves
	MossBlobVector blobList{};
	MossBlobList watchList{};
	IMossRenderer* renderer{nullptr};
};

// MossWorld.cpp
extern MossWorld gMoss;
