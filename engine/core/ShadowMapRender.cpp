//#include "ShadowMapRender.h"
//
//#include <iostream>
//
//#include "RenderTarget.h"
//#include "Render.h"
//
//void ShadowMapRender::Init(Render* render, int width, int height) {
//	m_render = render;
//
//	auto device = m_render->device();
//
//	D3D11_TEXTURE2D_DESC renderTexDesc = {};
//	renderTexDesc.Width = width;
//	renderTexDesc.Height = height;
//	renderTexDesc.ArraySize = 1;
//	renderTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
//	renderTexDesc.Usage = D3D11_USAGE_DEFAULT;
//	renderTexDesc.CPUAccessFlags = 0;
//	renderTexDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
//	renderTexDesc.MipLevels = 1;
//	renderTexDesc.MiscFlags = 0;
//	renderTexDesc.SampleDesc.Count = 1;
//	renderTexDesc.SampleDesc.Quality = 0;
//
//	D3D11_TEXTURE2D_DESC depthTexDesc = {};
//	depthTexDesc.Width = width;
//	depthTexDesc.Height = height;
//	depthTexDesc.ArraySize = 1;
//	depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
//	depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
//	depthTexDesc.CPUAccessFlags = 0;
//	depthTexDesc.Format = DXGI_FORMAT_R32_TYPELESS;
//	depthTexDesc.MipLevels = 1;
//	depthTexDesc.MiscFlags = 0;
//	depthTexDesc.SampleDesc.Count = 1;
//	depthTexDesc.SampleDesc.Quality = 0;
//	
//	D3D11_DEPTH_STENCIL_VIEW_DESC depthStenViewDesc = {};
//	depthStenViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
//	depthStenViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
//	depthStenViewDesc.Flags = 0;
//
//	D3D11_SHADER_RESOURCE_VIEW_DESC shadowMapSrvDesc = {};
//	shadowMapSrvDesc.Format = DXGI_FORMAT_R32_FLOAT;
//	shadowMapSrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
//	shadowMapSrvDesc.Texture2D.MipLevels = 1;
//	shadowMapSrvDesc.Texture2D.MostDetailedMip = 0;
//
//	CD3D11_RASTERIZER_DESC rastDesc = {};
//	rastDesc.CullMode = D3D11_CULL_FRONT;
//	rastDesc.FillMode = D3D11_FILL_SOLID;
//
//	rastDesc.DepthBias = 0;
//	rastDesc.DepthBiasClamp = 0;
//	rastDesc.SlopeScaledDepthBias = 0;
//
//	auto hres = device->CreateTexture2D(&renderTexDesc, nullptr, m_renderTex.GetAddressOf());
//	assert(SUCCEEDED(hres));
//
//	hres = device->CreateTexture2D(&depthTexDesc, nullptr, m_depthTex.GetAddressOf());
//	assert(SUCCEEDED(hres));
//
//	hres = device->CreateRenderTargetView(m_renderTex.Get(), nullptr, m_rtv.GetAddressOf());
//	assert(SUCCEEDED(hres));
//
//	hres = device->CreateDepthStencilView(m_depthTex.Get(), &depthStenViewDesc, m_dsv.GetAddressOf());
//	assert(SUCCEEDED(hres));
//	
//	hres = device->CreateShaderResourceView(m_depthTex.Get(), &shadowMapSrvDesc, m_srv.GetAddressOf());
//	assert(SUCCEEDED(hres));
//
//	hres = device->CreateRasterizerState(&rastDesc, m_rastState.GetAddressOf());
//	assert(SUCCEEDED(hres));
//
//}
//
//void ShadowMapRender::CreateRastState(int v1, float v2, float v3, D3D11_CULL_MODE CullMode) {
//
//	m_rastState.ReleaseAndGetAddressOf();
//
//	CD3D11_RASTERIZER_DESC rastDesc = {};
//	rastDesc.CullMode = CullMode; // D3D11_CULL_FRONT;//D3D11_CULL_BACK;// ;
//	rastDesc.FillMode = D3D11_FILL_SOLID;
//
//	rastDesc.DepthBias = v1;
//	rastDesc.DepthBiasClamp = v2;
//	rastDesc.SlopeScaledDepthBias = v3;
//
//	auto device = m_render->device();
//
//	auto hres = device->CreateRasterizerState(&rastDesc, m_rastState.GetAddressOf());
//	assert(SUCCEEDED(hres));
//}
//
//void ShadowMapRender::Begin() {
//
//	m_render->Clear();
//
//	auto context = m_render->context();
//
//	context->OMSetRenderTargets(1, m_rtv.GetAddressOf(), m_dsv.Get()); // Вызываем каждый кадо, если SwapEffect = FLIP
//	context->RSSetState(m_rastState.Get());
//
//	context->ClearRenderTargetView(m_rtv.Get(), clearColor);
//	context->ClearDepthStencilView(m_dsv.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
//}
//
//void ShadowMapRender::End() {
//	auto context = m_render->context();
//
//	ID3D11RenderTargetView* nullRTV = nullptr;
//	context->OMSetRenderTargets(1, &nullRTV, nullptr);
//
//	//ID3D11ShaderResourceView* views[2] = {};
//	//context->PSSetSamplers(0, 1, &this->samplerState_.p);
//	//context->PSSetShaderResources(0, 2, views);
//}