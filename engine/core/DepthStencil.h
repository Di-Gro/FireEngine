#pragma once

#include "wrl.h_d3d11_alias.h"

#include "RenderDevice.h"

class Game;


class DepthStencil {
public:
	float clearColor[4] = { 48 / 255.0f, 37 / 255.0f, 60 / 255.0f, 1.0f };

private:
	Game* m_game;

	comptr<ID3D11Texture2D> m_depthTex;
	comptr<ID3D11DepthStencilView> m_dsv;
	comptr<ID3D11ShaderResourceView> m_depthSRV;


public:

	ID3D11Texture2D* texture() { return m_depthTex.Get(); }
	ID3D11ShaderResourceView* shaderResource() { return m_depthSRV.Get(); }
	ID3D11DepthStencilView* depthStencil() { return m_dsv.Get(); }

	void Init(Game* game, int width, int height, bool createShaderResource);
	void InitResource();

	void Clear();

};
