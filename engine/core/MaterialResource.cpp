#include "MaterialResource.h"

#include "Render.h"
#include "Game.h"
#include "Assets.h"
#include "ShaderAsset.h"


void MaterialResource::Release() {
	if (materialConstBuffer.Get() != nullptr)
		materialConstBuffer.ReleaseAndGetAddressOf();

	if (depthStencilState.Get() != nullptr)
		depthStencilState.ReleaseAndGetAddressOf();
}

void MaterialResource::Init(Render* render) {

	m_render = render;

	D3D11_BUFFER_DESC materialCBufferDesc = {};
	materialCBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	materialCBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	materialCBufferDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
	materialCBufferDesc.MiscFlags = 0;
	materialCBufferDesc.StructureByteStride = 0;
	materialCBufferDesc.ByteWidth = sizeof(MaterialResource::Data);

	depthStencilDesc = {};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.StencilEnable = false;
	depthStencilDesc.StencilReadMask = 0x00;
	depthStencilDesc.StencilWriteMask = 0x00;

	render->device()->CreateBuffer(&materialCBufferDesc, nullptr, materialConstBuffer.GetAddressOf());

	UpdateDepthStencilState();
}

void MaterialResource::UpdateDepthStencilState() {
	if (depthStencilState.Get() != nullptr)
		depthStencilState.ReleaseAndGetAddressOf();

	m_render->device()->CreateDepthStencilState(&depthStencilDesc, depthStencilState.GetAddressOf());
}
