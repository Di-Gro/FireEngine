#include "AmbientLight.h"

DEF_PURE_COMPONENT(AmbientLight);


void AmbientLight::OnInit() {
	auto render = game()->render();

	m_screenQuad.Init(render, game()->shaderAsset()->GetShader(Assets::ShaderAmbientLight));

	m_lightSource = render->AddLightSource(this);
}

void AmbientLight::OnDestroy() {
	game()->render()->RemoveLightSource(m_lightSource);
}

void AmbientLight::OnDrawLight() {
	auto* render = game()->render();

	render->context()->RSSetState(render->GetRastState(CullMode::Back));
	m_screenQuad.Draw();
}

LightCBuffer AmbientLight::GetCBuffer() {
	LightCBuffer cbuffer;

	cbuffer.position = worldPosition();
	cbuffer.color = color;
	cbuffer.param1 = intensity;

	return cbuffer;
}
