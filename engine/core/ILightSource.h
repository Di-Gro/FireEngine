#pragma once
#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

class RenderPass;

#pragma pack(push, 4)
static struct LightCBuffer {
	Vector3 position;	float param1;
	Vector3 direction;	float param2;
	Vector3 color;		float param3;
	float param4; 
	float param5; 
	float param6; 
	float param7;
};
#pragma pack(pop)


class ILightSource {
public:
	virtual void OnDrawLight(RenderPass* renderPass) = 0;
	virtual LightCBuffer GetCBuffer() = 0;
};