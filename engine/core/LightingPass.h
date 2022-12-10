#pragma once
#include "RenderPass.h"

#include "Mesh.h"

class ILightSource;

class LightingPass : public RenderPass {

public:
	comptr<ID3D11Buffer> m_shadowCBuffer;
	comptr<ID3D11Buffer> m_lightCBuffer;

public:
	void Init(Game* game) override;
	void Draw() override;

private:
	void m_SetShadowCBuffer();
	void m_SetLightCBuffer(ILightSource* lightSource);
	
};

