#pragma once

#include <map>

#include "wrl.h_d3d11_alias.h"

#include "IWindow.h"
#include "Shader.h"
#include "Mesh.h"
#include "ImageAsset.h"

class CameraComponent;

class RenderDevice {
public:
	float clearColor[4] = { 48 / 255.0f, 37 / 255.0f, 60 / 255.0f, 1.0f }; // { 0.1f, 0.1f, 0.1f, 1.0f };

	CameraComponent* camera = nullptr;
	
private:
	comptr<ID3D11Device> m_device;
	comptr<ID3D11DeviceContext> m_context;
	comptr<IDXGISwapChain> m_swapChain;

	comptr<ID3D11Texture2D> m_backTex;
	comptr<ID3D11Texture2D> m_depthTex;

	comptr<ID3D11RenderTargetView> m_rtv;
	comptr<ID3D11DepthStencilView> m_dsv;


	comptr<ID3D11RasterizerState> m_rastState;

	D3D11_VIEWPORT m_viewport = {};

public:
	ID3D11Device* GetDevice() { return m_device.Get(); }
	ID3D11DeviceContext* GetContext() { return m_context.Get(); }
	IDXGISwapChain* GetSwapChain() { return m_swapChain.Get(); }
	comptr<ID3D11RenderTargetView> GetRTV() { return m_rtv; }
	D3D11_VIEWPORT* viewport() { return &m_viewport; }
	ID3D11RasterizerState* rastState() { return m_rastState.Get(); } 
	//ID3D11DepthStencilView* depthStencil() { return m_dsv.Get(); }

	void Create(HWND hwnd, int width, int height);

	//void Clear();
	void BeginDraw();
	void EndDraw();

};

