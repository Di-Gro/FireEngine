#include "SpotLight.h"

#include "Game.h"
#include "SimpleMath.h"
#include "Render.h"
#include "RenderPass.h"

#include "Assets.h"
#include "MeshAssets.h"
#include "MeshAsset.h"
#include "MaterialAsset.h"

#include "CameraComponent.h"


void SpotLight::OnInit() {
	auto render = game()->render();
	auto meshAsset = game()->meshAsset();

	m_mesh = meshAsset->GetMesh(MeshAssets::formCone);
	m_meshBlend = meshAsset->GetMesh(MeshAssets::formCone);
	m_material = meshAsset->CreateDynamicMaterial("Spot Light", Assets::ShaderSpotLight);
	m_materialShape = meshAsset->CreateDynamicMaterial("Spot Light", Assets::ShaderDiffuseColor);
	m_materialShape->resource.data.diffuseColor = { 0, 0.1f, 0, 1 };

	m_lightSource = scene()->renderer.AddLightSource(this);
}

void SpotLight::OnDestroy() {
	scene()->renderer.RemoveLightSource(m_lightSource);
}

void SpotLight::OnDrawLight(RenderPass* renderPass) {
	auto* render = game()->render();

	float phi = rad(angle / 2);
	float theta = (1 - blend) * phi;
	float radius = std::tanf(phi) * length;

	auto localTransform = Matrix::CreateRotationX(rad(90));
	localTransform *= Matrix::CreateScale(radius * 2, radius * 2, 1);
	localTransform *= Matrix::CreateScale(1, 1, length);
	localTransform *= Matrix::CreateTranslation(0, 0, -length / 2);

	auto position = worldPosition();
	auto worldMatrix = localTransform * GetWorldMatrix();
	auto transMatrix = worldMatrix * render->renderer()->camera()->cameraMatrix();

	m_material->resource.cullMode = CullMode::Front;

	MeshShaderData data;
	data.render = render;
	data.worldMatrix = &worldMatrix;
	data.transfMatrix = &transMatrix;
	data.cameraPosition = &position;

	renderPass->PrepareMaterial(&m_material->resource);
	m_mesh->resource.Draw(data);

	if (drawShape) {
		m_materialShape->resource.cullMode = CullMode::Back;
		m_materialShape->resource.fillMode = FillMode::Wireframe;

		renderPass->PrepareMaterial(&m_materialShape->resource);
		m_mesh->resource.Draw(data);

		float radiusBlend = std::tanf(theta) * length;

		localTransform = Matrix::CreateRotationX(rad(90));
		localTransform *= Matrix::CreateScale(radiusBlend * 2, radiusBlend * 2, 1);
		localTransform *= Matrix::CreateScale(1, 1, length);
		localTransform *= Matrix::CreateTranslation(0, 0, -length / 2);

		worldMatrix = localTransform * GetWorldMatrix();
		transMatrix = worldMatrix * render->renderer()->camera()->cameraMatrix();

		data.worldMatrix = &worldMatrix;
		data.transfMatrix = &transMatrix;

		m_meshBlend->resource.Draw(data);
	}
}

LightCBuffer SpotLight::GetCBuffer() {
	LightCBuffer cbuffer;

	float phi = rad(angle / 2);
	float theta = (1 - blend) * phi;
	
	cbuffer.position = worldPosition();
	cbuffer.direction = forward();
	cbuffer.color = color;
	cbuffer.param1 = intensity;
	cbuffer.param2 = length;
	cbuffer.param3 = theta;
	cbuffer.param4 = phi;
	cbuffer.param5 = m_blendPower;
	cbuffer.param6 = attenuation;

	return cbuffer;
}

DEF_COMPONENT(SpotLight, Engine.SpotLight, 6, RunMode::EditPlay) {
	OFFSET(0, SpotLight, color);
	OFFSET(1, SpotLight, intensity);
	OFFSET(2, SpotLight, length);
	OFFSET(3, SpotLight, angle);
	OFFSET(4, SpotLight, blend);
	OFFSET(5, SpotLight, attenuation);
}