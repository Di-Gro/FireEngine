#pragma once

#include <d3d11.h>
#include <SimpleMath.h>
#include <string>

using namespace DirectX::SimpleMath;

extern std::string ToString(const Vector3& vec);
extern std::string ToString(const Quaternion& vec);