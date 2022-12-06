#include "DepthStencil.h"
#include "Game.h"
#include "Render.h"
#include "RenderDevice.h"


void DepthStencil::Init(Game* game, int width, int height, bool createShaderResource) {
	m_game = game;

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

	auto device = m_game->render()->device();

	auto hres = device->CreateTexture2D(&depthTexDesc, nullptr, m_depthTex.GetAddressOf());
	assert(SUCCEEDED(hres));

	hres = device->CreateDepthStencilView(m_depthTex.Get(), &depthStenViewDesc, m_dsv.GetAddressOf());
	assert(SUCCEEDED(hres));

	if (createShaderResource)
		InitResource();

	Clear();
}

void DepthStencil::InitResource() {
	if (m_depthSRV.Get() != nullptr)
		return;

	auto device = m_game->render()->device();

	D3D11_SHADER_RESOURCE_VIEW_DESC depthSRVDesc = {};
	depthSRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
	depthSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	depthSRVDesc.Texture2D.MipLevels = 1;
	depthSRVDesc.Texture2D.MostDetailedMip = 0;

	auto hres = device->CreateShaderResourceView(m_depthTex.Get(), &depthSRVDesc, m_depthSRV.GetAddressOf());
	assert(SUCCEEDED(hres));
}

void DepthStencil::Clear() {
	auto context = m_game->render()->context();

	context->ClearDepthStencilView(m_dsv.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}
