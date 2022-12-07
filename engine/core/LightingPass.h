#pragma once
#include "RenderPass.h"

#include "Mesh.h"

class LightingPass : public RenderPass {

private:
	ScreenQuad m_screenQuad;

	comptr<ID3D11Buffer> m_directionLightCBuffer;

public:
	void Init(Game* game) override;
	void Draw() override;

private:
	void m_SetLightConstBuffer();
};

