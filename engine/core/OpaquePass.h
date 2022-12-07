#pragma once
#include "RenderPass.h"

class OpaquePass : public RenderPass {
private:
	Game* m_game;
	comptr<ID3D11Buffer> m_directionLightCBuffer;

public:
	void Init(Game* game) override;
	void Draw() override;

private:
	void m_SetLightConstBuffer();

};

