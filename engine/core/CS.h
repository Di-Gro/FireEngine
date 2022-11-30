#pragma once

#include "SimpleMath.h"


namespace CS {

	struct float2 { float _1; float _2; };
	struct float3 { float _1; float _2; float _3; };
	struct float4 { float _1; float _2; float _3; float _4; };

	float2 ToCS(const DirectX::SimpleMath::Vector2& source);
	float3 ToCS(const DirectX::SimpleMath::Vector3& source);
	float4 ToCS(const DirectX::SimpleMath::Vector4& source);
	float4 ToCS(const DirectX::SimpleMath::Quaternion& source);
}