#pragma once
#include "Game.h"
#include "RenderTarget.h"
#include "DepthStencil.h"
#include "ILightSource.h"

class SpotLight : public Component, public ILightSource {
	PURE_COMPONENT(SpotLight);

public:
	Vector3 color = Vector3::One;
	float intensity = 1.0f;
	float length = 300;
	/// [0, 180]
	float angle = 45;
	/// [0, 1]
	float blend = 0.150f; // ������� ������� �����
	/// [0, 5]
	float attenuation = 1; // ��������� �� ����������

	float drawShape = false;

private:
	float m_blendPower = 2;

private:
	const Mesh4* m_mesh;
	const Mesh4* m_meshBlend;
	Material* m_material;
	Material* m_materialShape;

	Pass::LightSource m_lightSource;

public:
	void OnInit() override;
	void OnDestroy() override;

	void OnDrawLight(RenderPass* renderPass) override;
	LightCBuffer GetCBuffer() override;
};
