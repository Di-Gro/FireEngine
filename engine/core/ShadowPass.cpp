#include "ShadowPass.h"

#include "Game.h"
#include "Scene.h"
#include "Render.h"
#include "Lighting.h"

#include "DirectionLight.h"
#include "IShadowCaster.h"


void ShadowPass::Init(Game* game) {
	RenderPass::Init(game);

	callPixelShader = false;

	CD3D11_RASTERIZER_DESC rastDesc = {};
	rastDesc.CullMode = D3D11_CULL_BACK;
	rastDesc.FillMode = D3D11_FILL_SOLID;

	rastDesc.DepthBias = 0;
	rastDesc.DepthBiasClamp = 0;
	rastDesc.SlopeScaledDepthBias = 0;

	auto hres = m_render->device()->CreateRasterizerState(&rastDesc, m_rastState.GetAddressOf());
	assert(SUCCEEDED(hres));
}

void ShadowPass::Resize(float width, float height) {
	auto *light = m_game->currentScene()->directionLight;
	if (light == nullptr)
		return;

	light->Resize(width, height);
}

void ShadowPass::Draw() {
	auto* light = m_game->currentScene()->directionLight;
	if (light == nullptr)
		return;

	auto* prevCamera = m_game->currentScene()->renderer.m_camera;
	ID3D11RasterizerState* prevRastState[] = { nullptr };
	m_render->context()->RSGetState(prevRastState);

	// Begin Draw
	auto rt = light->RT();
	rt->Clear();
	light->DS()->Clear();

	Vector3 shadowMapScale = { light->mapScale(), light->mapScale(), light->mapScale() };

	ID3D11RenderTargetView* targets[] = { rt->get() };
	m_render->context()->OMSetRenderTargets(1, targets, light->depthStencil());
	m_render->context()->RSSetState(m_rastState.Get());

	// Draw
	m_game->currentScene()->renderer.m_camera = light->camera();

	for (auto* shadowCaster : m_game->currentScene()->renderer.m_shadowCasters) {
		if (!shadowCaster->GetComponent()->IsDestroyed()) {
			shadowCaster->OnDrawShadow(this, shadowMapScale);
		}
	}

	// End Draw
	m_game->currentScene()->renderer.m_camera = prevCamera;
	m_render->context()->RSSetState(prevRastState[0]);
}

void ShadowPass::ResetRastState(
	int depthBias,
	float depthBiasClamp, 
	float slopeScaledDepthBias, 
	D3D11_CULL_MODE cullMode,
	D3D11_FILL_MODE fillMode)
{
	auto device = m_render->device();

	m_rastState.ReleaseAndGetAddressOf();

	CD3D11_RASTERIZER_DESC rastDesc = {};
	rastDesc.CullMode = cullMode; // D3D11_CULL_FRONT // D3D11_CULL_BACK
	rastDesc.FillMode = fillMode; // D3D11_FILL_SOLID // D3D11_FILL_WIREFRAME

	rastDesc.DepthBias = depthBias;
	rastDesc.DepthBiasClamp = depthBiasClamp;
	rastDesc.SlopeScaledDepthBias = slopeScaledDepthBias;

	auto hres = device->CreateRasterizerState(&rastDesc, m_rastState.GetAddressOf());
	assert(SUCCEEDED(hres));
}