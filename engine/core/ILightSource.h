#pragma once
#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

#pragma pack(push, 4)
static struct LightCBuffer {
	Vector3 position; float param1;
	Vector3 direction; float param2;
	Vector3 color; float param3;
};
#pragma pack(pop)


class ILightSource {
public:
	virtual void OnDrawLight() = 0;
	virtual LightCBuffer GetCBuffer() = 0;
};