#pragma once

#include "Actor.h"

#include "RenderTarget.h"
#include "DepthStencil.h"
#include "ILightSource.h"

#include "MaterialAlias.h"

class Mesh4;
class Material;
class RenderPass;

class PointLight : public Component, public ILightSource {
	COMPONENT(PointLight);

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
DEC_COMPONENT(PointLight);
