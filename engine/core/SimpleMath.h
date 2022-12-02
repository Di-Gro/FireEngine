#pragma once

#include <d3d11.h>
#include <SimpleMath.h>
#include <string>

using namespace DirectX::SimpleMath;

extern std::string ToString(const Vector3& vec);
extern std::string ToString(const Quaternion& vec);

extern inline void FixNaN(Vector3& value);
extern inline void FixNaN(Quaternion& value);
extern inline void FixNaN(Matrix& value);

extern inline bool IsNaN(Matrix& value);