
#pragma once

// The Jolt headers don't include Jolt.h. Always include Jolt.h before including any other Jolt header.
// You can use Jolt.h in your precompiled header to speed up compilation.
#include <Jolt/Jolt.h>

// Jolt includes
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>

namespace Physics
{

constexpr float UnitsPerMetre = 39.37007874f;

// Convert Hammer units into metres
template<class T>
inline T utom(const T& x)
{
	return x / UnitsPerMetre;
}

// Convert metres into Hammer units
template<class T>
inline T mtou(const T& x)
{
	return x * UnitsPerMetre;
}

inline Vector convert(JPH::Vec3Arg v)
{
	return &v.mF32[0];
}

inline JPH::Vec3 convert(Vector v)
{
	return JPH::Vec3(v.x, v.y, v.z);
}

inline Vector convert(JPH::Vec4Arg v)
{
	return &v.mF32[0];
}

inline JPH::Vec4 convert(Vector v, float w)
{
	return JPH::Vec4(v.x, v.y, v.z, w);	
}

}

#include "PhysicsBody.hpp"
#include "PhysicsSystem.hpp"
