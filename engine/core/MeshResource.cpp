#include "MeshResource.h"

#include "tiny_obj_loader.h"

#include "Game.h"
#include "Assets.h"
#include "MeshAssets.h"
#include "Render.h"
#include "RenderPass.h"
#include "DirectionLight.h"
#include "CameraComponent.h"
#include "ShaderStructs.h"

using namespace ShaderStructs;

MeshResource::MeshResource(const MeshResource& other) {
	*this = other;
}

MeshResource::MeshResource(MeshResource&& other) noexcept {
	*this = std::move(other);
}

MeshResource& MeshResource::operator=(const MeshResource& other) {
	if (&other == nullptr || &other == this)
		return *this;

	topology = other.topology;
	version = other.version;
	m_render = other.m_render;

	for (auto& shape : other.m_shapes) {
		std::vector<Vertex> verteces;
		std::vector<int> indeces;

		verteces.reserve(shape.vertecesSize);
		indeces.reserve(shape.indecesSize);

		for (int i = 0; i < shape.vertecesSize; i++)
			verteces.push_back(shape.verteces[i]);

		for (int i = 0; i < shape.indecesSize; i++)
			indeces.push_back(shape.indeces[i]);

		this->AddShape(&verteces, &indeces, other.m_render, shape.materialIndex);
	}
	return *this;
}

MeshResource& MeshResource::operator=(MeshResource&& other) noexcept {
	if (this == &other)
		return *this;

	topology = other.topology;
	version = other.version;
	m_render = other.m_render;
	m_shapes = std::move(other.m_shapes);

	other.m_render = nullptr;

	return *this;
}

MeshResource::~MeshResource() {
	Release();
}

void MeshResource::Release() {
	for (auto& shape : m_shapes) {
		if (shape.verteces != nullptr)
			delete[] shape.verteces;

		if (shape.indeces != nullptr)
			delete[] shape.indeces;
	}
	m_shapes.clear();
}

void MeshResource::AddShape(
	Vertex* verteces,
	int vertecesLength,
	int* indeces,
	int indecesLength,
	Render* render,
	int materialIndex)
{
	m_render = render;

	m_shapes.emplace_back();
	auto& shape = m_shapes.back();

	shape.verteces = new Vertex[vertecesLength];
	std::copy(verteces, verteces + vertecesLength, shape.verteces);

	shape.indeces = new int[indecesLength];
	std::copy(indeces, indeces + indecesLength, shape.indeces);

	shape.vertecesSize = vertecesLength;
	shape.indecesSize = indecesLength;
	shape.indexBuffer = nullptr;
	shape.vertexBuffer = nullptr;
	shape.meshCBuffer = nullptr;
	shape.materialIndex = materialIndex;

	m_InitShape(shape);
}


void MeshResource::AddShape(
	std::vector<Vertex>* verteces,
	std::vector<int>* indeces,
	Render* render,
	int materialIndex)
{
	m_render = render;

	m_shapes.emplace_back();
	auto& shape = m_shapes.back();

	shape.verteces = new Vertex[verteces->size()];
	std::copy(verteces->begin(), verteces->end(), shape.verteces);

	shape.indeces = new int[indeces->size()];
	std::copy(indeces->begin(), indeces->end(), shape.indeces);

	shape.vertecesSize = verteces->size();
	shape.indecesSize = indeces->size();
	shape.indexBuffer = nullptr;
	shape.vertexBuffer = nullptr;
	shape.meshCBuffer = nullptr;
	shape.materialIndex = materialIndex;

	m_InitShape(shape);
}

void MeshResource::m_InitShape(MeshShape& shape) {
	D3D11_BUFFER_DESC vertexBufferDesc = {};
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * shape.vertecesSize;

	D3D11_SUBRESOURCE_DATA vertexData = {};
	vertexData.pSysMem = shape.verteces;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	D3D11_BUFFER_DESC indexBufferDesc = {};
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.BindFlags = D3D10_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;
	indexBufferDesc.ByteWidth = sizeof(int) * shape.indecesSize;

	D3D11_SUBRESOURCE_DATA indexData = {};
	indexData.pSysMem = shape.indeces;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	D3D11_BUFFER_DESC constBufferDesc = {};
	constBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	constBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constBufferDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
	constBufferDesc.MiscFlags = 0;
	constBufferDesc.StructureByteStride = 0;
	constBufferDesc.ByteWidth = sizeof(MeshCBuffer);

	auto device = m_render->device();

	m_render->device()->CreateBuffer(&vertexBufferDesc, &vertexData, shape.vertexBuffer.GetAddressOf());
	m_render->device()->CreateBuffer(&indexBufferDesc, &indexData, shape.indexBuffer.GetAddressOf());
	m_render->device()->CreateBuffer(&constBufferDesc, nullptr, shape.meshCBuffer.GetAddressOf());
}

void MeshResource::Draw(const MeshShaderData& data) const {
	for (int index = 0; index < m_shapes.size(); ++index)
		DrawShape(data, index);
}

