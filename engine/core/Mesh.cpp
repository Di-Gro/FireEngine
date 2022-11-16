#include "Mesh.h"
#include "Render.h"
#include "DirectionLight.h"
#include "CameraComponent.h"

/// ¬ HLSL пол€ структур выравниваютс€ по 16 байт 
/// и упаковываютс€ в прошлые 16 байт, если вмещаютс€.

#pragma pack(push, 4)
	static struct MeshCBuffer {
		Matrix wvpMatrix;
		Matrix worldMatrix;
		Vector3 cameraPosition;
		float _1[1];
	};
#pragma pack(pop)

#pragma pack(push, 4)
	static struct DirectionLightCBuffer {
		Matrix uvMatrix;
		Vector3 direction; 
		float intensity;
		Vector3 color;
		float _1[1];
	};
#pragma pack(pop)


Mesh4::~Mesh4() {
	for (auto& shape : m_shapes) {
		if (shape.verteces != nullptr)
			delete[] shape.verteces;

		if (shape.indeces != nullptr)
			delete[] shape.indeces;
	}
}

void Mesh4::AddShape(
	std::vector<Vertex>* verteces,
	std::vector<int>* indeces,
	Render* render,
	int materialIndex)
{
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
	shape.constBuffer = nullptr;
	shape.materialConstBuffer = nullptr;
	shape.sampler = nullptr;
	shape.materialIndex = materialIndex;

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

	D3D11_BUFFER_DESC materialCBufferDesc = {};
	materialCBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	materialCBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	materialCBufferDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
	materialCBufferDesc.MiscFlags = 0;
	materialCBufferDesc.StructureByteStride = 0;
	materialCBufferDesc.ByteWidth = sizeof(Material::Data);

	D3D11_BUFFER_DESC dirLightCBufferDesc = {};
	dirLightCBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	dirLightCBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	dirLightCBufferDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
	dirLightCBufferDesc.MiscFlags = 0;
	dirLightCBufferDesc.StructureByteStride = 0;
	dirLightCBufferDesc.ByteWidth = sizeof(DirectionLightCBuffer);

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

	D3D11_SAMPLER_DESC compSampleDesc = {};
	compSampleDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	compSampleDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	compSampleDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	compSampleDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR; // D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	compSampleDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL; // D3D11_COMPARISON_ALWAYS;
	compSampleDesc.BorderColor[0] = 1.0f;
	compSampleDesc.BorderColor[1] = 0.0f;
	compSampleDesc.BorderColor[2] = 0.0f;
	compSampleDesc.BorderColor[3] = 1.0f;
	compSampleDesc.MaxLOD = INT_MAX;

	render->device()->CreateBuffer(&vertexBufferDesc, &vertexData, shape.vertexBuffer.GetAddressOf());
	render->device()->CreateBuffer(&indexBufferDesc, &indexData, shape.indexBuffer.GetAddressOf());
	render->device()->CreateBuffer(&constBufferDesc, nullptr, shape.constBuffer.GetAddressOf());
	render->device()->CreateBuffer(&materialCBufferDesc, nullptr, shape.materialConstBuffer.GetAddressOf());
	render->device()->CreateBuffer(&dirLightCBufferDesc, nullptr, shape.directionLightCBuffer.GetAddressOf());
	
	render->device()->CreateSamplerState(&sampleDesc, shape.sampler.GetAddressOf());
	render->device()->CreateSamplerState(&compSampleDesc, shape.compSampler.GetAddressOf());
	
}

void Mesh4::Draw(const DynamicData& data) const {
	auto* context = data.render->context();
	auto* camera = data.render->camera();
	bool callPixelShader = camera->callPixelShader;

	for (auto& shape : m_shapes) {
		assert(shape.materialIndex >= 0 && shape.materialIndex < data.materials->size());

		auto* mat = data.materials->at(shape.materialIndex);
		auto* shader = /*camera->shader != nullptr ? camera->shader :*/ mat->shader;

		context->IASetInputLayout(shader->layout.Get());
		context->IASetPrimitiveTopology(topology);
		context->IASetIndexBuffer(shape.indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		context->IASetVertexBuffers(0, 1, shape.vertexBuffer.GetAddressOf(), m_strides, m_offsets);

		context->VSSetShader(shader->vertex.Get(), nullptr, 0);

		if (callPixelShader)
			context->PSSetShader(shader->pixel.Get(), nullptr, 0);

		context->VSSetConstantBuffers(0, 1, shape.constBuffer.GetAddressOf());
		context->PSSetConstantBuffers(0, 1, shape.constBuffer.GetAddressOf());
		context->PSSetConstantBuffers(1, 1, shape.materialConstBuffer.GetAddressOf());
		context->PSSetConstantBuffers(2, 1, shape.directionLightCBuffer.GetAddressOf());

		/// Mesh ->
		D3D11_MAPPED_SUBRESOURCE res = {};
		context->Map(shape.constBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
		
		auto* cbuf = (MeshCBuffer*)res.pData;
		cbuf->wvpMatrix = *data.transfMatrix;
		cbuf->worldMatrix = *data.worldMatrix;
		cbuf->cameraPosition = *data.cameraPosition;

		context->Unmap(shape.constBuffer.Get(), 0);
		/// <-

		/// Material ->
		D3D11_MAPPED_SUBRESOURCE res2 = {};
		context->Map(shape.materialConstBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &res2);
		memcpy(res2.pData, &mat->data, sizeof(Material::Data));
		context->Unmap(shape.materialConstBuffer.Get(), 0);
		/// <-

		/// Direction Light ->
		D3D11_MAPPED_SUBRESOURCE res3 = {};
		context->Map(shape.directionLightCBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &res3);

		auto* cbuf2 = (DirectionLightCBuffer*)res3.pData;
		cbuf2->uvMatrix = data.directionLight->uvMatrix();
		cbuf2->direction = data.directionLight->transform.forward();
		cbuf2->color = data.directionLight->color;
		cbuf2->intensity = data.directionLight->intensity;

		context->Unmap(shape.directionLightCBuffer.Get(), 0);
		/// <-
		/// Resources ->
		auto shadowMapSrv = data.directionLight->shadowRT.depthSRV();

		context->PSSetSamplers(0, 1, shape.sampler.GetAddressOf());
		context->PSSetSamplers(1, 1, shape.compSampler.GetAddressOf());

		ID3D11ShaderResourceView* shaderResources[] = {
			mat->diffuse.srv.Get(),
			camera->setShaderMap ? shadowMapSrv.Get() : nullptr,
		};

		//context->PSSetShaderResources(0, 1, mat->diffuse.srv.GetAddressOf());
		
		context->PSSetShaderResources(0, 2, shaderResources);

		/// <-
		context->DrawIndexed(shape.indecesSize, 0, 0);
	}
}

Mesh4::Shape* Mesh4::GetShape(int index) {
	return &m_shapes[index];
}