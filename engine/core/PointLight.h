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

private:
	//ScreenQuad m_screenQuad;

	Pass::LightSource m_lightSource;

public:
	void OnInit() override;
	void OnDestroy() override;

	void OnDrawLight() override;
	LightCBuffer GetCBuffer() override;
};
