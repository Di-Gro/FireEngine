#pragma once

#include <vector>

#include <directxmath.h>
#include <d3d11.h>
#include <SimpleMath.h>

#include "wrl.h_d3d11_alias.h"

#include "Shader.h"
#include "MaterialResource.h"
#include "ShaderResource.h"
#include "MeshShape.h"

using namespace DirectX::SimpleMath;

namespace tinyobj {
	struct shape_t;
	struct attrib_t;
	struct material_t;
	struct index_t;
}

class Render;

struct MeshShaderData {
	Render* render;
	const Matrix* worldMatrix;
	const Matrix* transfMatrix;
	const Vector3* cameraPosition;
};

class MeshResource {
	friend class MeshAssets;
	friend class MeshComponent;

public:
	mutable D3D_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	int version = 0;

private:
	// Предустановленные материалы меша. Могуть отсутствовать. 
	
	std::vector<MeshShape> m_shapes;
	Render* m_render;

	UINT m_strides[1] = { sizeof(Vertex) };
	UINT m_offsets[1] = { 0 };

public:
	MeshResource() {};
	MeshResource(const MeshResource& other);
	MeshResource(MeshResource&& other) noexcept;
	MeshResource& operator=(const MeshResource& other);
	MeshResource& operator=(MeshResource&& other) noexcept;
	~MeshResource();

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

	void m_InitShape(MeshShape& shape);

	void Draw(const MeshShaderData& data) const;
	void DrawShape(const MeshShaderData& data, int index) const;

	int shapeCount() const { return (int)m_shapes.size(); }

	int maxMaterialIndex() const;

	MeshShape* GetShape(int index);
	const MeshShape* GetConstShape(int index) const;
	void Release();

public:
	static MeshResource CreateFromObj(Render* render, const fs::path& path);

	static void s_InitMesh(Render* render, MeshResource* mesh, const tinyobj::attrib_t& attrib, const std::vector<tinyobj::shape_t>& shapes);
	static void s_InitShape(Render* render, MeshResource* mesh, const tinyobj::attrib_t& attrib, const tinyobj::shape_t& shape);
	
	static Vertex s_ReadVertex(const tinyobj::attrib_t& attrib, const tinyobj::index_t& index);
};