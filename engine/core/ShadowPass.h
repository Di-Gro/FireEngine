#pragma once
#include "wrl.h_d3d11_alias.h"

#include "RenderPass.h"

class ShadowPass : public RenderPass {
private:
	comptr<ID3D11RasterizerState> m_rastState;

public:
	void Init(Game* game) override;
	void Draw() override;

	void ResetRastState(
		int depthBias,
		float depthBiasClamp,
		float slopeScaledDepthBias,
		D3D11_CULL_MODE cullMode = D3D11_CULL_FRONT,
		D3D11_FILL_MODE fillMode = D3D11_FILL_SOLID);
};

