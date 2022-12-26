#include "PointLight.h"

#include "Game.h"
#include "Render.h"
#include "MeshAsset.h"

#include "CameraComponent.h"
#include "SimpleMath.h"

DEF_PURE_COMPONENT(PointLight);


void PointLight::OnInit() {
	auto render = game()->render();
	auto meshAsset = game()->meshAsset();

	//m_screenQuad.Init(render, game()->shaderAsset()->GetShader(Assets::ShaderPointLight));
	m_mesh = meshAsset->GetMesh(MeshAsset::formSphere);
	m_material = meshAsset->CreateDynamicMaterial("Point Light", Assets::ShaderPointLightMesh);

	m_lightSource = render->AddLightSource(this);
}

void PointLight::OnDestroy() {
	game()->render()->RemoveLightSource(m_lightSource);
}

void PointLight::OnDrawLight(RenderPass* renderPass) {
	auto* render = game()->render();

	//render->context()->RSSetState(render->GetRastState(CullMode::Back));
	//m_screenQuad.Draw();

	auto position = worldPosition();
	auto worldMatrix = Matrix::CreateScale(radius * 2) * GetWorldMatrix();
	auto transMatrix = worldMatrix * render->camera()->cameraMatrix();

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
