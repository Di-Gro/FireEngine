#include "Mesh.h"

#include "Game.h"
#include "Assets.h"
#include "MeshAsset.h"
#include "Render.h"
#include "RenderPass.h"

#include "DirectionLight.h"
#include "CameraComponent.h"

/// ¬ HLSL пол€ структур выравниваютс€ по 16 байт 
/// и упаковываютс€ в прошлые 16 байт, если вмещаютс€.


Mesh4::Mesh4(const Mesh4& other) {
	this->operator=(other);
}

Mesh4& Mesh4::operator=(const Mesh4& other) {
	if (&other == nullptr || &other == this)
		return *this;

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

Mesh4::~Mesh4() {
	Release();
}

void Mesh4::Release() {
	for (auto& shape : m_shapes) {
		if (shape.verteces != nullptr)
			delete[] shape.verteces;

		if (shape.indeces != nullptr)
			delete[] shape.indeces;
	}
	m_shapes.clear();
}

void Mesh4::AddShape(
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


void Mesh4::AddShape(
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

void Mesh4::m_InitShape(Mesh4::Shape& shape) {
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


	auto vert_float3 = (DirectX::XMFLOAT3*)&shape.verteces->position;
	DirectX::BoundingBox::CreateFromPoints(shape.b_box, shape.vertecesSize, vert_float3, sizeof(Mesh4::Vertex));

	m_render->device()->CreateBuffer(&vertexBufferDesc, &vertexData, shape.vertexBuffer.GetAddressOf());
	m_render->device()->CreateBuffer(&indexBufferDesc, &indexData, shape.indexBuffer.GetAddressOf());
	m_render->device()->CreateBuffer(&constBufferDesc, nullptr, shape.meshCBuffer.GetAddressOf());
}

void Mesh4::Draw(const DynamicShapeData& data) const {
	for (int index = 0; index < m_shapes.size(); ++index)
		DrawShape(data, index);
}

void Mesh4::DrawShape(const DynamicShapeData& data, int index) const {
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

void ScreenQuad::Init(Render* render, const Shader* shader) {
	m_render = render;
	this->shader = shader;

	D3D11_SAMPLER_DESC sampleDesc = {};
	sampleDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampleDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampleDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampleDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampleDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sampleDesc.BorderColor[0] = 1.0f;
	sampleDesc.BorderColor[1] = 0.0f;
	sampleDesc.BorderColor[2] = 0.0f;
	sampleDesc.BorderColor[3] = 1.0f;
	sampleDesc.MaxLOD = INT_MAX;

	m_render->device()->CreateSamplerState(&sampleDesc, m_sampler.GetAddressOf());

	CD3D11_RASTERIZER_DESC rastDesc = {};
	rastDesc.CullMode = D3D11_CULL_FRONT;
	rastDesc.FillMode = D3D11_FILL_SOLID;

	m_render->device()->CreateRasterizerState(&rastDesc, rastState.GetAddressOf());
}

void ScreenQuad::Release() {
	if (m_sampler.Get() != nullptr)
		m_sampler.ReleaseAndGetAddressOf();

	if (rastState.Get() != nullptr)
		m_sampler.ReleaseAndGetAddressOf();
}

void ScreenQuad::Draw() const {
	auto* context = m_render->context();
	auto* camera = m_render->renderer()->camera();

	/// Material: SetResources
	ID3D11ShaderResourceView* resources[] = { deffuse != nullptr ? deffuse->get() : nullptr };
	context->PSSetShaderResources(PASS_R_MATERIAL_PS, 1, resources);
	context->PSSetSamplers(PASS_R_MATERIAL_PS, 1, m_sampler.GetAddressOf());

	/// Material: SetShader
	context->IASetInputLayout(nullptr);
	context->VSSetShader(shader->vertex.Get(), nullptr, 0);
	context->PSSetShader(shader->pixel.Get(), nullptr, 0);

	/// Material: SetMaterialConstBuffer 
	/// Material: SetRasterizerState
	/// Mesh: SetTopology
	context->IASetPrimitiveTopology(topology);

	/// Mesh: SetInput
	context->IASetIndexBuffer(nullptr, DXGI_FORMAT_R32_UINT, 0);

	/// Mesh: SetConstBuffers
	/// Mesh: Draw
	context->Draw(4, 0);
}

void ScreenQuad::Draw2() const {
	auto* context = m_render->context();
	auto* camera = m_render->renderer()->camera();

	/// Mesh: SetTopology
	context->IASetPrimitiveTopology(topology);

	/// Mesh: SetInput
	context->IASetIndexBuffer(nullptr, DXGI_FORMAT_R32_UINT, 0);

	/// Mesh: SetConstBuffers
	/// Mesh: Draw
	context->Draw(4, 0);
}

Mesh4::Shape* Mesh4::GetShape(int index) {
	return &m_shapes[index];
}

const Mesh4::Shape* Mesh4::GetConstShape(int index) const
{
	return &m_shapes[index];
}


int Mesh4::maxMaterialIndex() const {
	int maxIndex = 0;
	for (int i = 0; i < m_shapes.size(); i++) {
		if (m_shapes[i].materialIndex > maxIndex)
			maxIndex = m_shapes[i].materialIndex;
	}
	return maxIndex;
}


DEF_FUNC(Mesh4, ShapeCount, int)(CppRef mesh4Ref) {
	if (mesh4Ref == 0)
		return Mesh4().shapeCount();

	return CppRefs::ThrowPointer<Mesh4>(mesh4Ref)->shapeCount();
}

DEF_FUNC(Mesh4, MaxMaterialIndex, int)(CppRef mesh4Ref) {
	if (mesh4Ref == 0)
		return Mesh4().maxMaterialIndex();

	return CppRefs::ThrowPointer<Mesh4>(mesh4Ref)->maxMaterialIndex();
}

DEF_PUSH_ASSET(Mesh4);

DEF_FUNC(Mesh4, Init, void)(CppRef gameRef, CppRef meshRef, const char* path) {
	auto* game = CppRefs::ThrowPointer<Game>(gameRef);
	auto* mesh = CppRefs::ThrowPointer<Mesh4>(meshRef);

	game->meshAsset()->InitMesh(mesh, path);
	mesh->version++;
}

DEF_FUNC(Mesh4, materials_set, void)(CppRef meshRef, size_t* cppRefs, int count) {
	auto* mesh = CppRefs::ThrowPointer<Mesh4>(meshRef);

	mesh->f_staticMaterials.clear();

	auto ptr = cppRefs;
	for (int i = 0; i < count; i++, ptr++) {
		auto cppRef = RefCpp(*ptr);
		auto* material = CppRefs::ThrowPointer<Material>(cppRef);

		mesh->f_staticMaterials.push_back(material);
	}
}

DEF_FUNC(Mesh4, NextVersion, void)(CppRef meshRef) {
	auto* mesh = CppRefs::ThrowPointer<Mesh4>(meshRef);
	mesh->version++;
}