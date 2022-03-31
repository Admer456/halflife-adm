
#pragma once

#include <vector>
#include <list>

class MossBlob final
{
public:
	MossBlob(const Vector& blobPosition, const Vector& blobNormal, const float& blobScale, const float& blobAngle);

	void Update(const float& deltaTime);
	bool CanSpawnChildren() const;

	const Vector& GetPosition() const
	{
		return position;
	}

	const Vector& GetNormal() const
	{
		return normal;
	}

	const float& GetScale() const
	{
		return scale;
	}

	const float& GetAngle() const
	{
		return angle;
	}

private:
	Vector position{0.0f, 0.0f, 0.0f};
	Vector normal{0.0f, 0.0f, 1.0f};
	float scale{1.0f};
	float angle{0.0f}; // rotation around the normal
	bool canSpawnChildren{false}; // reached when scale reaches a certain threshold
};

using MossBlobVector = std::vector<MossBlob>;
using MossBlobList = std::list<MossBlob*>;
