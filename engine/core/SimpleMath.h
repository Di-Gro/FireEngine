#pragma once

#include <d3d11.h>
#include <SimpleMath.h>
#include <string>

using namespace DirectX::SimpleMath;

extern std::string ToString(const DirectX::SimpleMath::Vector2& vec);
extern std::string ToString(const DirectX::SimpleMath::Vector3& vec);
extern std::string ToString(const DirectX::SimpleMath::Quaternion& vec);

extern inline void FixNaN(DirectX::SimpleMath::Vector3& value);
extern inline void FixNaN(DirectX::SimpleMath::Quaternion& value);

extern inline void FixNaN(DirectX::SimpleMath::Matrix& value);

extern inline bool IsNaN(DirectX::SimpleMath::Matrix& value);


extern float VecMin(const DirectX::SimpleMath::Vector3& vec);
extern float VecMax(const DirectX::SimpleMath::Vector3& vec);