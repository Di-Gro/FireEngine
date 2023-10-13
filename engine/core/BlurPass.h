#pragma once
#include "RenderPass.h"
#include "ScreenQuad.h"
#include "TextureResource.h"
#include "RenderTarget.h"
#include "ShaderResource.h"

class Actor;

class BlurPass : public RenderPass {
public:
	TextureResource target0Tex;
	RenderTarget target0;
	ShaderResource target0Res;

	TextureResource tmpTarget1Tex;
	RenderTarget tmpTarget1;
	ShaderResource tmpTarget1Res;

	TextureResource tmpTarget2Tex;
	RenderTarget tmpTarget2;
	ShaderResource tmpTarget2Res;

	size_t passesCount = 6;

private:
	ScreenQuad m_screenQuad;

public:
	void Init(Game* game) override;
	void Draw() override;

	void Resize(float width, float height) override;

private:
	void m_Draw(ShaderResource* resource, RenderTarget* target);
};

