#include "DirectionLight.h"

#include <iostream>
#include <sstream>

#include "CameraComponent.h"
#include "MeshComponent.h"
#include "LineComponent.h"

DEF_PURE_COMPONENT(DirectionLight);

void DirectionLight::OnInit() {
	auto render = game()->render()->device();
	auto window = game()->window();

	m_camera = AddComponent<CameraComponent>();

	m_camera->drawDebug = false;
	m_camera->callPixelShader = false;
	m_camera->setShaderMap = false;

	m_camera->orthographic(true);
	m_camera->orthoWidth = window->GetWidth() * 3;
	m_camera->orthoHeight = window->GetHeight() * 3;
	m_camera->orthoNearPlane = -3000;
	m_camera->orthoFarPlane = 3000;

	shadowRT.Init(game(), window->GetWidth(), window->GetHeight());
}

void DirectionLight::drawDebug(bool value) {
	m_needDrawDebug = value;

	if (m_debugMesh != nullptr) {
		m_debugMesh->visible = m_needDrawDebug;
		m_debugLine->visible = m_needDrawDebug;
		return;
	}
	if (m_needDrawDebug) {
		m_debugMesh = AddComponent<MeshComponent>();

		auto form = Forms4::SphereLined(10, 6, 6, { 0,1,0,1 });
		m_debugMesh->AddShape(&form.verteces, &form.indexes, 0);
		m_debugMesh->SetMaterial(0, "../../data/engine/shaders/vertex_color.hlsl");
		m_debugMesh->mesh()->topology = form.topology;

		m_debugLine = AddComponent<LineComponent>();
		m_debugLine->SetPoint(Vector3::Forward * 100, { 0,1,0,1 });

		m_debugMesh->isDebug = true;
		m_debugLine->isDebug = true;
	}
}

CameraComponent* DirectionLight::camera() {

	auto rot = transform->localRotation();
	auto rotator = Matrix::CreateFromYawPitchRoll(rot.y, rot.x, 0);

	auto newMatrix = Matrix::CreateLookAt(transform->worldPosition(), transform->worldPosition() + rotator.Forward(), rotator.Up());
	m_camera->viewMatrix(newMatrix);
	m_camera->UpdateProjectionMatrix(/*game()->window()*/);

	m_uvMatrix = m_camera->cameraMatrix() * Matrix::CreateTranslation(Vector3(1, 1, 0)) * Matrix::CreateScale({ 0.5, 0.5, 1 });

	return m_camera; 
}

void DirectionLight::RecieveGameMessage(const std::string& msg) {
	std::string nearToken = "n=";
	std::string farToken = "f=";

	auto nPos = msg.find(nearToken);
	auto fPos = msg.find(farToken);

	int nValue = 0, fValue = 0;

	if (nPos != std::string::npos) {
		size_t end = nPos + nearToken.size();
		std::string str(msg.begin() + end, msg.end());
		std::stringstream ss(str);
		ss >> nValue;
	}
	if (fPos != std::string::npos) {
		size_t end = fPos + nearToken.size();
		std::string str(msg.begin() + end, msg.end());
		std::stringstream ss(str);
		ss >> fValue;
	}

	if(nValue != 0)
		m_camera->orthoNearPlane = nValue;

	if (fValue != 0)
		m_camera->orthoFarPlane = fValue;

	if (m_camera->orthoFarPlane <= m_camera->orthoNearPlane)
		m_camera->orthoFarPlane = m_camera->orthoNearPlane + 50;
}