#include "AmbientLight.h"

#include "Game.h"
#include "Scene.h"
#include "Render.h"
#include "Assets.h"
#include "ShaderAsset.h"
#include "Actor.h"
#include "Lighting.h"
#include "MaterialResource.h"


void AmbientLight::OnInit() {
	auto render = game()->render();

	m_screenQuad.Init(render, game()->shaderAsset()->GetShader(Assets::ShaderAmbientLight));

	m_lightSource = scene()->renderer.AddLightSource(this);
	scene()->ambientLight = this;
}

void AmbientLight::OnDestroy() {
	scene()->renderer.RemoveLightSource(m_lightSource);

	if(scene()->ambientLight == this)
		scene()->ambientLight = nullptr;
}

void AmbientLight::OnDrawLight(RenderPass* renderPass) {
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

DEF_COMPONENT(AmbientLight, Engine.AmbientLight, 2, RunMode::EditPlay) {
	OFFSET(0, AmbientLight, color);
	OFFSET(1, AmbientLight, intensity);
}
