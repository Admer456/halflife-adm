
#pragma once

#include "MossBlob.hpp"
#include "IMossRenderer.hpp"

class MossWorld final
{
public:
	void Init();
	void Shutdown();

	void AddBlob( const Vector& position, const Vector& normal, const float& angle );

private:
	// Future optimisation consideration: split blob clusters up across BSP leaves
	MossBlobList blobList{};
	IMossRenderer& renderer;
};
