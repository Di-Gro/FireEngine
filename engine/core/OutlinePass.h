#pragma once
#include "RenderPass.h"
#include "Mesh.h"

#include "Texture.h"
#include "RenderTarget.h"
#include "ShaderResource.h"

class Actor;

class OutlinePass : public RenderPass {
public:

private:
	ScreenQuad m_screenQuad;

public:
	void Init(Game* game) override;
	void Draw() override;

};
