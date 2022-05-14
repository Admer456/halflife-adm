
#pragma once

namespace Physics
{

// Usage example:
// BodyCreateInfo bci{ BodyCreateInfo::ShapeTypes::Box };
// bci.mShape.box.halfExtents = Vector( 32.0f, 64.0f, 0.5f );
struct BodyCreateInfo
{
	// TODO: separate bodies & shapes

	struct ShapeTypes
	{
		enum EnumType : uint32_t
		{
			Box = 0,
			Sphere,
			Cylinder,
			ConvexHull,
			TriangleMesh
		};
	};

	struct BoxShapeInfo
	{
		Vector halfExtents{0.0f, 0.0f, 0.0f};
	};

	struct CylinderShapeInfo
	{
		float radius{0.0f};
		float height{0.0f};
	};

	struct SphereShapeInfo
	{
		float radius{0.0f};
	};

	struct ConvexHullShapeInfo
	{
		// Todo: some other time
	};

	struct TriangleMeshShapeInfo
	{
		// Todo: some other time
	};

	// The shape type of this body
	ShapeTypes::EnumType mType;
	// Where the body will appear
	Vector mPosition;
	// How the body will initially be oriented
	Vector mAngles;
	// Whether this is a static, dynamic or kinematic body
	JPH::EMotionType mMotionType;
	// Object layer (for game logic purposes & stuff)
	uint8_t mObjectLayer;
	// Shape data
	union ShapeUnion
	{
		BoxShapeInfo box;
		CylinderShapeInfo cylinder;
		SphereShapeInfo sphere;
		ConvexHullShapeInfo hull;
		TriangleMeshShapeInfo mesh;
	} mShape;
};

// Wrapper around some JPH body-related classes
class PhysicsBody final
{
public:
	PhysicsBody(JPH::BodyInterface& bodyInterface)
		: mBodyInterface(bodyInterface)
	{
	
	}

	Vector GetOrigin() const;
	Vector GetAngles() const;
	Vector GetVelocity() const;
	Vector GetAngularVelocity() const;

	void SetOrigin(Vector origin);
	void SetAngles(Vector angles);
	void SetVelocity(Vector velocity);
	void SetAngularVelocity(Vector angularVelocity);

private: 
	friend class PhysicsSystem;

	// Called by PhysicsSystem. If you wanna remove this body,
	// call gPhysicsSystem.RemoveBody( bodyId );
	void Remove();

private:
	JPH::BodyID mBodyId;
	JPH::Body* mBodyPtr{nullptr};
	// There may be multiple physics worlds, so we use their body interface
	JPH::BodyInterface& mBodyInterface;
};

}
