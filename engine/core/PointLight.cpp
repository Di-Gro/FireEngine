#include "PointLight.h"

#include "Game.h"
#include "Scene.h"
#include "Render.h"
#include "RenderPass.h"
#include "MeshAsset.h"

#include "CameraComponent.h"
#include "SimpleMath.h"


void PointLight::OnInit() {
	auto render = game()->render();
	auto meshAsset = game()->meshAsset();

	//m_screenQuad.Init(render, game()->shaderAsset()->GetShader(Assets::ShaderPointLight));
	m_mesh = meshAsset->GetMesh(MeshAsset::formSphere);
	m_material = meshAsset->CreateDynamicMaterial("Point Light", Assets::ShaderPointLightMesh);

	m_lightSource = scene()->renderer.AddLightSource(this);
}

void PointLight::OnDestroy() {
	scene()->renderer.RemoveLightSource(m_lightSource);
}

void PointLight::OnDrawLight(RenderPass* renderPass) {
	auto* render = game()->render();

	//render->context()->RSSetState(render->GetRastState(CullMode::Back));
	//m_screenQuad.Draw();

	auto position = worldPosition();
	auto worldMatrix = Matrix::CreateScale(radius * 2) * GetWorldMatrix();
	auto transMatrix = worldMatrix * render->renderer()->camera()->cameraMatrix();

	m_material->cullMode = CullMode::Front;

	Mesh4::DynamicShapeData data;
	data.render = render;
	data.worldMatrix = &worldMatrix;
	data.transfMatrix = &transMatrix;
	data.cameraPosition = &position;

	renderPass->PrepareMaterial(m_material);
	m_mesh->Draw(data);
}

LightCBuffer PointLight::GetCBuffer() {
	LightCBuffer cbuffer;

	cbuffer.position = worldPosition();
	cbuffer.color = color;
	cbuffer.param1 = intensity;
	cbuffer.param2 = radius;

	return cbuffer;
}

DEF_COMPONENT(PointLight, Engine.PointLight, 3, RunMode::EditPlay) {
	OFFSET(0, PointLight, color);
	OFFSET(1, PointLight, intensity);
	OFFSET(2, PointLight, radius);
}