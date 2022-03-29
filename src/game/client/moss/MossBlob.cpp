
#include "hud.h"
#include "MossBlob.hpp"

MossBlob::MossBlob(const Vector& blobPosition, const Vector& blobNormal, const float& blobScale, const float& blobAngle)
	: position(blobPosition), normal(blobNormal), scale(blobScale), angle(blobAngle)
{

}

void MossBlob::Update(const float& deltaTime)
{
	if (scale >= 1.0f)
	{
		canSpawnChildren = true;
		return;
	}

	scale += deltaTime;
}

bool MossBlob::CanSpawnChildren() const
{
	return canSpawnChildren;
}
