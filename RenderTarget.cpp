#include "RenderTarget.h"
#include "Game.h"
#include "Render.h"
#include "RenderDevice.h"


void RenderTarget::Init(Game* game, int width, int height) {
	m_game = game;

	D3D11_TEXTURE2D_DESC renderTexDesc = {};
	renderTexDesc.Width = width;
	renderTexDesc.Height = height;
	renderTexDesc.ArraySize = 1;
	renderTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	renderTexDesc.Usage = D3D11_USAGE_DEFAULT;
	renderTexDesc.CPUAccessFlags = 0;
	renderTexDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	renderTexDesc.MipLevels = 1;
	renderTexDesc.MiscFlags = 0;
	renderTexDesc.SampleDesc.Count = 1;
	renderTexDesc.SampleDesc.Quality = 0;

	D3D11_TEXTURE2D_DESC depthTexDesc = {};
	depthTexDesc.Width = width;
	depthTexDesc.Height = height;
	depthTexDesc.ArraySize = 1;
	depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
	depthTexDesc.CPUAccessFlags = 0;
	depthTexDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	depthTexDesc.MipLevels = 1;
	depthTexDesc.MiscFlags = 0;
	depthTexDesc.SampleDesc.Count = 1;
	depthTexDesc.SampleDesc.Quality = 0;

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStenViewDesc = {};
	depthStenViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStenViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStenViewDesc.Flags = 0;

	D3D11_SHADER_RESOURCE_VIEW_DESC renderSRVDesc = {};
	renderSRVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	renderSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	renderSRVDesc.Texture2D.MipLevels = 1;
	renderSRVDesc.Texture2D.MostDetailedMip = 0;

	D3D11_SHADER_RESOURCE_VIEW_DESC depthSRVDesc = {};
	depthSRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
	depthSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	depthSRVDesc.Texture2D.MipLevels = 1;
	depthSRVDesc.Texture2D.MostDetailedMip = 0;

	CD3D11_RASTERIZER_DESC rastDesc = {};
	rastDesc.CullMode = D3D11_CULL_FRONT;
	rastDesc.FillMode = D3D11_FILL_SOLID;

	rastDesc.DepthBias = 0;
	rastDesc.DepthBiasClamp = 0;
	rastDesc.SlopeScaledDepthBias = 0;

	auto device = m_game->render()->device();

	auto hres = device->CreateTexture2D(&renderTexDesc, nullptr, m_renderTex.GetAddressOf());
	assert(SUCCEEDED(hres));

	hres = device->CreateTexture2D(&depthTexDesc, nullptr, m_depthTex.GetAddressOf());
	assert(SUCCEEDED(hres));

	hres = device->CreateRenderTargetView(m_renderTex.Get(), nullptr, m_rtv.GetAddressOf());
	assert(SUCCEEDED(hres));

	hres = device->CreateDepthStencilView(m_depthTex.Get(), &depthStenViewDesc, m_dsv.GetAddressOf());
	assert(SUCCEEDED(hres));

	hres = device->CreateShaderResourceView(m_renderTex.Get(), &renderSRVDesc, m_renderSRV.GetAddressOf());
	assert(SUCCEEDED(hres));

	hres = device->CreateShaderResourceView(m_depthTex.Get(), &depthSRVDesc, m_depthSRV.GetAddressOf());
	assert(SUCCEEDED(hres));

	hres = device->CreateRasterizerState(&rastDesc, m_rastState.GetAddressOf());
	assert(SUCCEEDED(hres));
}

void RenderTarget::ResetRastState(
	int depthBias,
	float depthBiasClamp, 
	float slopeScaledDepthBias, 
	D3D11_CULL_MODE cullMode,
	D3D11_FILL_MODE fillMode)
{
	auto device = m_game->render()->device();

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

void RenderTarget::PrepareRender() {

	auto context = m_game->render()->context();

	context->OMSetRenderTargets(1, m_rtv.GetAddressOf(), m_dsv.Get()); // Вызываем каждый кадо, если SwapEffect = FLIP
	context->RSSetState(m_rastState.Get());

	context->ClearRenderTargetView(m_rtv.Get(), clearColor);
	context->ClearDepthStencilView(m_dsv.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void RenderTarget::FinishRender() {

	auto context = m_game->render()->context();

	ID3D11RenderTargetView* nullRTV = nullptr;
	context->OMSetRenderTargets(1, &nullRTV, nullptr);

	//ID3D11ShaderResourceView* views[2] = {};
	//context->PSSetSamplers(0, 1, &this->samplerState_.p);
	//context->PSSetShaderResources(0, 2, views);
}