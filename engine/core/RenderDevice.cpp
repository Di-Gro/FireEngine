#include "RenderDevice.h"

#include <iostream>

#include <windows.h>
#include <WinUser.h>
#include <wrl.h>
#include <d3d.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>

#include "IWindow.h"
#include "Mesh.h"
#include "Material.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")


void RenderDevice::Create(HWND hwnd, int width, int height) {
	m_CreateDevice(hwnd, width, height);
	m_CreateRenderTarget();
	m_CreateDepthStencil(width, height);
	ResizeViewport(width, height);
	m_CreateRastState();
}

void RenderDevice::m_CreateDevice(HWND hwnd, int width, int height) {
	m_swapDesc.BufferCount = 2; // ���������� back �������
	m_swapDesc.BufferDesc.Width = width; // ������ � ������ ������
	m_swapDesc.BufferDesc.Height = height;
	m_swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // ������ �����
	m_swapDesc.BufferDesc.RefreshRate.Numerator = 240; // ������� ����������
	m_swapDesc.BufferDesc.RefreshRate.Denominator = 1;
	m_swapDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	m_swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	m_swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // ���������� ��� ������ ��� ������� � ������
	m_swapDesc.OutputWindow = hwnd; // ��������� � �����, ������� � ���� ���� �� back ������
	m_swapDesc.Windowed = true; // ������� ����������
	m_swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // m_rtv����� �������: �������� �� front �����, �������� �����
	m_swapDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // ������ ���� ��� �������� ����� �������� � ������������� �������
	m_swapDesc.SampleDesc.Count = 1; // ��������������
	m_swapDesc.SampleDesc.Quality = 0;

	D3D_FEATURE_LEVEL featureLevel[] = { D3D_FEATURE_LEVEL_11_0 };

	auto hres = D3D11CreateDeviceAndSwapChain(
		nullptr, // ���������� ��-���������
		D3D_DRIVER_TYPE_HARDWARE, // ���������� hardware �������
		nullptr, // software, ������������ ��� software �������� 
		D3D11_CREATE_DEVICE_DEBUG, // ���� ��� ������
		featureLevel, // ������������� ����� �������
		1,
		D3D11_SDK_VERSION, // ���������� 11 ������ sdk 
		&m_swapDesc, // ���������� ��� SwapChain-� 
		m_swapChain.GetAddressOf(),
		m_device.GetAddressOf(),
		nullptr, // ������������ ������� ������ �������
		m_context.GetAddressOf());
	assert(SUCCEEDED(hres));
}

void RenderDevice::m_CreateRastState() {
	CD3D11_RASTERIZER_DESC rastDesc = {};
	rastDesc.CullMode = D3D11_CULL_FRONT;
	rastDesc.FillMode = D3D11_FILL_SOLID;

	auto hres = m_device->CreateRasterizerState(&rastDesc, m_rastState.GetAddressOf());
	assert(SUCCEEDED(hres));
}

void RenderDevice::m_CreateRenderTarget() {
	auto hres = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)m_backTex.GetAddressOf());
	assert(SUCCEEDED(hres));

	hres = m_device->CreateRenderTargetView(m_backTex.Get(), nullptr, m_rtv.GetAddressOf());
	assert(SUCCEEDED(hres));
}

void RenderDevice::m_RemoveRenderTarget() {
	if (m_rtv.Get() != nullptr)
		m_rtv.ReleaseAndGetAddressOf();

	if (m_backTex.Get() != nullptr)
		m_backTex.ReleaseAndGetAddressOf();
}

void RenderDevice::ResizeViewport(float width, float height) {
	m_viewport.Width = width;
	m_viewport.Height = height;
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;
	m_viewport.MinDepth = 0;
	m_viewport.MaxDepth = 1.0f;
}

void RenderDevice::m_CreateDepthStencil(float width, float height) {
	m_depthTexDesc.Width = width;
	m_depthTexDesc.Height = height;
	m_depthTexDesc.ArraySize = 1;
	m_depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	m_depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
	m_depthTexDesc.CPUAccessFlags = 0;
	m_depthTexDesc.Format = DXGI_FORMAT_D32_FLOAT;
	m_depthTexDesc.MipLevels = 1;
	m_depthTexDesc.MiscFlags = 0;
	m_depthTexDesc.SampleDesc = { 1, 0 };

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStenViewDesc = {};
	depthStenViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStenViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStenViewDesc.Flags = 0;

	auto hres = m_device->CreateTexture2D(&m_depthTexDesc, nullptr, m_depthTex.GetAddressOf());
	assert(SUCCEEDED(hres));

	hres = m_device->CreateDepthStencilView(m_depthTex.Get(), &depthStenViewDesc, m_dsv.GetAddressOf());
	assert(SUCCEEDED(hres));
}

void RenderDevice::m_RemoveDepthStencil() {
	if (m_dsv.Get() != nullptr)
		m_dsv.ReleaseAndGetAddressOf();

	if (m_depthTex.Get() != nullptr)
		m_depthTex.ReleaseAndGetAddressOf();
}

void RenderDevice::Resize(float width, float height) {

	/// ������� ����� ������� 
	m_RemoveRenderTarget();
	m_RemoveDepthStencil();

	/// ������ resize SwapChain-� 
	auto format = m_swapDesc.BufferDesc.Format;
	auto flags = m_swapDesc.Flags;

	m_swapChain.Get()->ResizeBuffers(0, width, height, format, flags);

	m_swapDesc.BufferDesc.Width = width;
	m_swapDesc.BufferDesc.Height = height;

	/// ������� ����� ������� 
	m_CreateRenderTarget();
	m_CreateDepthStencil(width, height);
}

//void RenderDevice::Clear() {
//	m_context->ClearState();
//	m_context->RSSetState(m_rastState.Get());
//	m_context->RSSetViewports(1, &m_viewport);
//}

void RenderDevice::SetViewport() {
	m_context->RSSetViewports(1, &m_viewport);
}

void RenderDevice::BeginDraw() {

	ID3D11RenderTargetView* m_dxRenderTargets[8] = { m_rtv.Get(), nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
	m_context->OMSetRenderTargets(8, m_dxRenderTargets, m_dsv.Get()); // �������� ������ ����, ���� SwapEffect = FLIP
	
	m_context->ClearRenderTargetView(m_rtv.Get(), clearColor);
	m_context->ClearDepthStencilView(m_dsv.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void RenderDevice::EndDraw() {
	m_swapChain->Present(1, 0); // ����� RefreshRate �� 1 -> 60 ��� � ���.
}

