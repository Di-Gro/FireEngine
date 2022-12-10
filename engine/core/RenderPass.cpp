#include "RenderPass.h"

#include "Game.h"
#include "Render.h"
#include "RenderTarget.h"
#include "ShaderResource.h"
#include "DepthStencil.h"
#include "CameraComponent.h"


void RenderPass::Init(Game* game) {
	m_render = game->render();
	m_game = game;

	blendStateDesc = D3D11_BLEND_DESC{ false, false };
	blendStateDesc.RenderTarget[0].RenderTargetWriteMask = 0x0f;
	blendStateDesc.RenderTarget[0].BlendEnable = true;
	blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;

	UpdateBlendState();

	D3D11_BUFFER_DESC cameraBufferDesc = {};
	cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cameraBufferDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
	cameraBufferDesc.MiscFlags = 0;
	cameraBufferDesc.StructureByteStride = 0;
	cameraBufferDesc.ByteWidth = sizeof(CameraCBuffer);

	m_render->device()->CreateBuffer(&cameraBufferDesc, nullptr, m_cameraBuffer.GetAddressOf());
}

RenderPass::~RenderPass() { }

void RenderPass::UpdateBlendState() {
	if (m_blendState.Get() != nullptr)
		m_blendState.ReleaseAndGetAddressOf();

	m_render->device()->CreateBlendState(&blendStateDesc, m_blendState.GetAddressOf());
}

void RenderPass::Draw() {
	BeginDraw();
	EndDraw();
}

void RenderPass::SetRenderTargets(std::initializer_list<RenderTarget*> targets) {
	for (int i = 0; i < RTCount; i++)
		m_targets[i] = i < targets.size() ? *(targets.begin() + i) : nullptr;
}

void RenderPass::SetVSShaderResources(std::initializer_list<ShaderResource*> resources) {
	for (int i = 0; i < SRCount; i++)
		m_VSResources[i] = i < resources.size() ? *(resources.begin() + i) : nullptr;
}

void RenderPass::SetPSShaderResources(std::initializer_list<ShaderResource*> resources) {
	for (int i = 0; i < SRCount; i++)
		m_PSResources[i] = i < resources.size() ? *(resources.begin() + i) : nullptr;
}

inline void RenderPass::BeginDraw() {
	auto* context = m_render->context();

	m_PrepareResources();
	m_PrepareTargets();
	m_ClearTargets();
	m_SetCameraConstBuffer();

	auto depthStencil = m_depthStencil != nullptr ? m_depthStencil->get() : nullptr;
	context->OMSetRenderTargets(8, m_dxTargets, depthStencil);

	context->VSSetShaderResources(0, SRCount, m_dxVSResources);
	context->PSSetShaderResources(0, SRCount, m_dxPSResources);

	context->VSSetSamplers(0, SRCount, m_dxVSSamplers);
	context->PSSetSamplers(0, SRCount, m_dxPSSamplers);
}


inline void RenderPass::EndDraw() {
	auto* context = m_render->context();

	ID3D11RenderTargetView* targets[RTCount] = { nullptr };
	context->OMSetRenderTargets(8, targets, nullptr);

	ID3D11ShaderResourceView* resources[SRCount] = { nullptr };
	context->VSSetShaderResources(0, SRCount, resources);
	context->PSSetShaderResources(0, SRCount, resources);
}

void RenderPass::PrepareMaterial(const Material* material) {
	m_PrepareMaterialResources(material);
	m_SetShader(material);
	m_SetMaterialConstBuffer(material);

	m_render->context()->RSSetState(m_render->GetRastState(material));
}

void RenderPass::m_PrepareMaterialResources(const Material* material) {
	auto* context = m_render->context();

	for (int i = 0; i < material->resources.size(); ++i) {
		auto& resource = material->resources[i];

		context->PSSetShaderResources(SRCount + i, 1, resource.getRef());
		context->PSSetSamplers(SRCount + i, 1, resource.samplerRef());
	}
}

void RenderPass::m_SetShader(const Material* material) {
	auto* context = m_render->context();
	auto* shader = material->shader;
		
	context->IASetInputLayout(shader->layout.Get());

	context->VSSetShader(shader->vertex.Get(), nullptr, 0);

	if (callPixelShader)
		context->PSSetShader(shader->pixel.Get(), nullptr, 0);

}

void RenderPass::m_SetMaterialConstBuffer(const Material* material) {
	auto* context = m_render->context();

	context->PSSetConstantBuffers(PASS_CB_MATERIAL_PS, 1, material->materialConstBuffer.GetAddressOf());

	D3D11_MAPPED_SUBRESOURCE res = {};
	context->Map(material->materialConstBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
	memcpy(res.pData, &material->data, sizeof(Material::Data));
	context->Unmap(material->materialConstBuffer.Get(), 0);
}

void RenderPass::m_SetCameraConstBuffer() {
	auto* context = m_render->context();

	context->PSSetConstantBuffers(PASS_CB_CAMERA_PS, 1, m_cameraBuffer.GetAddressOf());

	D3D11_MAPPED_SUBRESOURCE res = {};
	context->Map(m_cameraBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &res);

	auto* cbuf = (CameraCBuffer*)res.pData;
	cbuf->position = m_render->camera()->worldPosition();

	context->Unmap(m_cameraBuffer.Get(), 0);
}

void RenderPass::m_PrepareResources() {
	for (int i = 0; i < SRCount; i++) {
		m_dxVSResources[i] = m_VSResources[i] != nullptr ? m_VSResources[i]->get() : nullptr;
		m_dxPSResources[i] = m_PSResources[i] != nullptr ? m_PSResources[i]->get() : nullptr;

		m_dxVSSamplers[i] = m_VSResources[i] != nullptr ? m_VSResources[i]->sampler() : nullptr;
		m_dxPSSamplers[i] = m_PSResources[i] != nullptr ? m_PSResources[i]->sampler() : nullptr;
	}
}

void RenderPass::m_PrepareTargets() {
	for (int i = 0; i < RTCount; i++) {
		m_dxTargets[i] = m_targets[i] != nullptr ? m_targets[i]->get() : nullptr;
	}
}

void RenderPass::m_ClearTargets() {
	for (int i = 0; i < 8; i++) {
		if (m_targets[i] != nullptr)
			m_targets[i]->Clear();
	}
}

