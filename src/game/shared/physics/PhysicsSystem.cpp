
#include "shared_utils.h"
#include "physics/Physics.hpp"

using namespace Physics;

PhysicsSystem::PhysicsSystem(int tempAllocatorMB)
	: mTempAllocator(tempAllocatorMB * 1024 * 1024)
{

}

PhysicsSystem::~PhysicsSystem()
{
	Shutdown();
}

bool PhysicsSystem::Init(JPH::DebugRenderer* debugRenderer)
{
	// Print into the developer console
	JPH::Trace = Con_Printf;
	
	// Register all JoltPhysics types
	if (nullptr == JPH::Factory::sInstance)
	{
		JPH::Factory::sInstance = new JPH::Factory();
	}
	JPH::RegisterTypes();
	// Initialise the physics system and set gravity
	mSystem.Init(MaxBodies, NumBodyMutexes, MaxBodyPairs, MaxContactConstraints,
		mBroadphaseLayerInterface, MyBroadPhaseCanCollide, MyObjectCanCollide);
	
	SetGravity({0.0f, 0.0f, -9.81f});

	// Begin with 64 phys body slots
	if (!ExpandBodySlots(64U))
	{
		return false;
	}

	JPH::Trace("PhysicsSystem::Init: initialised physics system\n");

	mDebugRenderer.reset(debugRenderer);
	if (mDebugRenderer)
	{
		JPH::Trace("PhysicsSystem::Init: attached debug renderer\n");
	}

	mJobSystem = std::make_unique<JPH::JobSystemThreadPool>(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, 4);

	return true;
}

void PhysicsSystem::Shutdown()
{
	// Already shut down, don't do it again
	if (mBodies.capacity() == 0)
	{
		return;
	}

	mJobSystem.reset();

	for (int32_t id = 0; id < mBodies.size(); id++)
	{
		RemoveBody(id);
	}

	mBodies.clear();
	mBodies.shrink_to_fit();

	JPH::Trace("PhysicsSystem::Shutdown\n");

	delete JPH::Factory::sInstance;
	JPH::Factory::sInstance = nullptr;
}

void PhysicsSystem::Update(const float& deltaTime, const int& numCollisionSteps, const int& numIntegrationSteps)
{
	// We gotta rebuild the broadphase every time new bodies are added
	// This is pretty performance costly, so we don't do it every frame,
	// and neither do we do it every time a new phys body spawns.
	// Instead, we do it at the beginning of the physics frame, when we're
	// sure that X new objects have spawned
	if (mBroadphaseNeedUpdate && mBroadphaseUpdateTimer <= 0.0f)
	{
		mSystem.OptimizeBroadPhase();

		mBroadphaseUpdateTimer = 0.5f;
		mBroadphaseNeedUpdate = false;
	}

	mBroadphaseUpdateTimer -= deltaTime;

	mSystem.Update(deltaTime, numCollisionSteps, numIntegrationSteps, &mTempAllocator, mJobSystem.get());
}

void PhysicsSystem::Render()
{
#ifdef CLIENT_DLL
	if (!mDebugRenderer)
	{
		return;
	}

	JPH::BodyManager::DrawSettings ds;
	ds.mDrawShape = true;
	//ds.mDrawShapeWireframe = true;
	ds.mDrawBoundingBox = true;

	mSystem.DrawBodies(ds, mDebugRenderer.get());
#else
	ALERT(at_error, "PhysicsSystem::Render() called on server\n");
#endif
}

void PhysicsSystem::SetGravity(Vector newGravity)
{
	mSystem.SetGravity(convert(newGravity));
}

int32_t PhysicsSystem::CreateBody(BodyCreateInfo& createInfo)
{
	int32_t id;
	for ( id = 0; id < mBodies.size(); id++ )
	{
		if (!mBodies[id].mOccupied)
		{
			return CreateBodyInternal(id, createInfo);
		}
	}

	// No free slots have been found, add some more. The physics 
	// engine will crash if we proceed to add more than the allowed 
	// number of bodies, so bail out if that happens
	if ( !ExpandBodySlots(32U) )
	{
		return -1;
	}

	// id is already at mBodies.max_size
	return CreateBodyInternal(id, createInfo);
}

PhysicsBody* PhysicsSystem::GetBodyById(const int32_t& id)
{
	if (id >= 0 && id < mBodies.size() && mBodies[id].mOccupied)
	{
		return &mBodies[id].mBody;
	}

	return nullptr;
}

void PhysicsSystem::RemoveBody(const int32_t& id)
{
	if (id < 0 || id >= mBodies.size())
	{
		return;
	}

	if (!mBodies[id].mOccupied)
	{
		return;
	}

	mBodies[id].mBody.Remove();
	mBodies[id].mOccupied = false;
}

int32_t PhysicsSystem::CreateBodyInternal(const int32_t& id, BodyCreateInfo& createInfo)
{
	if (id < 0 || id >= MaxBodies)
	{
		return -1;
	}

	PhysicsBody& body = mBodies[id].mBody;

	// Todo: separation, separation, SEPARATION OF LOGIC AAAAAAA
	// This is fine for now, but will be painful when I support multiple shapes
	// Also, I just realised now, we gotta separate bodies & shapes, so bodies can reuse em
	JPH::ShapeSettings::ShapeResult shapeResult;

	if (createInfo.mType == BodyCreateInfo::ShapeTypes::Box)
	{
		JPH::BoxShapeSettings shapeSettings(utom(convert(createInfo.mShape.box.halfExtents)));
		shapeResult = shapeSettings.Create();
	}
	else
	{
		JPH::Trace("PhysicsSystem::CreateBodyInternal: only Box shapes are supported\n");
		return -1;
	}

	JPH::BodyCreationSettings bodySettings(
		shapeResult.Get(),
		utom(convert(createInfo.mPosition)),
		// Todo: thoroughly test this to make sure orientations are consistent and right between GoldSRC and JoltPhysics
		JPH::Quat::sEulerAngles(convert(createInfo.mAngles) * (M_PI / 180.0f)),
		createInfo.mMotionType, createInfo.mObjectLayer);

	body.mBodyPtr = mBodyInterface.CreateBody(bodySettings);
	body.mBodyId = body.mBodyPtr->GetID();
	mBodyInterface.AddBody(body.mBodyId,
		createInfo.mMotionType == JPH::EMotionType::Static ? JPH::EActivation::DontActivate : JPH::EActivation::Activate);

	// A body has been added, should probably regenerate the broadphase
	mBroadphaseNeedUpdate = true;
	mBodies[id].mOccupied = true;

	return id;
}

bool PhysicsSystem::ExpandBodySlots(size_t howMany)
{
	const size_t newSize = mBodies.capacity() + howMany;
	if (newSize >= MaxBodies)
	{
		return false;
	}

	mBodies.reserve(newSize);
	for (int i = 0; i < howMany; i++)
	{
		mBodies.emplace_back(mBodyInterface);
	}

	return true;
}
