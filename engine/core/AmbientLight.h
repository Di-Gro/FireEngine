#pragma once

#include "RenderTarget.h"
#include "DepthStencil.h"
#include "ILightSource.h"
#include "MaterialAlias.h"

#include "Actor.h"
#include "Mesh.h"

class RenderPass;

class AmbientLight : public Component, public ILightSource {
	COMPONENT(AmbientLight);

public:
	Vector3 color = Vector3::One;
	float intensity = 0.2f;

private:
	ScreenQuad m_screenQuad;

	Pass::LightSource m_lightSource;

public:
	void OnInit() override;
	void OnDestroy() override;

	void OnDrawLight(RenderPass* renderPass) override;
	LightCBuffer GetCBuffer() override;
};
DEC_COMPONENT(AmbientLight);
