
#include "Physics.hpp"

using namespace Physics;

Vector PhysicsBody::GetOrigin() const
{
	return mtou(convert(mBodyPtr->GetPosition()));
}

Vector PhysicsBody::GetAngles() const
{
	return convert(mBodyPtr->GetRotation().GetEulerAngles()) * (180.0f / M_PI);
}

Vector PhysicsBody::GetVelocity() const
{
	return mtou(convert(mBodyPtr->GetLinearVelocity()));
}

Vector PhysicsBody::GetAngularVelocity() const
{
	return mtou(convert(mBodyPtr->GetAngularVelocity())) * (180.0f / M_PI);
}

void PhysicsBody::SetOrigin(Vector origin)
{
	mBodyInterface.SetPosition(mBodyId, utom(convert(origin)), JPH::EActivation::Activate);
}

void PhysicsBody::SetAngles(Vector angles)
{
	mBodyInterface.SetRotation(mBodyId, JPH::Quat::sEulerAngles(convert(angles) * (M_PI / 180.0f)), JPH::EActivation::Activate);
}

void PhysicsBody::SetVelocity(Vector velocity)
{
	mBodyPtr->SetLinearVelocity(utom(convert(velocity)));
}

void PhysicsBody::SetAngularVelocity(Vector angularVelocity)
{
	mBodyPtr->SetAngularVelocity((convert(angularVelocity)) * (M_PI / 180.0f));
}

void PhysicsBody::Remove()
{
	if (nullptr == mBodyPtr)
	{
		return;
	}

	mBodyInterface.RemoveBody(mBodyId);
	mBodyPtr = nullptr;
}
