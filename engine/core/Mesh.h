#pragma once

#include <vector>

#include <directxmath.h>
#include <d3d11.h>
#include <SimpleMath.h>

#include "wrl.h_d3d11_alias.h"
#include "Shader.h"
#include "Material.h"

using namespace DirectX::SimpleMath;

class DirectionLight;
class Render;

class Mesh4 {
public:
	class Vertex {
	public:
		Vector4 position = Vector4::Zero;
		Vector4 color = Vector4::One;
		Vector4 normal = Vector4::One;
		Vector4 uv = Vector4::Zero;
	};

public:
	struct DynamicData {
		Render* render;
		const std::vector<const Material*>* materials;

		const Matrix* worldMatrix;
		const Matrix* transfMatrix;
		const Vector3* cameraPosition;

		DirectionLight* directionLight;
	};

private:
	struct Shape {
		Vertex* verteces = nullptr;
		int* indeces = nullptr;
		int materialIndex = -1;

		int vertecesSize = 0;
		int indecesSize = 0;

		comptr<ID3D11Buffer> vertexBuffer;
		comptr<ID3D11Buffer> indexBuffer;
		comptr<ID3D11Buffer> constBuffer;
		comptr<ID3D11Buffer> materialConstBuffer;
		comptr<ID3D11Buffer> directionLightCBuffer;
		comptr<ID3D11SamplerState> sampler;
		comptr<ID3D11SamplerState> compSampler;
	};

public:
	mutable D3D_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

private:
	std::vector<Shape> m_shapes;

	UINT m_strides[1] = { sizeof(Vertex) }; // Размер в байтах на каждую вершину
	UINT m_offsets[1] = { 0 };

public:
	Mesh4() {};
	~Mesh4();

	void AddShape(
		std::vector<Vertex>* verteces,
		std::vector<int>* indeces,
		Render* render,
		int materialIndex);

	void Draw(const DynamicData& data) const;

	int shapeCount() { return m_shapes.size(); }

	Shape* GetShape(int index);

};