#pragma once

#include "wrl.h_d3d11_alias.h"

#include "RenderDevice.h"

class Game;


class RenderTarget {
public:
	float clearColor[4] = { 48 / 255.0f, 37 / 255.0f, 60 / 255.0f, 1.0f };

private:
	Game* m_game;

	comptr<ID3D11Texture2D> m_renderTex;
	comptr<ID3D11Texture2D> m_depthTex;

	comptr<ID3D11RenderTargetView> m_rtv;
	comptr<ID3D11DepthStencilView> m_dsv;
	comptr<ID3D11ShaderResourceView> m_renderSRV;
	comptr<ID3D11ShaderResourceView> m_depthSRV;

	comptr<ID3D11RasterizerState> m_rastState;

public:

	comptr<ID3D11Texture2D> renderTexture() { return m_renderTex; }
	comptr<ID3D11Texture2D> depthTexture() { return m_depthTex; }
	comptr<ID3D11ShaderResourceView> renderSRV() { return m_renderSRV; }
	comptr<ID3D11ShaderResourceView> depthSRV() { return m_depthSRV; }

	void Init(Game* game, int width, int height);

	void ResetRastState(
		int depthBias,
		float depthBiasClamp,
		float slopeScaledDepthBias,
		D3D11_CULL_MODE cullMode = D3D11_CULL_FRONT,
		D3D11_FILL_MODE fillMode = D3D11_FILL_SOLID);

	void PrepareRender();
	void FinishRender();

};
