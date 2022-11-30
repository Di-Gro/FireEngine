#include "CS.h"


CS::float2 CS::ToCS(const DirectX::SimpleMath::Vector2& source) {
	CS::float2 tmp;
	tmp._1 = source.x;
	tmp._2 = source.y;
	return tmp;
}

CS::float3 CS::ToCS(const DirectX::SimpleMath::Vector3& source) {
	CS::float3 tmp;
	tmp._1 = source.x;
	tmp._2 = source.y;
	tmp._3 = source.z;
	return tmp;
}

CS::float4 CS::ToCS(const DirectX::SimpleMath::Vector4& source) {
	CS::float4 tmp;
	tmp._1 = source.x;
	tmp._2 = source.y;
	tmp._3 = source.z;
	tmp._4 = source.w;
	return tmp;
}

CS::float4 CS::ToCS(const DirectX::SimpleMath::Quaternion& source) {
	CS::float4 tmp;
	tmp._1 = source.x;
	tmp._2 = source.y;
	tmp._3 = source.z;
	tmp._4 = source.w;
	return tmp;
}