#pragma once 

#include "wrl.h_d3d11_alias.h"

#include "RenderDevice.h"

class Render;


class ShadowMapRender { 
public:
	float clearColor[4] = { 48 / 255.0f, 37 / 255.0f, 60 / 255.0f, 1.0f };

private:
	Render* m_render;

	comptr<ID3D11Texture2D> m_renderTex;
	comptr<ID3D11Texture2D> m_depthTex;

	comptr<ID3D11RenderTargetView> m_rtv;
	comptr<ID3D11DepthStencilView> m_dsv;
	comptr<ID3D11ShaderResourceView> m_srv;

	comptr<ID3D11RasterizerState> m_rastState;

public:

	comptr<ID3D11Texture2D> renderTexture() { return m_renderTex; }
	comptr<ID3D11Texture2D> depthTexture() { return m_depthTex; }
	comptr<ID3D11ShaderResourceView> srv() { return m_srv; } 

	void Init(Render* render, int width, int height);

	void CreateRastState(int v1, float v2, float v3, D3D11_CULL_MODE CullMode);

	void Begin();
	void End();
};

