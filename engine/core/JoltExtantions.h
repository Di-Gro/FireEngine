#pragma once

#include <Jolt/Jolt.h>
#include "SimpleMath.h"

JPH::Vec3 ToJolt(const Vector3& value) {
	return JPH::Vec3(value.x, value.y, value.z);
}

JPH::Quat ToJolt(const Quaternion& value) {
	return JPH::Quat(value.x, value.y, value.z, value.w);
}

Vector3 FromJolt(JPH::Vec3 value) {
	return Vector3(value.GetX(), value.GetY(), value.GetZ());
}

Quaternion FromJolt(JPH::Quat value) {
	return Quaternion(value.GetX(), value.GetY(), value.GetZ(), value.GetW());
}