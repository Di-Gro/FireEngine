#include "LightingPass.h"

#include "Render.h"
#include "CameraComponent.h"
#include "MaterialAlias.h"
#include "Mesh.h"
#include "MeshComponent.h"
#include "DirectionLight.h"

#include "RenderTarget.h"
#include "ShaderResource.h"
#include "DepthStencil.h"
#include "Material.h"

#include "ILightSource.h"


void LightingPass::Init(Game* game) {
	RenderPass::Init(game);

	blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;

	UpdateBlendState();

	D3D11_BUFFER_DESC cbufferDesc = {};
	cbufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbufferDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
	cbufferDesc.MiscFlags = 0;
	cbufferDesc.StructureByteStride = 0;

	cbufferDesc.ByteWidth = sizeof(LightCBuffer);
	m_render->device()->CreateBuffer(&cbufferDesc, nullptr, m_lightCBuffer.GetAddressOf());

	cbufferDesc.ByteWidth = sizeof(ShadowCBuffer);
	m_render->device()->CreateBuffer(&cbufferDesc, nullptr, m_shadowCBuffer.GetAddressOf());
}

void LightingPass::Draw() {
	BeginDraw();
	m_SetShadowCBuffer();
	
	for (auto* lightSource : m_render->m_lightSources) {
		m_SetLightCBuffer(lightSource);
		lightSource->OnDrawLight(this);
	}
	EndDraw();
}

void LightingPass::m_SetShadowCBuffer() {
	auto* context = m_render->context();
	auto* dirLight = m_game->lighting()->directionLight();

	context->PSSetConstantBuffers(PASS_CB_SHADOW_PS, 1, m_shadowCBuffer.GetAddressOf());

	D3D11_MAPPED_SUBRESOURCE res = {};
	context->Map(m_shadowCBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &res);

	auto* cbuf = (ShadowCBuffer*)res.pData;
	cbuf->uvMatrix = dirLight->uvMatrix();
	cbuf->mapScale = dirLight->mapScale();

	context->Unmap(m_shadowCBuffer.Get(), 0);
}

void LightingPass::m_SetLightCBuffer(ILightSource* lightSource) {
	auto* context = m_render->context();
	auto lightCBuffer = lightSource->GetCBuffer();

	context->PSSetConstantBuffers(PASS_CB_LIGHT_PS, 1, m_lightCBuffer.GetAddressOf());

	D3D11_MAPPED_SUBRESOURCE res3 = {};
	context->Map(m_lightCBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &res3);
	memcpy(res3.pData, &lightCBuffer, sizeof(LightCBuffer));
	context->Unmap(m_lightCBuffer.Get(), 0);
}