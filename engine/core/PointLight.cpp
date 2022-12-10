#include "PointLight.h"

DEF_PURE_COMPONENT(PointLight);


void PointLight::OnInit() {
	auto render = game()->render();

	m_lightSource = render->AddLightSource(this);
}

void PointLight::OnDestroy() {
	game()->render()->RemoveLightSource(m_lightSource);
}

void PointLight::OnDrawLight() {

}

LightCBuffer PointLight::GetCBuffer() {
	LightCBuffer cbuffer;

	cbuffer.position = worldPosition();
	cbuffer.color = color;
	cbuffer.param1 = intensity;

	return cbuffer;
}
