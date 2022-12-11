#pragma once
#include "Game.h"
#include "RenderTarget.h"
#include "DepthStencil.h"
#include "ILightSource.h"

class PointLight : public Component, public ILightSource {
	PURE_COMPONENT(PointLight);

public:
	Vector3 color = Vector3::One;
	float intensity = 1.0f;
	float radius = 30;

private:
	//ScreenQuad m_screenQuad;
	const Mesh4* m_mesh;
	Material* m_material;

	Pass::LightSource m_lightSource;

public:
	void OnInit() override;
	void OnDestroy() override;

	void OnDrawLight(RenderPass* renderPass) override;
	LightCBuffer GetCBuffer() override;
};