void MeshResource::DrawShape(const MeshShaderData& data, int index) const {
	auto* context = data.render->context();
	auto& shape = m_shapes[index];

	ID3D11VertexShader* currentShade[] = { nullptr };
	context->VSGetShader(currentShade, nullptr, 0);
	assert(currentShade[0] != nullptr);

	/// Mesh: SetTopology
	context->IASetPrimitiveTopology(topology);

	/// Mesh: SetInput
	context->IASetIndexBuffer(shape.indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	context->IASetVertexBuffers(0, 1, shape.vertexBuffer.GetAddressOf(), m_strides, m_offsets);

	/// Mesh: SetConstBuffers
	context->VSSetConstantBuffers(PASS_CB_MESH_VS, 1, shape.meshCBuffer.GetAddressOf());
	context->PSSetConstantBuffers(PASS_CB_MESH_PS, 1, shape.meshCBuffer.GetAddressOf());

	D3D11_MAPPED_SUBRESOURCE res = {};
	context->Map(shape.meshCBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &res);

	auto* cbuf = (MeshCBuffer*)res.pData;
	cbuf->wvpMatrix = *data.transfMatrix;
	cbuf->worldMatrix = *data.worldMatrix;
	cbuf->cameraPosition = *data.cameraPosition;
	//cbuf->absLocalMatrix = *data.absLocalMatrix;

	context->Unmap(shape.meshCBuffer.Get(), 0);

	/// Mesh: Draw
	context->DrawIndexed(shape.indecesSize, 0, 0);
}


MeshShape* MeshResource::GetShape(int index) {
	return &m_shapes[index];
}

const MeshShape* MeshResource::GetConstShape(int index) const {
	return &m_shapes[index];
}

int MeshResource::maxMaterialIndex() const {
	int maxIndex = 0;
	for (int i = 0; i < m_shapes.size(); i++) {
		if (m_shapes[i].materialIndex > maxIndex)
			maxIndex = m_shapes[i].materialIndex;
	}
	return maxIndex;
}


MeshResource MeshResource::CreateFromObj(Render* render, const fs::path& path) {
	auto dir = path.parent_path().string();

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string warn, err;
	bool res = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.string().c_str(), dir.c_str());
	assert(res);

	MeshResource meshRes;

	s_InitMesh(render , &meshRes, attrib, shapes);

	if (err != "")
		std::cout << err << std::endl;

	//if (warn != "")
	//	std::cout << warn << std::endl;
	
	return meshRes;
}

void MeshResource::s_InitShape(
	Render* render,
	MeshResource* mesh,
	const tinyobj::attrib_t& attrib,
	const tinyobj::shape_t& shape)
{
	auto& verts = attrib.vertices;

	std::vector<Vertex> verteces;
	std::vector<int> indeces;

	verteces.reserve(verts.size() / 3);
	indeces.reserve(shape.mesh.indices.size());

	int faceSize = 3;
	for (int i = 0; i < shape.mesh.indices.size(); i += faceSize) {
		auto vertex1 = s_ReadVertex(attrib, shape.mesh.indices[i + 0]);
		auto vertex2 = s_ReadVertex(attrib, shape.mesh.indices[i + 1]);
		auto vertex3 = s_ReadVertex(attrib, shape.mesh.indices[i + 2]);

		verteces.insert(verteces.end(), { vertex1 , vertex2, vertex3 });
		indeces.insert(indeces.end(), { i + 2, i + 1, i + 0 });
	}

	int matIndex = shape.mesh.material_ids[0];
	matIndex = matIndex >= 0 ? matIndex : 0;

	mesh->AddShape(&verteces, &indeces, render, matIndex);
}

void MeshResource::s_InitMesh(
	Render* render,
	MeshResource* mesh,
	const tinyobj::attrib_t& attrib,
	const std::vector<tinyobj::shape_t>& shapes)
{
	for (auto& shape : shapes) {
		s_InitShape(render, mesh, attrib, shape);
	}
}

Vertex MeshResource::s_ReadVertex(const tinyobj::attrib_t& attrib, const tinyobj::index_t& index) {
	auto& verts = attrib.vertices;
	auto& colors = attrib.colors;
	auto& normals = attrib.normals;
	auto& texcoords = attrib.texcoords;

	Vertex vertex;

	auto v = (size_t)index.vertex_index * 3;
	auto n = (size_t)index.normal_index * 3;
	auto t = (size_t)index.texcoord_index * 2;

	if (index.vertex_index >= 0) {
		vertex.position = { verts[v], verts[v + 1], verts[v + 2], 0.0f };
		vertex.color = { colors[v], colors[v + 1], colors[v + 2], 1.0f };
	}
	if (index.normal_index >= 0)
		vertex.normal = { normals[n], normals[n + 1], normals[n + 2], 1.0f };

	if (index.texcoord_index >= 0)
		vertex.uv = { texcoords[t], texcoords[t + 1] };

	return vertex;
}