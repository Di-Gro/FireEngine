#pragma once

#include <vector>

#include <directxmath.h>
#include <d3d11.h>
#include <SimpleMath.h>

#include "wrl.h_d3d11_alias.h"
#include "Shader.h"
#include "Material.h"
#include "CsLink.h"
#include "CSBridge.h"

using namespace DirectX::SimpleMath;

class DirectionLight;
class Render;
class MeshAsset;
class MeshComponent;

class Mesh4 : public CsLink {
	friend class MeshAsset;
	friend class MeshComponent;

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

	struct DynamicShapeData {
		Render* render;
		const Material* material;

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
	size_t f_assetHash = 0;
	std::vector<Shape> m_shapes;
	Render* m_render;

	UINT m_strides[1] = { sizeof(Vertex) }; // ������ � ������ �� ������ �������
	UINT m_offsets[1] = { 0 };

public:
	Mesh4() {};
	Mesh4(const Mesh4& other);
	~Mesh4();


	void AddShape(
		std::vector<Vertex>* verteces,
		std::vector<int>* indeces,
		Render* render,
		int materialIndex);

	void AddShape(
		Vertex* verteces,
		int vertecesLength,
		int* indeces,
		int indecesLength,
		Render* render,
		int materialIndex);

	void m_InitShape(Mesh4::Shape& shape);


	void Draw(const DynamicData& data) const;
	void DrawShape(const DynamicData& data, int index) const;

	int shapeCount() const { return m_shapes.size(); }

	int maxMaterialIndex() const;

	Shape* GetShape(int index);

};

class ScreenQuad {
public:
	mutable D3D_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

	ID3D11ShaderResourceView* deffuseSRV = nullptr;

private:
	Render* m_render;
	const Shader* m_shader;
	comptr<ID3D11SamplerState> m_sampler;
	
public:

	void Init(Render* render, const Shader* shader);
	void Draw() const;
};

FUNC(Mesh4, ShapeCount, int)(CppRef mesh4Ref);
FUNC(Mesh4, MaterialMaxIndex, int)(CppRef mesh4Ref);