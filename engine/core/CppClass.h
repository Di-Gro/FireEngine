#pragma once

#include <iostream>
#include <iostream>
#include <stddef.h>


#include <d3d11.h>
#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

#include "Refs.h"
#include "ClassInfo.h"
#include "CSBridge.h"


class CppClass {
	OBJECT;

public:
	float floatValue = 1.154f;
	int intValue = 1;

	Vector3 vector3{ 1, 2, 3 };
	Quaternion quat{ 0.1f, 0.2f, 0.3f, 0.4f };

private:
	long m_longValue = 2;
	Vector3 m_vector3{ 4, 5, 6 };

public:
	long longValue() { return m_longValue; }
	void longValue(long value) { m_longValue = value; }

	Vector3 vector3m() { return m_vector3; }
	void vector3m(Vector3 value) { m_vector3 = value; }

	float SomeFunc() { return intValue * floatValue; }
};

PROP_GETSET(CppClass, long, longValue)
PROP_GETSET(CppClass, Vector3, vector3m)

FUNC(CppClass, SomeFunc, float)(size_t objRef);

