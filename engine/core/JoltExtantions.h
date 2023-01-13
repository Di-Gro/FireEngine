#pragma once

#include <Jolt/Jolt.h>
#include "SimpleMath.h"

extern JPH::Vec3 ToJolt(const Vector3& value);
extern JPH::Quat ToJolt(const Quaternion& value);

extern Vector3 FromJolt(JPH::Vec3 value);
extern Quaternion FromJolt(JPH::Quat value);