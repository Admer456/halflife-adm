
#pragma once

namespace Physics
{

using JPH::uint;
using std::unique_ptr;
using JPH::BroadPhaseLayer;
using JPH::ObjectLayer;

constexpr uint MaxBodies = 1024;
constexpr uint MaxBodyPairs = 1024;
constexpr uint MaxContactConstraints = 1024;
constexpr uint NumBodyMutexes = 0;

// Layer that objects can be in, determines which other objects it can collide with
// Typically you at least want to have 1 layer for moving bodies and 1 layer for static bodies, but you can have more
// layers if you want. E.g. you could have a layer for high detail collision (which is not used by the physics simulation
// but only if you do collision testing).
namespace Layers
{
	static constexpr uint8_t NON_MOVING = 0;
	static constexpr uint8_t MOVING = 1;
	static constexpr uint8_t NUM_LAYERS = 2;
};

// Function that determines if two object layers can collide
static bool MyObjectCanCollide(ObjectLayer inObject1, ObjectLayer inObject2)
{
	using namespace JPH;

	switch (inObject1)
	{
	case Layers::NON_MOVING:
		return inObject2 == Layers::MOVING; // Non moving only collides with moving
	case Layers::MOVING:
		return true; // Moving collides with everything
	default:
		JPH_ASSERT(false);
		return false;
	}
};

// Each broadphase layer results in a separate bounding volume tree in the broad phase. You at least want to have
// a layer for non-moving and moving objects to avoid having to update a tree full of static objects every frame.
// You can have a 1-on-1 mapping between object layers and broadphase layers (like in this case) but if you have
// many object layers you'll be creating many broad phase trees, which is not efficient. If you want to fine tune
// your broadphase layers define JPH_TRACK_BROADPHASE_STATS and look at the stats reported on the TTY.
namespace BroadPhaseLayers
{
	static constexpr BroadPhaseLayer NON_MOVING(0);
	static constexpr BroadPhaseLayer MOVING(1);
	static constexpr uint NUM_LAYERS(2);
};

// BroadPhaseLayerInterface implementation
// This defines a mapping between object and broadphase layers.
class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
{
public:
	BPLayerInterfaceImpl()
	{
		// Create a mapping table from object to broad phase layer
		mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
		mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
	}

	virtual uint GetNumBroadPhaseLayers() const override
	{
		return BroadPhaseLayers::NUM_LAYERS;
	}

	virtual BroadPhaseLayer GetBroadPhaseLayer(ObjectLayer inLayer) const override
	{
		using namespace JPH;
		JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
		return mObjectToBroadPhase[inLayer];
	}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
	virtual const char* GetBroadPhaseLayerName(BroadPhaseLayer inLayer) const override
	{
		using namespace JPH;

		switch ((BroadPhaseLayer::Type)inLayer)
		{
		case (BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING: return "NON_MOVING";
		case (BroadPhaseLayer::Type)BroadPhaseLayers::MOVING: return "MOVING";
		default: JPH_ASSERT(false); return "INVALID";
		}
	}
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

private:
	BroadPhaseLayer mObjectToBroadPhase[Layers::NUM_LAYERS];
};

// Function that determines if two broadphase layers can collide
static bool MyBroadPhaseCanCollide(ObjectLayer inLayer1, BroadPhaseLayer inLayer2)
{
	using namespace JPH;

	switch (inLayer1)
	{
	case Layers::NON_MOVING:
		return inLayer2 == BroadPhaseLayers::MOVING;
	case Layers::MOVING:
		return true;
	default:
		JPH_ASSERT(false);
		return false;
	}
}

class PhysicsBodySlot final
{
public:
	PhysicsBodySlot(JPH::BodyInterface& bodyInterface)
		: mBody(bodyInterface)
	{
	
	}

	bool			mOccupied{false};
	PhysicsBody		mBody;
};

// The GoldSRC-wrapped physics system. One instance is 
// on the clientside, and another is on the serverside
class PhysicsSystem final
{
public:
	PhysicsSystem(int tempAllocatorMB = 10);
	~PhysicsSystem();

	// Initialises everything
	bool			Init(JPH::DebugRenderer* debugRenderer);
	// Clear all bodies etc.
	void			Shutdown();

	// Call every frame
	void			Update(const float& deltaTime, const int& numCollisionSteps, const int& numIntegrationSteps);
	// Call from somewhere like HUD_DrawTransparentTriangles
	// Only available on the clientside!
	void			Render();

	// Sets the global gravity
	void			SetGravity(Vector newGravity);

	// Creates a physics body and returns a handle to it
	// -1 on failure or invalid create info parameters
	int32_t			CreateBody(BodyCreateInfo& createInfo);
	// Gets a body from this ID, obtained by CreateBody
	PhysicsBody*	GetBodyById(const int32_t& id);
	// Deletes a body that has this ID
	void			RemoveBody(const int32_t& id);

private:
	int32_t			CreateBodyInternal(const int32_t& id, BodyCreateInfo& createInfo);
	bool			ExpandBodySlots(size_t howMany = 32U);

private:
	// Higher-level collision checking
	BPLayerInterfaceImpl mBroadphaseLayerInterface;
	// Debug wireframe overlay
	unique_ptr<JPH::DebugRenderer> mDebugRenderer;
	// This here must be destroyed before SDL2 shuts down
	unique_ptr<JPH::JobSystemThreadPool> mJobSystem;
	// Allocates some temporary stuff for whatever the phys engine needs
	JPH::TempAllocatorImpl mTempAllocator;

	// Everything
	JPH::PhysicsSystem mSystem;
	// A lil shortcut to the physics engine's body manager
	JPH::BodyInterface& mBodyInterface{mSystem.GetBodyInterface()};

	// The bodies(tm)
	std::vector<PhysicsBodySlot> mBodies;

	bool mBroadphaseNeedUpdate{true};
	float mBroadphaseUpdateTimer{0.0f};
};

}

inline Physics::PhysicsSystem gPhysics;
