#pragma once
#include "RenderPass.h"

#include "Texture.h"
#include "RenderTarget.h"
#include "ShaderResource.h"


class OpaquePass : public RenderPass {
public:
	Texture target0Tex; 
	Texture target1Tex;
	Texture target2Tex;
	Texture target3Tex;
	Texture target4Tex;

	RenderTarget target0; // target0.rgb = diffuse
	RenderTarget target1; // target1 = normals 
	RenderTarget target2; // target2 = emissive
	RenderTarget target3; // target3 = world pos
	RenderTarget target4; // material (diffuse, ambient, specular, shininess)

	ShaderResource target0Res;
	ShaderResource target1Res;
	ShaderResource target2Res;
	ShaderResource target3Res;
	ShaderResource target4Res;

public:
	void Init(Game* game) override;
	void Draw() override;

	void Resize(float width, float height) override;

};

