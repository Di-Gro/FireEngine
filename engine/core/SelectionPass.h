#pragma once
#include "RenderPass.h"

#include "TextureResource.h"
#include "RenderTarget.h"
#include "ShaderResource.h"

class Actor;
class MaterialAsset;

class SelectionPass : public RenderPass {
public:
	TextureResource target0Tex;
	RenderTarget target0;
	ShaderResource target0Res;

private:
	//comptr<ID3D11Buffer> m_editorBuffer;
	//ScreenQuad m_screenQuad;

	MaterialAsset* m_highlightMaterial = nullptr;

public:
	void Init(Game* game) override;
	void Draw() override;

	void Resize(float width, float height) override;

	void m_DrawActor(Actor* actor);

};

