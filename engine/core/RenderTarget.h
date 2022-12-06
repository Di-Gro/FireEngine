#pragma once

#include "wrl.h_d3d11_alias.h"

class Render;

class RenderTarget {
public:
	float clearColor[4] = { 48 / 255.0f, 37 / 255.0f, 60 / 255.0f, 1.0f };

private:
	Game* m_game;

	//comptr<ID3D11Texture2D> m_renderTex;
	comptr<ID3D11RenderTargetView> m_rtv;
	
public:

	ID3D11Texture2D* texture() { return m_renderTex.Get(); }
	ID3D11RenderTargetView* renderTarget() { return m_rtv.Get(); }

	void Init(Render* render, int width, int height, bool createShaderResource);

	void Clear();

};
