#pragma once

#include "Actor.h"

#include "MeshResource.h"
#include "RenderTarget.h"
#include "DepthStencil.h"
#include "ILightSource.h"

#include "MaterialAlias.h"

class MeshAsset;
class MaterialAsset;
class RenderPass;

class PointLight : public Component, public ILightSource {
	COMPONENT(PointLight);

public:
	Vector3 color = Vector3::One;
	float intensity = 1.0f;
	float radius = 30;

private:
	const MeshAsset* m_mesh;
	MaterialAsset* m_material;

	Pass::LightSource m_lightSource;

public:
	void OnInit() override;
	void OnDestroy() override;

	void OnDrawLight(RenderPass* renderPass) override;
	LightCBuffer GetCBuffer() override;
};
DEC_COMPONENT(PointLight);
