#include "ShadowPass.h"
#include "Render.h"
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

void ShadowPass::Draw() {
	auto* prevCamera = m_render->m_camera;
	ID3D11RasterizerState* prevRastState[] = { nullptr };
	m_render->context()->RSGetState(prevRastState);

	// Directional light
	auto* light = m_render->m_game->lighting()->directionLight();

	// Begin Draw
	auto rt = light->RT();
	rt->Clear();
	light->DS()->Clear();

	ID3D11RenderTargetView* targets[] = { rt->get() };
	m_render->context()->OMSetRenderTargets(1, targets, light->depthStencil());
	m_render->context()->RSSetState(m_rastState.Get());

	// Draw
	m_render->m_camera = light->camera();

	for (auto* shadowCaster : m_render->m_shadowCasters) {
		if (!shadowCaster->GetComponent()->IsDestroyed()) {
			shadowCaster->OnDrawShadow(this);
		}
	}

	// End Draw
	m_render->m_camera = prevCamera;
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