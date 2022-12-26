#include "SpotLight.h"

#include "Game.h"
#include "Render.h"
#include "Assets.h"
#include "MeshAsset.h"

#include "CameraComponent.h"
#include "SimpleMath.h"

DEF_PURE_COMPONENT(SpotLight);


void SpotLight::OnInit() {
	auto render = game()->render();
	auto meshAsset = game()->meshAsset();

	m_mesh = meshAsset->GetMesh(MeshAsset::formCone);
	m_meshBlend = meshAsset->GetMesh(MeshAsset::formCone);
	m_material = meshAsset->CreateDynamicMaterial("Spot Light", Assets::ShaderSpotLight);
	m_materialShape = meshAsset->CreateDynamicMaterial("Spot Light", Assets::ShaderDiffuseColor);
	m_materialShape->data.diffuseColor = { 0, 0.1f, 0, 1 };

	m_lightSource = render->AddLightSource(this);
}

void SpotLight::OnDestroy() {
	game()->render()->RemoveLightSource(m_lightSource);
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
	auto transMatrix = worldMatrix * render->camera()->cameraMatrix();

	m_material->cullMode = CullMode::Front;

	Mesh4::DynamicShapeData data;
	data.render = render;
	data.worldMatrix = &worldMatrix;
	data.transfMatrix = &transMatrix;
	data.cameraPosition = &position;

	renderPass->PrepareMaterial(m_material);
	m_mesh->Draw(data);

	if (drawShape) {
		m_materialShape->cullMode = CullMode::Back;
		m_materialShape->fillMode = FillMode::Wireframe;

		renderPass->PrepareMaterial(m_materialShape);
		m_mesh->Draw(data);

		float radiusBlend = std::tanf(theta) * length;

		localTransform = Matrix::CreateRotationX(rad(90));
		localTransform *= Matrix::CreateScale(radiusBlend * 2, radiusBlend * 2, 1);
		localTransform *= Matrix::CreateScale(1, 1, length);
		localTransform *= Matrix::CreateTranslation(0, 0, -length / 2);

		worldMatrix = localTransform * GetWorldMatrix();
		transMatrix = worldMatrix * render->camera()->cameraMatrix();

		data.worldMatrix = &worldMatrix;
		data.transfMatrix = &transMatrix;

		m_meshBlend->Draw(data);
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
