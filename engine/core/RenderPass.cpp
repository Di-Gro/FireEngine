#include "RenderPass.h"

#include "Game.h"
#include "Render.h"
#include "RenderTarget.h"
#include "UI/UserInterface.h"
#include "Actor.h"
#include "ShaderResource.h"
#include "DepthStencil.h"
#include "CameraComponent.h"
#include "ActorCBuffer.h"
#include "EditorCBuffer.h"
//#include "UI\UserInterface.h"


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

	D3D11_BUFFER_DESC anyBufferDesc = {};
	anyBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	anyBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	anyBufferDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
	anyBufferDesc.MiscFlags = 0;
	anyBufferDesc.StructureByteStride = 0;

	anyBufferDesc.ByteWidth = sizeof(CameraCBuffer);
	m_render->device()->CreateBuffer(&anyBufferDesc, nullptr, m_cameraBuffer.GetAddressOf());

	anyBufferDesc.ByteWidth = sizeof(ActorCBuffer);
	m_render->device()->CreateBuffer(&anyBufferDesc, nullptr, m_actorBuffer.GetAddressOf());

	anyBufferDesc.ByteWidth = sizeof(EditorCBuffer);
	m_render->device()->CreateBuffer(&anyBufferDesc, nullptr, m_editorBuffer.GetAddressOf());
	
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

	/// RenderPass: PrepareTargets
	m_PrepareTargets();

	/// RenderPass: PrepareResources
	m_PrepareResources();

	/// RenderPass: ClearTargets
	m_ClearTargets();

	/// RenderPass: SetCameraConstBuffer
	m_SetCameraConstBuffer();

	/// RenderPass: SetTargets
	auto depthStencil = m_depthStencil != nullptr ? m_depthStencil->get() : nullptr;
	context->OMSetRenderTargets(8, m_dxTargets, depthStencil);

	/// RenderPass: SetResources
	context->VSSetShaderResources(0, SRCount, m_dxVSResources);
	context->PSSetShaderResources(0, SRCount, m_dxPSResources);

	context->VSSetSamplers(0, SRCount, m_dxVSSamplers);
	context->PSSetSamplers(0, SRCount, m_dxPSSamplers);

	/// RenderPass: SetBlendState
	context->OMSetBlendState(m_blendState.Get(), nullptr, 0xFFFFFFFF);
}

inline void RenderPass::EndDraw() {
	auto* context = m_render->context();

	ID3D11RenderTargetView* targets[RTCount] = { nullptr };
	context->OMSetRenderTargets(8, targets, nullptr);

	ID3D11ShaderResourceView* resources[SRCount] = { nullptr };
	context->VSSetShaderResources(0, SRCount, resources);
	context->PSSetShaderResources(0, SRCount, resources);

	ID3D11SamplerState* samplers[SRCount] = { nullptr };
	context->VSSetSamplers(0, SRCount, samplers);
	context->PSSetSamplers(0, SRCount, samplers);
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

void RenderPass::SetActorConstBuffer(Actor* actor) {
	auto* context = m_render->context();

	context->PSSetConstantBuffers(PASS_CB_ACTOR_PS, 1, m_actorBuffer.GetAddressOf());

	D3D11_MAPPED_SUBRESOURCE res = {};
	context->Map(m_actorBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &res);

	auto* cbuf = (ActorCBuffer*)res.pData;
	cbuf->actorId = (UINT)actor->cppRef().value;

	context->Unmap(m_actorBuffer.Get(), 0);
}

void RenderPass::SetEditorConstBuffer() {
	auto* context = m_render->context();

	context->PSSetConstantBuffers(PASS_CB_EDITOR_PS, 1, m_editorBuffer.GetAddressOf());

	D3D11_MAPPED_SUBRESOURCE res = {};
	context->Map(m_editorBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &res);

	auto* cbuf = (EditorCBuffer*)res.pData;

	if (m_game->ui()->HasActor())
		cbuf->selectedActorId = (UINT)m_game->ui()->GetActor()->cppRef().value;
	else
		cbuf->selectedActorId = 0;

	context->Unmap(m_editorBuffer.Get(), 0);
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
	if (!clearTargets)
		return;

	for (int i = 0; i < 8; i++) {
		if (m_targets[i] != nullptr)
			m_targets[i]->Clear();
	}
}

