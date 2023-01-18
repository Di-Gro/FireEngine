#pragma once

#include "SimpleMath.h"

struct Contact {
	Vector3 point1;
	Vector3 point2;
	Vector3 relativePoint1;
	Vector3 relativePoint2;
	Vector3 normal;
	Vector3 offset;
	float depth;
};