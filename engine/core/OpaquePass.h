#pragma once
#include "RenderPass.h"

#include "TextureResource.h"
#include "RenderTarget.h"
#include "ShaderResource.h"


class OpaquePass : public RenderPass {
public:
	TextureResource target0Tex;
	TextureResource target1Tex;
	TextureResource target2Tex;
	TextureResource target3Tex;
	TextureResource target4Tex;
	TextureResource target5Tex;

	RenderTarget target0; // target0.rgb = diffuse
	RenderTarget target1; // target1 = normals 
	RenderTarget target2; // target2 = emissive
	RenderTarget target3; // target3 = world pos
	RenderTarget target4; // material (diffuse, ambient, specular, shininess)
	RenderTarget target5; // actorId

	ShaderResource target0Res;
	ShaderResource target1Res;
	ShaderResource target2Res;
	ShaderResource target3Res;
	ShaderResource target4Res;
	ShaderResource target5Res;

public:
	void Init(Game* game) override;
	void Draw() override;

	void Resize(float width, float height) override;

};

