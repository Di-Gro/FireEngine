#pragma once
#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

class Vertex {
public:
	Vector4 position = Vector4::Zero;
	Vector4 color = Vector4::One;
	Vector4 normal = Vector4::One;
	Vector4 uv = Vector4::Zero;

	Vertex() {}
	Vertex(Vector3 pos) {
		position.x = pos.x;
		position.y = pos.y;
		position.z = pos.z;
	}
};