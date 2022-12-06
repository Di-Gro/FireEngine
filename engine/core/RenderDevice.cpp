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

//#define CHECK(hres) { \
//	if (FAILED(hres))\
//	return hres; \
//}\

void RenderDevice::Create(HWND hwnd, int width, int height) {

	DXGI_SWAP_CHAIN_DESC swapDesc = {};
	swapDesc.BufferCount = 2; // количество back буферов
	swapDesc.BufferDesc.Width = width; // ширина и высота буфера
	swapDesc.BufferDesc.Height = height;
	swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // формат цвета
	swapDesc.BufferDesc.RefreshRate.Numerator = 240; // частота обновления
	swapDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // используем как буффер для рендера и вывода
	swapDesc.OutputWindow = hwnd; // связываем с окном, выводит в него инфу из back буфера
	swapDesc.Windowed = true; // оконное приложение
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // m_rtvпосле рендера: копируем во front буфер, остается мусор
	swapDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; // ставим флаг для перехода между оконными и полноэкранным режимом
	swapDesc.SampleDesc.Count = 1; // мультисемплинг
	swapDesc.SampleDesc.Quality = 0;

	D3D_FEATURE_LEVEL featureLevel[] = { D3D_FEATURE_LEVEL_11_0 };

	HRESULT hres;

	hres = D3D11CreateDeviceAndSwapChain(
		nullptr, // видеокарта по-умолчанию
		D3D_DRIVER_TYPE_HARDWARE, // используем hardware драйвер
		nullptr, // software, используется для software драйвера 
		D3D11_CREATE_DEVICE_DEBUG, // флаг для дебага
		featureLevel, // поддрдиваемый набор функций
		1,
		D3D11_SDK_VERSION, // используем 11 версию sdk 
		&swapDesc, // дескриптор для SwapChain-а 
		m_swapChain.GetAddressOf(),
		m_device.GetAddressOf(),
		nullptr, // максимальный уровень набора функций
		m_context.GetAddressOf());

	if (FAILED(hres)) {
		std::cout << "D3D11CreateDeviceAndSwapChain return FAIL" << std::endl;
		std::exit(1);
		return;
	}
	
	hres = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)m_backTex.GetAddressOf());
	hres = m_device->CreateRenderTargetView(m_backTex.Get(), nullptr, m_rtv.GetAddressOf());

	CD3D11_RASTERIZER_DESC rastDesc = {};
	rastDesc.CullMode = D3D11_CULL_FRONT;// D3D11_CULL_NONE;
	rastDesc.FillMode = D3D11_FILL_SOLID; // D3D11_FILL_SOLID; // wireframe или solid

	//rastDesc.DepthBias = 0;
	//rastDesc.DepthBiasClamp = 15.0f;
	//rastDesc.SlopeScaledDepthBias = 3.9f;

	hres = m_device->CreateRasterizerState(&rastDesc, m_rastState.GetAddressOf());

	m_viewport.Width = width;
	m_viewport.Height = height;
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;
	m_viewport.MinDepth = 0;
	m_viewport.MaxDepth = 1.0f;

	D3D11_TEXTURE2D_DESC depthTexDesc = {};
	depthTexDesc.Width = width;
	depthTexDesc.Height = height;
	depthTexDesc.ArraySize = 1;
	depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
	depthTexDesc.CPUAccessFlags = 0;
	depthTexDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthTexDesc.MipLevels = 1;
	depthTexDesc.MiscFlags = 0;
	depthTexDesc.SampleDesc = { 1, 0 };

	D3D11_DEPTH_STENCIL_VIEW_DESC depthStenViewDesc = {};
	depthStenViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStenViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStenViewDesc.Flags = 0;

	hres = m_device->CreateTexture2D(&depthTexDesc, nullptr, m_depthTex.GetAddressOf());

	if (FAILED(hres)) {
		std::cout << "D3D11_TEXTURE2D return FAIL" << std::endl;
		std::exit(1);
		return;
	}

	hres = m_device->CreateDepthStencilView(m_depthTex.Get(), &depthStenViewDesc, m_dsv.GetAddressOf());

	if (FAILED(hres)) {
		std::cout << "D3D11_DEPTH_STENCIL_VIEW return FAIL" << std::endl;
		std::exit(1);
		return;
	}

}

//void RenderDevice::Clear() {
//	m_context->ClearState();
//	m_context->RSSetState(m_rastState.Get());
//	m_context->RSSetViewports(1, &m_viewport);
//}

void RenderDevice::BeginDraw() {

	ID3D11RenderTargetView* m_dxRenderTargets[8] = { m_rtv.Get(), nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
	m_context->OMSetRenderTargets(8, m_dxRenderTargets, m_dsv.Get()); // Вызываем каждый кадо, если SwapEffect = FLIP
	
	m_context->ClearRenderTargetView(m_rtv.Get(), clearColor);
	m_context->ClearDepthStencilView(m_dsv.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void RenderDevice::EndDraw() {
	m_swapChain->Present(0, 0); // делит RefreshRate на 1 -> 60 раз в сек.
}
