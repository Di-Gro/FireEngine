#pragma once

#include "Actor.h"

#include "RenderTarget.h"
#include "DepthStencil.h"
#include "ILightSource.h"

#include "MaterialAlias.h"

class MeshAsset;
class MaterialAsset;
class RenderPass;

class SpotLight : public Component, public ILightSource {
	COMPONENT(SpotLight);

public:
	Vector3 color = Vector3::One;
	float intensity = 1.0f;
	float length = 300;
	/// [0, 180]
	float angle = 45;
	/// [0, 1]
	float blend = 0.150f; // Процент мягкого света
	/// [0, 5]
	float attenuation = 1; // Затухание на расстаянии

	float drawShape = false;

private:
	float m_blendPower = 2;

private:
	const MeshAsset* m_mesh;
	const MeshAsset* m_meshBlend;
	MaterialAsset* m_material;
	MaterialAsset* m_materialShape;

	Pass::LightSource m_lightSource;

public:
	void OnInit() override;
	void OnDestroy() override;

	void OnDrawLight(RenderPass* renderPass) override;
	LightCBuffer GetCBuffer() override;
};
DEC_COMPONENT(SpotLight);

